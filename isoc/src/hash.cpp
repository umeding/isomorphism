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
 * This file contains the functions that compute the values of vertices in
 * the graph and the procedures that enter the vertices in the hash table
 * and determine Unique vertices.
 */

#include "compare.h"

#define hash(number,hashSize) 	(number % hashSize)

/**
 * Allocates memory for the hashtable
 * @param graph is the graph we are working on
 */
void allocHashTable(IMEnv * imEnv, Graph * graph) {
    graph->hashTable = (Bucket *)
            fastAlloc((unsigned) imEnv->maxHashSize * sizeof (Bucket));
}

/**
 * Initialize a hash table by setting all the bucket lists to NULL
 * @param graph is the graph we are working on
 * @param numberVertices are the number of vertices inserted in this pass
 */
void initHashTable(IMEnv * imEnv, Graph * graph, int numberVertices) {
    register int i;

    graph->checkSum = 0;
    if (imEnv->hashSize == 0) {
        /*
         * Only reset HashSize if not initialized.
         * Calculate appropriate hash table size based on number of entries.
         */
        imEnv->hashSize = (numberVertices / HASHRATIO) + 1;
        if (imEnv->hashSize > imEnv->maxHashSize) {
            fireWarningEvent(imEnv,
                    "Maximum hash size exceeded: %d > %d - defaulting to %d.",
                    imEnv->hashSize, imEnv->maxHashSize,
                    imEnv->maxHashSize);
            imEnv->hashSize = imEnv->maxHashSize;
        }
    }

    for (i = 0; i < imEnv->hashSize; i++) {
        register Bucket *bucket = &graph->hashTable[i];

        bucket->bucketSum = 0; /* Clear bucket check sum */
        bucket->minPartSize = MAXINT;
        ClearQueue(&bucket->notUnique);
        ClearQueue(&bucket->Unique);
        ClearQueue(&bucket->overflow);
    }
}

#ifdef DEBUG

/**
 * Print the hash table for debugging
 */
void printHashTable(IMEnv * imEnv, Graph * graph) {
    register int i;

    printf("%d buckets\n", imEnv->hashSize);
    for (i = 0; i < imEnv->hashSize; i++) {
        register Bucket *bucket = &graph->hashTable[i];

        printf("Bucket %d:\n", i);
        printf("	%d Unique vertices\n", bucket->Unique.size);
        PrintQueue(imEnv, &(bucket->Unique));
        printf("	%d Not unique vertices\n", bucket->notUnique.size);
        PrintQueue(imEnv, &(bucket->notUnique));
        printf("	%d overflow vertices\n", bucket->overflow.size);
        PrintQueue(imEnv, &(bucket->overflow));
    }
}
#endif

/**
 *  Enter a vertex in the hash table
 *  If the vertex value is already present, add vertex to overflow queue.
 *  Otherwise enter in the Unique queue.  It is possible for the vertex to knock
 *  a vertex out of the Unique queue.
 *  All NON-UNIQUE values are added to the bucket checksum and hash table
 *  checksum.
 *
 * @param aVertex is the nmode we are entering
 * @param graph is the graph the vertex is entered into
 */
void enterHash(IMEnv * imEnv, Vertex * aVertex, Graph * graph) {
    Vertex *curVertex,
            *prevVertex;
    unsigned int thisValue = aVertex->vertexValue;

    /*
     * Bucket in which the entry is made 
     */
    Bucket *bucket =
            &graph->hashTable[hash(aVertex->vertexValue, imEnv->hashSize)];

    /*
     * Values of vertices MUST be positive when the vertices are entered in the table.
     * It is assumed that computeValue yields a positive number.
     */

    /*
     * look for the vertex in the non-uniques list
     */
    for (curVertex = bucket->notUnique.top;
            ((curVertex != NULL) && (curVertex->vertexValue != thisValue));
            curVertex = curVertex->next);

    /*
     * If the value is in the non-unique list, insert it to the overflow queue
     * later to be used by the matching routines in case the checksums didn't
     * match
     */
    if (curVertex != NULL) {
        graph->checkSum += thisValue;
        bucket->bucketSum += thisValue;
        curVertex->sectionSize++;
        insertQueue(aVertex, &bucket->overflow);
        return;
    }

    /*
     * Otherwise, search the unique list 
     */

    debug(DBG_ENTERHASH, printf("Value not non-unique\n");
            );
    prevVertex = NULL;
    curVertex = bucket->Unique.top;
    while ((curVertex != NULL) && (curVertex->vertexValue != thisValue)) {
        prevVertex = curVertex;
        curVertex = curVertex->next;
    }
    /*
     * If the vertexValue is in neither list, then the vertex is entered as unique
     */
    if (curVertex == NULL) {
        /*
         * Enter new vertex in hash table 
         */
        insertQueue(aVertex, &bucket->Unique);
        debug(DBG_ENTERHASH,
                printf("value not in hash\n");
                printf("Table entry: value:%d, hash:%d\n",
                thisValue, hash(thisValue, imEnv->hashSize));
                );
    } else {
        /*
         * The value WAS unique, but is not anymore.
         * * Move entry from unique list to non-unique list 
         */
        debug(DBG_ENTERHASH,
                printf("Unique value now non-unique: value:%d\n", thisValue);
                );
        bucket->Unique.size--;
        if (prevVertex == NULL) {
            /*
             * the vertex is the first element of the list  
             */
            bucket->Unique.top = curVertex->next;
            if (bucket->Unique.top == NULL)
                bucket->Unique.bottom = NULL;
        } else {
            prevVertex->next = curVertex->next;
        }
        if (bucket->Unique.bottom == curVertex) {
            bucket->Unique.bottom = prevVertex;
        }

        /*
         * Insert one of the vertices in the non-unique queue and the other in the 
         * NotQueued queue
         */
        graph->checkSum += 2 * thisValue;
        bucket->bucketSum += 2 * thisValue;
        insertQueue(aVertex, &bucket->overflow);
        insertQueue(curVertex, &bucket->notUnique);
        curVertex->sectionSize = 2;
    }
}

