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
 * This file contains the functions used to find vertices of the graphs
 * unique and to match them
 */

#include "compare.h"
static struct Vertex *__queueTMP;

/**
 * Inserts all the neighbors of the vertex that are PENDING
 * into the queue (evaluation queue)
 *
 * The vertex neighbors are found in the vertex's connection list.
 * The current pass value is assigned as the vertex's pass value to
 * mark that the vertex has already
 * been inserted in the nextEval queue.  This must be done since a vertex
 * may be the neighbor of several unique vertices.
 * @param avertex a unique vertex
 * @param queue the (nextEval) queue
 */
static void queueNeighbors(IMEnv * imEnv, Vertex * avertex, Queue * queue) {
    register int i;
    register int numlinks = NumberOfLinks(imEnv, avertex); /*  number of neighbors */
    int value = avertex->vertexValue;

    switch (avertex->vertexType) {
        case DEVICE:
        {
            register Vertex **connections = avertex->connects.netList;

            for (i = 0; i < numlinks; i++) {
                /*
                 * not in the queue, and pending
                 */
                if ((connections[i]->pass != imEnv->pass) && /*  */
                        (pendingVertex(connections[i]))) {
                    connections[i]->pass = imEnv->pass;
                    connections[i]->vertexValue += value;
                    insertQueue(connections[i], queue);
                    debug(DBG_MATCH, printf("Inserting - ");
                            PrintVertex(imEnv, connections[i]););
                }
            }
        }
            break;

        case NET:
        {
            register DeviceConnection *connections = avertex->connects.devList;

            for (i = 0; i < numlinks; i++) {
                /*
                 * not in the queue, and pending
                 */
                if ((connections[i].vertex->pass != imEnv->pass) &&
                        (pendingVertex(connections[i].vertex))) {
                    connections[i].vertex->pass = imEnv->pass;
                    connections[i].vertex->vertexValue += value;
                    insertQueue(connections[i].vertex, queue);
                }
            }
        }
            break;
    }
}

/**
 * Runs through the queue of new unique vertices ( graph->newUniques )
 * and sets the flag of all the vertices to UNIQUE.
 *
 * For each of those vertices, it inserts all its neighbors that are
 * neither SUSPECT nor UNIQUE to the evaluation queue.
 * The neighbors will be processed in the next pass.
 *
 * @param graph is the graph we are processing
 */
void processUniques(IMEnv * imEnv, Graph * graph) {
    register Vertex *uVertex; /* Current unique vertex to be processed */
    int noMatches = (graph->evaluationQueue.top == NULL);

    /*
     * Just pass through the list turning each vertex into a 'Unique' vertex.
     */

    if (!noMatches)
        imEnv->deducedMatches += graph->evaluationQueue.size;
    if (imEnv->trace) {
        fireProgressEvent(imEnv, "%s: %d new unique vertices\n",
                graph->graphName, graph->newUniques.size);
        if (!noMatches) {
            fireProgressEvent(imEnv, "%s: %d all ready to be matched\n",
                    graph->graphName, graph->evaluationQueue.size);
        }
    }
    for (uVertex = graph->newUniques.top; uVertex != NULL;
            uVertex = uVertex->next) {
        uVertex->flag = UNIQUE; /* Sign flags unique vertex */
        uVertex->pass = imEnv->pass; /* Pass on which vertex became unique */
        if (noMatches)
            queueNeighbors(imEnv, uVertex, &graph->evaluationQueue);
        debug(DBG_TRACEALL, PrintVertex(imEnv, uVertex);
                );
    }
}

/**
 * Clean up the pendingNet/Device arrays by removing all except
 * pending Vertices.  If queue is not NULL, set it to all the pending vertices.
 * If the result queue is not given and it is seen that all the vertices in
 * the Pending array are indeed Pending, then don't do anything.
 *
 * All vertices should be in the pending queue, the Suspect queues,
 * the Bad queues or the Unique queues.
 *
 * @param graph we we are cleanin up
 * @param whichVertices  insert nets or devices into evaluation queue
 * @param queue queue to be formed
 */
