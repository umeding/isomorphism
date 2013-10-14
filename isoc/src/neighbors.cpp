/*
 * Copyright (c) 2013 Uwe B. Meding <uwe@uwemeding.com>
 *
 * This file is part of UM/ISO
 * This PCA software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Isomorphism is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UM/ISO.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file contains code that tries to deduce new unique vertices by looking
 * at the neighbors of old unique vertices.
 */

#include "compare.h"

/*
 * Local hashtable entry
 */
typedef struct Hentry {
    Vertex *vertex;

    unsigned int value; /* This includes class and value */
    short int count; /* The number of entries with this hash */
    short int used; /* Flags whether the entry was found on 2nd vertex */
    unsigned int version; /* Used to mark empty entry */
} Hentry;

/* @todo: This size should be determined dynamically */
static Hentry HTable[10 * DEDUCEHTSIZE];
static unsigned int currentVersion = 0; /* Indicates if empty entry */
static int currentSize = 0;

static void startNewHT(int size) {
    register int i;

    if (size < (2 * DEDUCEHTSIZE / 3 - 1))
        currentSize = 3 * size / 2 + 1;
    else
        currentSize = DEDUCEHTSIZE;
    if (currentVersion == 0) {
        for (i = 0; i < DEDUCEHTSIZE; i++)
            HTable[i].version = 0;
    }
    if (++currentVersion == 0)
        fprintf(stderr, "Overflow in StartNewHT\n");
}

static int insertHT(Vertex * vertex, unsigned int devClass) {
    register unsigned int value = vertex->vertexValue + devClass;
    register int i,
            count;
    register unsigned int index = value % (unsigned) currentSize;

    for (count = 0, i = index;
            count < currentSize;
            count++, i = ((i == (currentSize - 1)) ? 0 : i + 1)) {
        if (HTable[i].version != currentVersion) { /* Empty entry */
            HTable[i].vertex = vertex;
            HTable[i].value = value;
            HTable[i].count = 1;
            HTable[i].version = currentVersion;
            HTable[i].used = FALSE;
            return FALSE;
        } else if (value != HTable[i].value) {
            /*
             * collision: continue 
             */
            continue;
        } else {
            /*
             * Same value 
             */
            HTable[i].count++;
            HTable[i].used = TRUE;
            return FALSE;
        }
    }
    printf("[DHT full]\n");
    return TRUE; /* Return TRUE if the table is filled up */
}

static Vertex * matchHT(IMEnv * imEnv, Vertex * vertex, unsigned int devClass) {
    register unsigned int value = vertex->vertexValue + devClass;
    register int i,
            count;
    register unsigned int index = value % (unsigned) currentSize;

    for (count = 0, i = index;
            count < currentSize;
            count++, i = ((i == (currentSize - 1)) ? 0 : i + 1)) {

        /*
         * Empty entry? 
         */
        if (HTable[i].version != currentVersion) {
            return (Vertex *) - 1;

        } else
            /*
             * Collision? 
             */
            if (value != HTable[i].value)
            continue;
        else if (HTable[i].used) {
            /*
             * Entry found: Already used ==> value not unique 
             */
            return NULL;

        } else {
            /*
             * First time use of entry 
             */
            HTable[i].used = TRUE;

            /*
             * Unique value? 
             */
            if (HTable[i].count == 1) {
                /*
                 * Vertices must match 
                 */
                return HTable[i].vertex;
            } else {
                return (NULL); /* Matching entry, but not unique */
            }
        }
    }
    throwError(imEnv, "Program error in MatchHT\n");
    return NULL; /* Should never happen */
}

/**
 * This function examines the hash table for entries that were not used
 * by MatchHT.  There are 3 cases:
 *   1) The entry is empty.
 *   2) The entry was used (Match or non-Unique)
 *   3) The entry contains a duplicate vertex.
 *
 * @param graph is the graph to which the vertices in HT belong
 */
static void finishHT(IMEnv * imEnv, Graph * graph) {
    register int i;

    for (i = 0; i < currentSize; i++) {
        if ((HTable[i].version == currentVersion) &&
                (HTable[i].used == FALSE) && (HTable[i].vertex->flag == PENDING)) {
            HTable[i].vertex->vertexValue = Random();
            debug(DBG_DEDUCE,
                    printf("Unmatched vertex in circuit 1:");
                    PrintVertex(imEnv, HTable[i].vertex);
                    );
        }
    }
}