/**
 *  Primes is a vector of values that are used to multiply neighbor
 *  values depending on the terminal class.
 */
#include "primes.h"



/**
 * random[12] are variations on the normal rand call: allows us to chain numbers
 * into a random number
 */
#define random1(x) (x * 1103515245 + 12345)
#define random2(x) (x * 1015351425 + 12435)

/**
 * Computes the new value of the vertex based on the values of the neighbors
 * If the vertex is device, the terminals are used to differentiate neighbors.
 * If the vertex is a net, the terminals of the device to which it is connected
 * is used to differentiate neighbors.
 * computeValue must return a positive value.
 * @param aVertex is the vertex we are calculating
 */
void computeValue(IMEnv * imEnv, Vertex * aVertex) {
    register int i;
    register unsigned int newValue = aVertex->vertexValue;
    int numConnects = NumberOfLinks(imEnv, aVertex);

    switch (aVertex->vertexType) {
        case DEVICE:
        {
            VertexPt *connections = aVertex->connects.netList;
            TermClass *cT = imEnv->deviceDefs[aVertex->n.vertexDef].terminals;

            for (i = 0; i < numConnects; i++) {
                if (connections[i] != (VertexPt) NULL)
                    newValue += connections[i]->vertexValue * primeFactor(cT[i]);
            }
        }
            break;

        case NET:
        {
            register DeviceConnection *connections = aVertex->connects.devList;

            for (i = 0; i < numConnects; i++) {
                if (connections != (DeviceConnection *) NULL)
                    newValue += connections[i].vertex->vertexValue
                        * primeFactor2(connections[i].devClass);
            }
            break;
        }
    }
    aVertex->vertexValue = newValue;
    debug(DBG_COMPUTEVALUE, PrintVertex(imEnv, aVertex););
}

void incrementValue(Vertex * aVertex, int value, int devClass) {
    aVertex->vertexValue += value * primeFactor(devClass);
}

/**
 * Takes a graph and appends all the lists of unique elements into
 * one queue.
 * @param graph is the graph
 */
void appendUniques(IMEnv * imEnv, Graph * graph) {
    register int i;

    ClearQueue(&graph->newUniques);
    debug(DBG_HASH,
            printf("appendUniques: table size = %d\n", imEnv->hashSize);
            );
    for (i = 0; i < imEnv->hashSize; i++) {
        debug(DBG_HASH,
                printf
                ("    bucket %d: %d uniques, %d not uniques, %d overflow\n", i,
                graph->hashTable[i].Unique.size,
                graph->hashTable[i].notUnique.size,
                graph->hashTable[i].overflow.size););
        AppendQueue(&graph->newUniques, &graph->hashTable[i].Unique);
    }

    if (!(graph->newUniques.bottom == NULL ||
            graph->newUniques.bottom->next == NULL))
        throwError(imEnv, "AppendUniques: queue bottom error");
}

/**
 * Assign initial values to the vertices in the vertex list and build the initial
 * queues.
 * NETS: The initial value is the number of connections
 * DEVICES: The initial value is the primeFactor[definition-index]
 *
 * @param vertex is the vertex
 * @param circuit  1,2 for which circuit
 */
void assignInitialValue(IMEnv * imEnv, Vertex * vertex, int circuit) {
    register int value;

    if ((value = findEqName(imEnv, vertex->name, circuit)) == 0) {
        switch (vertex->vertexType) {
            case DEVICE:
                value = vertex->n.vertexDef + 1;
                value = random1(value);
                break;

            case NET:
                value = vertex->n.netConnects;
                value = random2(value);
                break;
        }
    } else {
        vertex->flag = MATCHING;
    }

    debug(DBG_INITVALUE,
            printf("Initial value for "); PrintVertex(imEnv, vertex);
            printf("	is: %d\n", value););

    vertex->vertexValue = value;
}

/**
 * InitialDeviceValues and InitialNetValues both do the initial characterizing of 
 * graph.  This must be done to get the same names matched correctly.
 * @param graph is the graph.
 */
void initialDeviceValues(IMEnv * imEnv, Graph * graph) {
    register VertexPt *vertexArray = graph->pendingDevices;
    register int size = graph->numPendingDevices;
    register int i;

    for (i = 0; i < size; i++) {
        vertexArray[i]->pass = -1;
        assignInitialValue(imEnv, vertexArray[i], graph->graphNumber);
        /*
         * This hack makes sure that devices do not
         * get set to MATCHING 
         */
        vertexArray[i]->flag = PENDING;
    }
}

/**
 * Same as InitialDeviceValues except for Nets.
 */
void initialNetValues(IMEnv * imEnv, Graph * graph) {
    register VertexPt *vertexArray = graph->pendingNets;
    register int size = graph->numPendingNets;
    register int i;

    for (i = 0; i < size; i++) {
        vertexArray[i]->flag = PENDING;
        vertexArray[i]->pass = -1;
        assignInitialValue(imEnv, vertexArray[i], graph->graphNumber);
        if (vertexArray[i]->flag == MATCHING) {
            /*
             * Put equated nets in
             * MATCHING queue 
             */
            vertexArray[i]->pass = 0; /* flag to similarVertices that these are OK */
            insertQueue(vertexArray[i], &graph->evaluationQueue);
        }
    }
}