void cleanPendingArray(IMEnv * imEnv, Graph * graph, int whichVertices,
        Queue * queue) {
    register int i;
    register VertexPt *vertexArray; /* Array of pending vertices */
    register Vertex *vertex;
    register int size; /* Size of vertex array */

    switch (whichVertices) {
        case NET:
            if ((queue == NULL) && PendingNetsAreClean(graph))
                return; /* Already OK */
            vertexArray = graph->pendingNets;
            size = graph->numPendingNets;
            break;
        case DEVICE:
            if ((queue == NULL) && PendingDevicesAreClean(graph))
                return; /* OK */
            vertexArray = graph->pendingDevices;
            size = graph->numPendingDevices;
            break;
    }

    debug(DBG_CLEAN, printf("Clean %s %s: begin size = %d\n",
            graph->graphName,
            (whichVertices == NET) ? "NETS" : "DEVICES",
            size););
    if (queue != NULL) {
        ClearQueue(queue);
        for (i = 0; i < size;) {
            if (pendingVertex(vertexArray[i])) {
                vertex = vertexArray[i++];
                computeValue(imEnv, vertex);
                insertQueue(vertex, queue);
            } else if (vertexArray[i]->flag == MATCHING) {

#ifdef DEBUG
                fireProgressEvent(imEnv, "MATCHING->PENDING: ");
                PrintVertex(imEnv, vertexArray[i]);
#endif

                vertexArray[i]->flag = PENDING;
                vertex = vertexArray[i++];
                computeValue(imEnv, vertex);
                insertQueue(vertex, queue);
            } else {
                /*
                 * Replace the non-pending vertex with the one 
                 * at the end of the array 
                 */
                vertexArray[i] = vertexArray[--size];
            }
        }
    } else {
        for (i = 0; i < size;) {
            if (vertexArray[i]->flag == MATCHING) {
                fireProgressEvent(imEnv, "Error: MATCHING->PENDING: ");
                PrintVertex(imEnv, vertexArray[i]);
                vertexArray[i]->flag = PENDING;
            }
            if (pendingVertex(vertexArray[i])) {
                i++;
            } else {
                /*
                 * Replace the non-pending vertex with the one 
                 * at the end of the array 
                 */
                vertexArray[i] = vertexArray[--size];
            }
        }
    }
    debug(DBG_CLEAN, printf("Clean: end size = %d\n", size);
            );
    switch (whichVertices) {
        case NET:
            graph->numPendingNets = size;

            if (!PendingNetsAreClean(graph))
                throwError(imEnv, "CleanPending: Nets lost or gained");
            break;
        case DEVICE:
            graph->numPendingDevices = size;

            if (!PendingDevicesAreClean(graph))
                throwError(imEnv, "CleanPending: Devices lost or gained");
            break;
    }
    debug(DBG_CLEAN, printf("After cleaning:	");
            );
}

/**
 * Take a queue of vertices that are now suspect (newQueue) and append
 * it to the correct suspect queue in graph.
 * 
 * @param newQueue queue of vertices to set suspect
 * @param graph is the graph
 */
static void setSuspectQueue(IMEnv * imEnv, Queue * newQueue, Graph * graph) {
    register Vertex *nextVertex;

    for (nextVertex = newQueue->top; nextVertex != NULL;
            nextVertex = nextVertex->next) {
        setSuspectVertex(imEnv, nextVertex);
    }

    if (newQueue->top != NULL) {
        switch (newQueue->top->vertexType) {
            case NET:
                AppendQueue(&graph->suspectNets, newQueue);
                break;
            case DEVICE:
                AppendQueue(&graph->suspectDevices, newQueue);
                break;
        }
    }
}

/**
 * Insert a bad vertex into the appropriate bad queue of the graph
 */
#define InsertBadVertex(vertex, graph)\
   switch(vertex->vertexType)	\
     { case NET:    insertQueue(vertex, &graph->badNets);	\
     		    break;	\
       case DEVICE: insertQueue(vertex, &graph->badDevices);\
       		    break;	\
     }

/**
 * Redeem all the suspect vertices in a graph.
 * First clean the pending vertex arrays then add the suspect vertices to
 * the pending arrays and queues.
 *
 * @param graph is the graph we are operating on
 */
void resetSuspects(IMEnv * imEnv, Graph * graph) {
    register Vertex *vertex;
    register int size;

    debug(DBG_SUSPECTS, printf("Resetting suspect vertices\n");
            );
    cleanPendingArray(imEnv, graph, NET, (Queue *) NULL);

    size = graph->numPendingNets;
    for (vertex = graph->suspectNets.top; vertex != NULL;
            vertex = vertex->next) {
        graph->pendingNets[size++] = vertex;

        assignInitialValue(imEnv, vertex, graph->graphNumber);

        /*
         * Forced vertices should not be here 
         */
        vertex->flag = PENDING;
    }
    graph->numPendingNets = size;
    ClearQueue(&graph->suspectNets);

    cleanPendingArray(imEnv, graph, DEVICE, (Queue *) NULL);
    size = graph->numPendingDevices;
    for (vertex = graph->suspectDevices.top; vertex != NULL;
            vertex = vertex->next) {
        graph->pendingDevices[size++] = vertex;

        assignInitialValue(imEnv, vertex, graph->graphNumber);

        /*
         * Forced vertices should not be here 
         */
        vertex->flag = PENDING;
    }
    graph->numPendingDevices = size;
    ClearQueue(&graph->suspectDevices);

    if (!(PendingNetsAreClean(graph) && PendingDevicesAreClean(graph)))
        throwError(imEnv, "ResetSuspects: nets/devices lost or gained");
    debug(DBG_TRACE,
            printf
            ("resetSuspects: pending devices: %d, pending nets:%d\n",
            graph->numPendingDevices, graph->numPendingNets););
}

/**
 * Redeem all the bad vertices in a graph.
 * First clean the pending vertex arrays then add the bad vertices to
 * the pending arrays and queues.
 *
 * @param graph is the graph we are operating on
 */