/**
 * Try to match neighbors of vertices that have been matched.  We must add in
 * the new value of the vertices because, while they will make no difference
 * now, they may in the future.
 *
 * @param graph1 
 * @param vertex1 
 * @param graph1 
 * @param vertex2 
 */
void matchNeighbors(IMEnv * imEnv, Graph * graph1, Vertex * vertex1, Graph * graph2,
        Vertex * vertex2) {
    register int i;
    int numNeighbors;
    VertexPt vertex;

    numNeighbors = NumberOfLinks(imEnv, vertex1);
    if (numNeighbors < imEnv->deduceNeighbors) {
        /*
         * Make sure all neighbors 
         * fit in hash table 
         */
        startNewHT(numNeighbors);
        if (vertex1->vertexType == DEVICE) {
            register VertexPt *connections;
            register TermClass *cT = TerminalList(imEnv, vertex1);

            connections = vertex1->connects.netList;
            for (i = 0; i < numNeighbors; i++) {
                if (pendingVertex(connections[i])) {
                    incrementValue(connections[i], vertex1->vertexValue,
                            cT[i]);
                    if (insertHT(connections[i], cT[i]))
                        break;
                }
            }
            connections = vertex2->connects.netList;
            for (i = 0; i < numNeighbors; i++) {
                if (pendingVertex(connections[i])) {
                    incrementValue(connections[i], vertex2->vertexValue,
                            cT[i]);
                    if ((vertex = matchHT(imEnv, connections[i], cT[i]))) {
                        if (vertex == (Vertex *) - 1) {
                            /*
                             * Vertex not matched: value is set to
                             * unique value
                             */
                            connections[i]->vertexValue = Random();
                            debug(DBG_DEDUCE,
                                    printf("Unmatched vertex in circuit 2:");
                                    PrintVertex(imEnv, connections[i]);
                                    );
                        } else {
                            connections[i]->vertexValue =
                                    vertex->vertexValue = Random();
                            connections[i]->flag = vertex->flag = MATCHING;
                            debug(DBG_DEDUCE,
                                    printf("FM:");
                                    PrintVertex(imEnv, vertex);
                                    PrintVertex(imEnv, connections[i]);
                                    );
                            insertQueue(connections[i],
                                    &graph2->evaluationQueue);
                            insertQueue(vertex, &graph1->evaluationQueue);
                        }
                    }
                }
            }
        } else {
            /*
             * NET 
             */
            register DeviceConnection *connections;

            connections = vertex1->connects.devList;
            for (i = 0; i < numNeighbors; i++) {
                if (pendingVertex(connections[i].vertex)) {
                    /*
                     * Update neighbor value 
                     */
                    incrementValue(connections[i].vertex,
                            vertex1->vertexValue,
                            connections[i].devClass);
                    if (insertHT
                            (connections[i].vertex, connections[i].devClass))
                        break;
                }
            }
            connections = vertex2->connects.devList;
            for (i = 0; i < numNeighbors; i++) {
                if (pendingVertex(connections[i].vertex)) {
                    /*
                     * Update neighbor value 
                     */
                    incrementValue(connections[i].vertex,
                            vertex2->vertexValue,
                            connections[i].devClass);
                    if ((vertex =
                            matchHT(imEnv, connections[i].vertex,
                            connections[i].devClass))) {
                        if (vertex == (Vertex *) - 1) {
                            /*
                             * Vertex not matched: value is set to
                             * unique value
                             */
                            connections[i].vertex->vertexValue = Random();
                            debug(DBG_DEDUCE,
                                    printf
                                    ("Unmatched vertex in circuit 2:");
                                    PrintVertex(imEnv, connections[i].vertex);
                                    );
                        } else {
                            connections[i].vertex->vertexValue =
                                    vertex->vertexValue = Random();
                            connections[i].vertex->flag = vertex->flag =
                                    MATCHING;
                            debug(DBG_DEDUCE, printf("FM:");
                                    PrintVertex(imEnv, vertex);
                                    PrintVertex(imEnv, connections[i].vertex);
                                    );
                            insertQueue(connections[i].vertex,
                                    &graph2->evaluationQueue);
                            insertQueue(vertex, &graph1->evaluationQueue);
                        }
                    }
                }
            }
        }
        /*
         * Find unmatched vertices in first graph 
         */
        finishHT(imEnv, graph1);
    }
}