void resetBad(IMEnv * imEnv, Graph * graph) {
    register Vertex *vertex;
    register int size;

    cleanPendingArray(imEnv, graph, NET, (Queue *) NULL);
    size = graph->numPendingNets;
    for (vertex = graph->badNets.top; vertex != NULL; vertex = vertex->next) {
        graph->pendingNets[size++] = vertex;

        assignInitialValue(imEnv, vertex, graph->graphNumber);

        /*
         * Forced vertices should not be here 
         */
        vertex->flag = PENDING;
    }
    graph->numPendingNets = size;
    ClearQueue(&graph->badNets);

    cleanPendingArray(imEnv, graph, DEVICE, (Queue *) NULL);

    size = graph->numPendingDevices;
    for (vertex = graph->badDevices.top; vertex != NULL;
            vertex = vertex->next) {
        graph->pendingDevices[size++] = vertex;

        assignInitialValue(imEnv, vertex, graph->graphNumber);

        /*
         * Forced vertices should not be here 
         */
        vertex->flag = PENDING;
    }
    graph->numPendingDevices = size;
    ClearQueue(&graph->badDevices);

    if (!(PendingNetsAreClean(graph) && PendingDevicesAreClean(graph)))
        throwError(imEnv, "ResetBad: nets/devices lost or gained");
    debug(DBG_TRACE,
            printf
            ("resetBad: pending devices: %d, pending nets:%d\n",
            graph->numPendingDevices, graph->numPendingNets););
}

/**
 * Re-characterize either the Net or Device (passType) vertices in the graph.
 *
 * If noOpt is non zero, the evaluation queue is cleared which causes all
 * the vertices to be re-characterized in the pass
 *
 * This routine assigns a value to all the vertices in evaluation queue.
 * It then appends all the unique queues from the hash table to
 * graph->newUniques and sorts the newUniques queue.
 *	 
 * All the new uniques are processed, setting their flag to UNIQUE
 * and inserting their neighbors to the evaluation queue for the next 
 * pass.
 *
 * @param imEnv is the compare environment
 * @param graph is the graph
 */
void assignNewValues(IMEnv * imEnv, Graph * graph) {
    register Vertex *vertex;

    /*
     * Assume recomputation is not needed.  We must
     * * recompute if we have no EvalQueue to work
     * * with 
     */
    int recomputeFlag = FALSE;

    if (imEnv->trace)
        fireProgressEvent(imEnv, "Pass #%d, %d vertices left in %s\n",
            imEnv->pass, NUM_VERTICES_LEFT(graph),
            graph->graphName);

    if (imEnv->noOpt) {
        ClearQueue(&graph->evaluationQueue);
        recomputeFlag = TRUE;
    }

    /*
     * Check that the evaluation queue has vertices of the right type.
     * (It is assumed that all the vertices in the queue are of the same type.)
     */
    if (!(graph->evaluationQueue.top == NULL ||
            graph->evaluationQueue.top->vertexType == imEnv->passType))
        throwError(imEnv, "Wrong type of vertices in evaluation queue");

    if (graph->evaluationQueue.top == NULL) {
        recomputeFlag = FALSE; /* CleanPending will recompute */
        switch (imEnv->passType) {
            case NET:
                cleanPendingArray(imEnv, graph, NET, &graph->evaluationQueue);
                break;
            case DEVICE:
                cleanPendingArray(imEnv, graph, DEVICE, &graph->evaluationQueue);
                break;
        }
        if (imEnv->trace)
            fireProgressEvent(imEnv,
                "Inserted %d vertices into the evaluation queue",
                graph->evaluationQueue.size);
    }

    /*
     * Re-characterize vertices in the evaluation queue
     * If we have an EvalQueue, then allocate a larger hash table on the assumption
     * that we will not have as many collisions
     */

    initHashTable(imEnv, graph, graph->evaluationQueue.size);
    for (vertex = PopQueue(&graph->evaluationQueue);
            vertex != NULL; vertex = PopQueue(&graph->evaluationQueue)) {
        if (vertex->flag == MATCHING) {
            /*
             * Change back, but do not re-characterize 
             */
            vertex->flag = PENDING;
        } else {
            debug(DBG_ALWAYS,
            if (!(pendingVertex(vertex)))
                    throwError(imEnv, "AssignValues: vertex not pending"););
            if (recomputeFlag)
                computeValue(imEnv, vertex);
        }
        enterHash(imEnv, vertex, graph);
    }

    debug(DBG_HASHTABLE, printHashTable(imEnv, graph);
            );
    /*
     * append the queues of unique vertices 
     */
    appendUniques(imEnv, graph);
    sortQueue(imEnv, &graph->newUniques);
}

/**
 * Compare two vertices: they are 'similar' if:
 *   1. They have the same type
 *   2. They have the same number of connections
 * Only vertices with the same vertex values will be checked for similarity
 *
 * Two vertices are 'identical' if they are similar and their neighbors have
 * the same vertex values.
 */
#define similarVertices(vertex1, vertex2)	\
(((vertex1)->vertexType == (vertex2)->vertexType)   &&	\
 ((vertex1)->n.vertexDef == (vertex2)->n.vertexDef))

/**
 * Use a random value when matching two vertices
 */
#define newUniqueValue()  Random()

/**
 * Match the new unique vertex queues of two graphs.  The queues are
 * assumed to be sorted so that a simple merge-match is sufficient.
 * Vertices that do not match are BAD: their values are set to zero so that
 * they don't pollute neighboring values.
 *
 * If the two corresponding vertices are similar but have different
 * properties  a warning is issued (if PrintWarnings is true)
 *
 * For each pair of matching vertices, deduce which neighbors must match.
 * These are put in the evaluation queue.  If this is not empty, then vertices
 * do not need to be re-characterized and this process continues apace.
 *
 * @param graph1 is the 1st graph
 * @param graph2 is the 2nd graph
 */
void matchUniques(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    Queue queue1,
            queue2;
    register Vertex *vertex1,
            *vertex2; /*  Index pointers */

    queue1 = graph1->newUniques;
    queue2 = graph2->newUniques;
    ClearQueue(&graph1->newUniques);
    ClearQueue(&graph2->newUniques);
    ClearQueue(&graph1->evaluationQueue);
    ClearQueue(&graph2->evaluationQueue);
    vertex1 = PopQueue(&queue1);
    vertex2 = PopQueue(&queue2);
    while ((vertex1 != NULL) && (vertex2 != NULL)) { /* Neither queue empty */
        vertex1->match = vertex2;
        vertex2->match = vertex1;
        if (vertex1->vertexValue == vertex2->vertexValue) { /* Vertex values match */

            /*
             * Make sure vertices match: recompute values.  This is done because we may go
             * a long time without re-characterizing and two vertices may still have the same
             * value even though the neighbors have been characterized differently
             */
            debug(DBG_DOUBLECHECK,
                    computeValue(imEnv, vertex1);
                    computeValue(imEnv, vertex2);
            if (vertex1->vertexValue != vertex2->vertexValue) {
                printf("Vertices with same value do not match!!\n");
                        PrintVertex(imEnv, vertex1);
                        debug(DBG_PRINTBAD,
                        PrintVertexNeighbors(imEnv,
                        vertex1);) PrintVertex(imEnv,
                        vertex2);
                        debug(DBG_PRINTBAD,
                        PrintVertexNeighbors(imEnv,
                        vertex2);)
                        setBadVertex(imEnv, vertex1);
                        setBadVertex(imEnv, vertex2);
            }
            );

            if (badVertex(vertex1) || badVertex(vertex2)) {
                PrintVertex(imEnv, vertex1);
                PrintVertex(imEnv, vertex2);
                InsertBadVertex(vertex1, graph1);
                InsertBadVertex(vertex2, graph2);
            } else {
                vertex1->vertexValue = vertex2->vertexValue =
                        newUniqueValue();

                /*
                 * Now try to find neighbors that we can call unique 
                 */
                if (imEnv->findMatch)
                    matchNeighbors(imEnv, graph1, vertex1, graph2, vertex2);
                insertQueue(vertex1, &graph1->newUniques);
                insertQueue(vertex2, &graph2->newUniques);

                debug(DBG_UNIQUES,
                        printf("Matched: ");
                        PrintVertex(imEnv, vertex1);
                        printf("	 ");
                        PrintVertex(imEnv, vertex2););

                vertex1->match = vertex2;
                vertex2->match = vertex1;
            }
            vertex1 = PopQueue(&queue1);
            vertex2 = PopQueue(&queue2);
        } else {
            /*
             * Vertex values do not match: the vertex with lesser value is BAD 
             */
            if (vertex1->vertexValue < vertex2->vertexValue) {
                /*
                 * First queue has bad vertex 
                 */
                debug(DBG_PRINTBAD,
                        printf("Bad vertex in %s:\n", graph1->graphName);
                        PrintVertex(imEnv, vertex1););
                setBadVertex(imEnv, vertex1);
                InsertBadVertex(vertex1, graph1);
                vertex1 = PopQueue(&queue1);
            } else { /* Second queue has bad vertex */
                debug(DBG_PRINTBAD,
                        printf("Bad vertex in %s:\n", graph2->graphName);
                        PrintVertex(imEnv, vertex2););
                setBadVertex(imEnv, vertex2);
                InsertBadVertex(vertex2, graph2);
                vertex2 = PopQueue(&queue2);
            }
        }
    }

    /*
     * Finish off unmatched queues 
     */

    for (; vertex1 != NULL; vertex1 = PopQueue(&queue1)) {
        setBadVertex(imEnv, vertex1);
        debug(DBG_PRINTBAD,
                printf("Bad vertex in %s:\n", graph1->graphName);
                PrintVertex(imEnv, vertex1););
        InsertBadVertex(vertex1, graph1);
    }

    for (; vertex2 != NULL; vertex2 = PopQueue(&queue2)) {
        setBadVertex(imEnv, vertex2);
        debug(DBG_PRINTBAD,
                printf("Bad vertex in %s:\n", graph2->graphName);
                PrintVertex(imEnv, vertex2););
        InsertBadVertex(vertex2, graph2);
    }

}

/**
 * The input queue is assume to be sorted. 
 * Extract the first set of vertices with the same value and return 
 * as the result queue.
 * (This routine can easily be optimized)
 *
 * @param inQueue
 * @param result
 */
static void extractQueue(Queue * inQueue, Queue * result) {
    register Vertex *tempVertex;
    register unsigned int value;

    ClearQueue(result);
    if (inQueue->top != NULL) {
        value = inQueue->top->vertexValue;
        while ((inQueue->top != NULL) && (value == inQueue->top->vertexValue)) {
            tempVertex = PopQueue(inQueue);
            insertQueue(tempVertex, result);
        }
    }
}

/**
 * Match the non-singular sections in the two graphs.
 * Only the sections with non-matching checksums need to be matched.
 *
 * @param graph1 the 1st graph
 * @param graph2 the 2nd graph
 */
void matchSections(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    Queue section1,
            section2;
    register int index;
    register Bucket *bucket1,
            *bucket2;
    register Vertex *vertex;
    Queue stillNotQueued1, /* Vertices that are not queued and not */

            stillNotQueued2; /* suspect */

    debug(DBG_CHECKSUM,
            printf("main: checksum for %s: %d\n", graph2->graphName,
            graph2->checkSum););

    for (index = 0; index < imEnv->hashSize; index++) {
        /*
         * loop through hash table buckets 
         */

        bucket1 = &graph1->hashTable[index];
        bucket2 = &graph2->hashTable[index];
        debug(DBG_CHECKSUM,
        if ((bucket1->bucketSum != 0) || (bucket2->bucketSum != 0))
                printf("bucketSums: %d, %d\n",
                bucket1->bucketSum, bucket2->bucketSum););

        if (bucket1->bucketSum == bucket2->bucketSum)
            continue;

        /*
         * For each entry in the hash table, append all the vertices that are
         * not unique, sort them and compare the sections of the two graphs
         */
        ClearQueue(&stillNotQueued1);
        ClearQueue(&stillNotQueued2);

        AppendQueue(&bucket1->notUnique, &bucket1->overflow);
        ClearQueue(&bucket1->overflow);
        sortQueue(imEnv, &bucket1->notUnique);

        AppendQueue(&bucket2->notUnique, &bucket2->overflow);
        ClearQueue(&bucket2->overflow);
        sortQueue(imEnv, &bucket2->notUnique);

        extractQueue(&bucket1->notUnique, &section1);
        extractQueue(&bucket2->notUnique, &section2);
        bucket1->minPartSize = MAXINT;
        bucket2->minPartSize = MAXINT;
        while ((section1.top != NULL) && (section2.top != NULL)) {
            /*
             * loop through sections in one bucket 
             */
            if (section1.top->vertexValue == section2.top->vertexValue) {
                /*
                 * values are the same 
                 */
                if (section1.size != section2.size) {
                    /*
                     * Sections have the same value but different sizes.
                     * All the vertices in both are suspect.
                     */
                    setSuspectQueue(imEnv, &section1, graph1);
                    debug(DBG_SUSPECTS,
                            printf("%s:section found suspect:\n",
                            graph1->graphName);
                            PrintQueue(imEnv, &section1););
                    setSuspectQueue(imEnv, &section2, graph2);
                    debug(DBG_SUSPECTS,
                            printf("%s:section found suspect:\n",
                            graph2->graphName);
                            PrintQueue(imEnv, &section2););
                } else {
                    /*
                     * The sections match: reinsert into queues 
                     */
                    vertex = PopQueue(&section1);
                    vertex->sectionSize = section1.size + 1;
                    if (vertex->sectionSize < bucket1->minPartSize) {
                        bucket1->minPartSize = vertex->sectionSize;
                    }
                    insertQueue(vertex, &stillNotQueued1);
                    AppendQueue(&bucket1->overflow, &section1);

                    vertex = PopQueue(&section2);
                    vertex->sectionSize = section2.size + 1;
                    if (vertex->sectionSize < bucket2->minPartSize) {
                        bucket2->minPartSize = vertex->sectionSize;
                    }
                    insertQueue(vertex, &stillNotQueued2);
                    AppendQueue(&bucket2->overflow, &section2);
                }

                extractQueue(&bucket1->notUnique, &section1);
                extractQueue(&bucket2->notUnique, &section2);
            } else {
                /*
                 * sections out of sync 
                 */
                if (section1.top->vertexValue < section2.top->vertexValue) {
                    /*
                     * Section from graph 1 is suspect 
                     */
                    setSuspectQueue(imEnv, &section1, graph1);
                    debug(DBG_SUSPECTS,
                            printf("%s:section found suspect:\n",
                            graph1->graphName);
                            PrintQueue(imEnv, &section1););
                    extractQueue(&bucket1->notUnique, &section1);
                } else {
                    /*
                     * Section from graph 2 is suspect 
                     */
                    setSuspectQueue(imEnv, &section2, graph2);
                    debug(DBG_SUSPECTS,
                            printf("%s:section found suspect:\n",
                            graph2->graphName);
                            PrintQueue(imEnv, &section2););
                    extractQueue(&bucket2->notUnique, &section2);
                }
            }
        } /* end of section loop */

        /*
         * Finish off unmatched sections at end 
         */
        setSuspectQueue(imEnv, &section1, graph1);
        setSuspectQueue(imEnv, &section2, graph2);

        setSuspectQueue(imEnv, &bucket1->notUnique, graph1);
        setSuspectQueue(imEnv, &bucket2->notUnique, graph2);

        bucket1->notUnique = stillNotQueued1;
        bucket2->notUnique = stillNotQueued2;

    } /* end of hash table loop */

}

/**
 * AssignMatch guesses a pair of vertices in the two graphs to match.
 * An arbitrary vertex from the smallest sections is chosen as having the 
 * best chance of matching.  These two vertices are assigned a random value so 
 * that characterizing can continue.
 * Although this routine does not assume that the sections match, they will
 * in practice since MatchSections is called by CharacterizeGraphs.
 *
 * The two new unique vertices are then placed in the Unique queue and processed
 * as normal new unique vertices.
 * If there are no neighbors of the unique vertices, then the AssignMatch process
 * is done again until there are some neighbors for the re-characterizing process.
 * Vertices are taken from the notUnique queues and put back into the overflow
 * queues.  Since the vertices are likely to be sorted the next time around,
 * SortQueue should be able recognize this (eg. unlike quicksort)
 * 
 * @param graph1 is the 1st graph
 * @param graph2 is the 2nd graph
 */
int assignMatch(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    register Vertex *vertex1,
            *vertex2;
    register Bucket *minBucket1,
            *minBucket2;
    register Vertex *firstVertex;
    Queue section1,
            section2,
            tmpSection;
    int index,
            firstIndex;
    int minSize;
    unsigned int minValue; /* Size of smallest section found */
    int minIndex; /* Bucket from which minSections came */
    int success = 0; /* Number of vertices matched */

    index = characterizeGraphs(imEnv, graph1, graph2);
    if (index < 0)
        return (0);
    if (index > 0) {
        if (imEnv->trace) {
            fireProgressEvent(imEnv,
                    "Recalculating before guessing found matching vertices -- matching aborted\n");
        }
        return (index);
    }

    minIndex = 0;
    while (TRUE) {
        /*
         * until there are neighbors of the unique vertices 
         */
        minSize = MAXINT;
        debug(DBG_FORCE,
                printf("AssignMatch: %d buckets\n", imEnv->hashSize);
                );

        /*
         * First find the smallest section size
         * We always start at the same bucket in which we last looked to avoid
         * painfully wending our way back to where we were if there are sections
         * of size 2.  (We could use a heap to keep track of the smallest sections.)
         * (Also, we should extract the sections being matched and sort by 
         * reversed name.  This would tend to match vertices with the same suffixes.)
         */
        firstIndex = index = minIndex; /* Marks start bucket */
        do {
            minBucket1 = &graph1->hashTable[index];
            minBucket2 = &graph2->hashTable[index];

            /*
             * Compute min section sizes if not already done 
             */
            if (minBucket1->minPartSize == MAXINT) {
                if (minBucket1->notUnique.size == 0)
                    continue; /* Empty bucket */

                QForEach(vertex1, minBucket1->notUnique) {
                    if (vertex1->sectionSize < minBucket1->minPartSize) {
                        minBucket1->minPartSize = vertex1->sectionSize;
                        if (vertex1->sectionSize == 2)
                            break; /* Can't be smaller */
                    }
                }
            }
            if (minBucket2->minPartSize == MAXINT) {
                if (minBucket2->notUnique.size == 0)
                    continue;

                QForEach(vertex2, minBucket2->notUnique) {
                    if (vertex2->sectionSize < minBucket2->minPartSize) {
                        minBucket2->minPartSize = vertex2->sectionSize;
                        if (vertex2->sectionSize == 2)
                            break;
                    }
                }
            }
            if (!((minBucket1->minPartSize == minBucket2->minPartSize)))
                throwError(imEnv, "minPartSize Error");
            if (minBucket1->minPartSize < minSize) { /* Update global min size */
                minSize = minBucket1->minPartSize;
                minIndex = index;
                if (minSize == 2)
                    break;
            }
        }        while ((index = (index + 1) % imEnv->hashSize) != firstIndex);

        /*
         * minSize == MAXINT ==> no sections left to be matched.  We are done 
         */

        if (minSize == MAXINT)
            return (success);

        if (imEnv->trace)
            fireProgressEvent(imEnv,
                "Matching vertices from sections of size %d:\n",
                minSize);
        ClearQueue(&graph1->newUniques);
        ClearQueue(&graph2->newUniques);
        minBucket1 = &graph1->hashTable[minIndex];
        minBucket2 = &graph2->hashTable[minIndex];
        debug(DBG_FORCE,
                printf("minSize = %d, minIndex = %d\n", minSize, minIndex);
                printf("notUnique.size = %d, overflow.size = %d\n",
                minBucket1->notUnique.size,
                minBucket1->overflow.size););

        /*
         * If each section contains equivalent vertices (ie. connected to the same
         * things) then all can be set to the same value
         * 
         * Now find a vertex from the smallest section in graph 1 
         */

        firstVertex = PopQueue(&minBucket1->notUnique);
        vertex1 = firstVertex;
        while (TRUE) {
            if (!(vertex1 != NULL))
                throwError(imEnv, "notUnique 1 empty");
            if (vertex1->sectionSize == minSize)
                break;
            insertQueue(vertex1, &minBucket1->notUnique);
            vertex1 = PopQueue(&minBucket1->notUnique);
            if (!(vertex1 != firstVertex))
                throwError(imEnv, "AssignMatch: Section 1 error");
        }
        minValue = vertex1->vertexValue;
        ClearQueue(&section1);
        insertQueue(vertex1, &section1);

        /*
         * Now find a vertex from the matching section in graph 2 
         */

        firstVertex = PopQueue(&minBucket2->notUnique);
        vertex2 = firstVertex;
        while (TRUE) {
            if (!(vertex2 != NULL))
                throwError(imEnv, "notUnique 2 empty");
            if (vertex2->vertexValue == minValue)
                break;
            insertQueue(vertex2, &minBucket2->notUnique);
            vertex2 = PopQueue(&minBucket2->notUnique);
            if (!(vertex2 != firstVertex))
                throwError(imEnv, "AssignMatch: Section 2 error");
        }
        ClearQueue(&section2);
        insertQueue(vertex2, &section2);

        /*
         * Grab the remaining vertices from the two sections 
         */
        ClearQueue(&tmpSection);
        for (vertex1 = PopQueue(&minBucket1->overflow); vertex1 != NULL;
                vertex1 = PopQueue(&minBucket1->overflow)) {
            if (vertex1->vertexValue == minValue) {
                insertQueue(vertex1, &section1);
            } else {
                insertQueue(vertex1, &tmpSection);
            }
        }
        minBucket1->overflow = tmpSection;

        ClearQueue(&tmpSection);
        for (vertex2 = PopQueue(&minBucket2->overflow); vertex2 != NULL;
                vertex2 = PopQueue(&minBucket2->overflow)) {
            if (vertex2->vertexValue == minValue) {
                insertQueue(vertex2, &section2);
            } else {
                insertQueue(vertex2, &tmpSection);
            }
        }
        minBucket2->overflow = tmpSection;

        /*
         * MatchBySuffix is very expensive: don't use for large sections 
         */
        if (imEnv->useSuffix && (imEnv->passType == NET) &&
                (minSize < 20) && matchBySuffix(&section1, &section2)) {
            if (imEnv->trace)
                fireProgressEvent(imEnv, "Sucessfully matched by suffix."); /* success */
        } else { /* try sorting by attribute */
            insertionSortQ(imEnv, &section1);
            insertionSortQ(imEnv, &section2);
        }

        /*
         * Match the two first vertices from the smallest section 
         */
        vertex1 = PopQueue(&section1);
        vertex2 = PopQueue(&section2);

        vertex1->vertexValue = vertex2->vertexValue = newUniqueValue();
        vertex1->match = vertex2;
        vertex2->match = vertex1;
        insertQueue(vertex1, &graph1->newUniques);
        insertQueue(vertex2, &graph2->newUniques);
        matchNeighbors(imEnv, graph1, vertex1, graph2, vertex2); /* Match neighbors */

        if (imEnv->trace)
            fireProgressEvent(imEnv, "Successful local match."); /* Indicates vertices were matched */
        success++;
        vertex1 = PopQueue(&section1);
        vertex2 = PopQueue(&section2);

        /*
         * If the section size was 2, then the other pair of vertices must match too 
         * * Otherwise, just add the next pair to notUnique
         */
        if (minSize == 2) {
            vertex1->vertexValue = vertex2->vertexValue = newUniqueValue();
            vertex1->match = vertex2;
            vertex2->match = vertex1;
            insertQueue(vertex1, &graph1->newUniques);
            insertQueue(vertex2, &graph2->newUniques);
            matchNeighbors(imEnv, graph1, vertex1, graph2, vertex2); /* Match neighbors */

            if (imEnv->trace)
                fireProgressEvent(imEnv, "Successful local match."); /* Indicates vertices were matched */
            success++;
        } else {
            vertex1->sectionSize = vertex2->sectionSize = minSize - 1;
            insertQueue(vertex1, &minBucket1->notUnique);
            insertQueue(vertex2, &minBucket2->notUnique);
            AppendQueue(&minBucket1->overflow, &section1);
            AppendQueue(&minBucket2->overflow, &section2);
        }

        /*
         * Force recomputation of the size of the smallest section in the bucket 
         */

        minBucket1->minPartSize = minBucket2->minPartSize = MAXINT;
        fflush(stdout); /* Send out those messages */
        processUniques(imEnv, graph1);
        processUniques(imEnv, graph2);
        switch (imEnv->passType) {
            case DEVICE:
                AppendQueue(&graph1->uniqueDevices, &graph1->newUniques);
                AppendQueue(&graph2->uniqueDevices, &graph2->newUniques);
                break;
            case NET:
                AppendQueue(&graph1->uniqueNets, &graph1->newUniques);
                AppendQueue(&graph2->uniqueNets, &graph2->newUniques);
                break;
        }

        /*
         * If we were able to do some local matching on the opposite vertex type, then
         * do another local matching:  If this does not produce anything then we
         * will continue through the sections.  The vertices that are matched here are
         * guaranteed not to be the same type and so do not conflict with the
         * section matching algorithm.
         */
        if ((graph1->evaluationQueue.size != 0) &&
                (graph1->evaluationQueue.top->flag == MATCHING)) {
            imEnv->passType = TOGGLE_TYPE(imEnv->passType);
            localMatchUniques(imEnv, graph1, graph2);
            processUniques(imEnv, graph1);
            processUniques(imEnv, graph2);
            switch (imEnv->passType) {
                case DEVICE:
                    AppendQueue(&graph1->uniqueDevices, &graph1->newUniques);
                    AppendQueue(&graph2->uniqueDevices, &graph2->newUniques);
                    break;
                case NET:
                    AppendQueue(&graph1->uniqueNets, &graph1->newUniques);
                    AppendQueue(&graph2->uniqueNets, &graph2->newUniques);
                    break;
            }
            /*
             * If we did not find anything and must continue matching sections
             * then reset the PassType
             */
            if ((graph1->evaluationQueue.size +
                    graph2->evaluationQueue.size) == 0) {
                imEnv->passType = TOGGLE_TYPE(imEnv->passType);
            }
        }

        /*
         * At this point, there will be vertices in the evaluation queue if:
         *  1) There are neighbors of the matched section (but not locally matched)
         *  2) There are locally matching neighbors of locally matching neighbors of
         *     matched section.
         *  In either case we exit and continue with the normal algorithm
         */
        if ((graph1->evaluationQueue.size + graph2->evaluationQueue.size) > 0) {
            break;
        }
        continue;
    }

    /*
     * succeeded in matching vertices: return number of vertices matched 
     */
    return (success);
}

/**
 * This procedure is used when there are locally matched vertices to be
 * processed.  These are presumed in order, but not sorted.  We will never
 * find unmatched vertices.  These matched vertices start out in the evaluation queue
 * and are first turned into unique vertices while at the same time performing
 * more local matching.
 *
 * @param graph1 is the 1st graph
 * @param graph2 is the 1st graph
 */
void localMatchUniques(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    Queue queue1,
            queue2;
    register Vertex *vertex1,
            *vertex2; /*  Index pointers */

    if (imEnv->trace)
        fireProgressEvent(imEnv, "Matching local unique vertices.");

    queue1 = graph1->evaluationQueue;
    queue2 = graph2->evaluationQueue;
    ClearQueue(&graph1->newUniques);
    ClearQueue(&graph2->newUniques);
    ClearQueue(&graph1->evaluationQueue);
    ClearQueue(&graph2->evaluationQueue);

    vertex1 = PopQueue(&queue1);
    vertex2 = PopQueue(&queue2);
    while ((vertex1 != NULL) && (vertex2 != NULL)) {
        /*
         * Neither queue empty 
         */
        if ((vertex1->vertexValue != vertex2->vertexValue)) {
            throwError(imEnv, "LocalMatchUniques: Vertices do not match.");

            /*
                        fireWarningEvent(imEnv, 
                                 "LocalMatchUniques: Vertices do not match.");
                        PrintVertex (imEnv, vertex1);
                        PrintVertex (imEnv, vertex2);

                        exit(1);
             */
        }
        vertex1->vertexValue = vertex2->vertexValue = newUniqueValue();

        /*
         * Now try to find neighbors that we can call unique 
         */
        if (imEnv->findMatch) {
            matchNeighbors(imEnv, graph1, vertex1, graph2, vertex2);
        }
        insertQueue(vertex1, &graph1->newUniques);
        insertQueue(vertex2, &graph2->newUniques);
        debug(DBG_UNIQUES,
                printf("Matched: ");
                PrintVertex(imEnv, vertex1);
                printf("	 ");
                PrintVertex(imEnv, vertex2););
        vertex1 = PopQueue(&queue1);
        vertex2 = PopQueue(&queue2);
    }

    if ((vertex1 != NULL) || (vertex2 != NULL))
        throwError(imEnv, "Local Match Error: Queues are not empty!!!\n");

}
