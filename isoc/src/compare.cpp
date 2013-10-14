/*
 * Copyright (c) 2013 Uwe B. Meding <uwe@uwemeding.com>
 *
 * This file is part of UM/ISO
 * This PCA software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Final Term is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UM/ISO.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "compare.h"

void isomorphism(IMEnv * imEnv) {
    Graph *graph1 = imEnv->__graphEnv[0].graph;
    Graph *graph2 = imEnv->__graphEnv[1].graph;

    /*
     * Initialize the flag variables 
     */

    DebugLevel = 0; /* Debug level defaults to nuthin' */

    sRandom(1);

    if (imEnv->findMatch && imEnv->noOpt)
        throwError(imEnv,
            "Cannot deduce matches and turn off optimization too!");

    /*
     * Number the two graphs, both graphs are matched in parallel. 
     */
    graph1->graphNumber = 1;
    graph2->graphNumber = 2;

    matchTheGraphs(imEnv, graph1, graph2);

    /*
     * End of matching: try to determine what happened. 
     */

    if (graph1->numNets) {
        if (imEnv->findMatch) {
            fireProgressEvent(imEnv,
                    "%d (%d%%) matches were found by local matching.",
                    imEnv->deducedMatches / 2,
                    (100 * imEnv->deducedMatches /
                    (2 *
                    (graph1->numDevices + graph1->numNets))));
        }
    }

    if (DONE_GRAPH(graph1) && DONE_GRAPH(graph2)) {

        /*
         * Both Graphs were uniquely matched. 
         */
        fireProgressEvent(imEnv, "All vertices were matched in %d passes",
                imEnv->pass);

#ifdef DEBUG

#if 0
        for (i = 0; i < imEnv->numDeviceDefs; i++) {
            printf("%3d: %s, %d\n",
                    i, imEnv->deviceDefs[i].name,
                    imEnv->deviceDefs[i].numTerminals);
        }
#endif
#endif

    } else {

        //determineSimilarity (imEnv, graph1, graph2, 70.);

        /*
         * Graphs have some mismatches 
         */

        if (!DONE_GRAPH(graph1)) {
            reportStatus(imEnv, graph1);
        }

        if (!DONE_GRAPH(graph2)) {
            reportStatus(imEnv, graph2);
        }
    }

    // We are finished
    imEnv->runComplete = TRUE;
}

/**
 * reportStatus -- Report the status of the comparison
 *
 * @param imEnv is the compare environment
 * @param graph is the graph we are reporting
 */
void reportStatus(IMEnv * imEnv, Graph * graph) {
    fireProgressEvent(imEnv, "Graph: %s", graph->graphName);

    /*
     * Report the nets 
     */
    if (graph->badNets.size > 0)
        fireStatusEvent(imEnv, graph, &graph->badNets, RPT_BAD, NET,
            "%d nets do not match", graph->badNets.size);
    if (graph->suspectNets.size > 0)
        fireStatusEvent(imEnv, graph, &graph->suspectNets,
            RPT_NO_MATCH_OTHER, NET,
            "%d nets could not be matched",
            graph->suspectNets.size);
    cleanPendingArray(imEnv, graph, NET, &graph->nets);
    if (graph->nets.size)
        fireStatusEvent(imEnv, graph, &graph->nets, RPT_NO_MATCH_SYMMETRY,
            NET, "%d nets were not matched", graph->nets.size);

    if (graph->uniqueNets.size)
        fireStatusEvent(imEnv, graph, &graph->uniqueNets, RPT_MATCH, NET,
            "%d nets matched", graph->uniqueNets.size);

    /*
     * Report the devices 
     */
    if (graph->badDevices.size)
        fireStatusEvent(imEnv, graph, &graph->badDevices, RPT_BAD, DEVICE,
            "%d devices do not match", graph->badDevices.size);
    if (graph->suspectDevices.size)
        fireStatusEvent(imEnv, graph, &graph->suspectDevices,
            RPT_NO_MATCH_OTHER, DEVICE,
            "%d devices could not be matched",
            graph->suspectDevices.size);

    cleanPendingArray(imEnv, graph, DEVICE, &graph->devices);
    if (graph->devices.size)
        fireStatusEvent(imEnv, graph, &graph->devices, RPT_NO_MATCH_SYMMETRY,
            DEVICE, "%d devices were not matched",
            graph->devices.size);

    if (graph->uniqueDevices.size)
        fireStatusEvent(imEnv, graph, &graph->uniqueDevices, RPT_MATCH,
            DEVICE, "%d devices matched",
            graph->uniqueDevices.size);

    /*
     * reset everything 
     */
    resetSuspects(imEnv, graph);
    resetBad(imEnv, graph);
    cleanPendingArray(imEnv, graph, NET, &graph->nets);
    cleanPendingArray(imEnv, graph, DEVICE, &graph->devices);
}

#if 0

void markVertices(IMEnv * imEnv, Vertex * vertex) {
    Vertex *device;
    int i,
            j;

    for (i = 0; i < vertex->n.netConnects; i++) {
        device = vertex->connects.devList[i].vertex;
        for (j = 0; j < NumberOfLinksD(imEnv, device); j++) {
            if (0 == strcmp(vertex->name, device->connects.netList[j]->name))
                device->connects.netList[j]->matched = 1;
            else
                device->connects.netList[j]->matched = 0;
        }
    }
}

int nConnections(IMEnv * imEnv, Vertex * vertex) {
    Vertex *device;
    int i,
            j;
    int count = 0;

    for (i = 0; i < vertex->n.netConnects; i++) {
        device = vertex->connects.devList[i].vertex;
        for (j = 0; j < NumberOfLinksD(imEnv, device); j++) {
            if (device->connects.netList[j]->matched)
                count++;
        }
    }
    return (count);
}

void clearOccurenceCounter(IMEnv * imEnv, Vertex * vertex) {
    Vertex *device;
    int i;

    for (i = 0; i < vertex->n.netConnects; i++) {
        device = vertex->connects.devList[i].vertex;
        imEnv->deviceDefs[device->n.vertexDef].occ[0] = 0;
        imEnv->deviceDefs[device->n.vertexDef].occ[1] = 0;
    }
}

void setOccurence(IMEnv * imEnv, Vertex * vertex, int n) {
    Vertex *device1,
            *device2;
    int i,
            j;

    for (i = 0; i < vertex->n.netConnects; i++) {
        device1 = vertex->connects.devList[i].vertex;
        for (j = 0; j < vertex->n.netConnects; j++) {
            device2 = vertex->connects.devList[j].vertex;

            if (0 ==
                    strcmp(imEnv->deviceDefs[device1->n.vertexDef].name,
                    imEnv->deviceDefs[device2->n.vertexDef].name)) {
                imEnv->deviceDefs[device1->n.vertexDef].occ[n]++;
                break;
            }
        }
    }
}

int getOccurence(IMEnv * imEnv, int n) {
    int occ = 0;
    int i;

    for (i = 0; i < imEnv->numDeviceDefs; i++) {
        occ += imEnv->deviceDefs[i].occ[n];
    }

    return (occ);
}

int getCommonOcc(IMEnv * imEnv) {
    int occ = 0;
    int i;

    for (i = 0; i < imEnv->numDeviceDefs; i++) {
        int a = imEnv->deviceDefs[i].occ[0];
        int b = imEnv->deviceDefs[i].occ[1];

        if (a > b)
            occ += b;
        else
            occ += a;
    }

    return (occ);
}

int getUnCommonOcc(IMEnv * imEnv) {
    int occ = 0;
    int i;

    for (i = 0; i < imEnv->numDeviceDefs; i++) {
        int a = imEnv->deviceDefs[i].occ[0];
        int b = imEnv->deviceDefs[i].occ[1];
        int d = a - b;

        d = d < 0 ? -d : d;
        occ += d;
    }

    return (occ);
}

int similar(IMEnv * imEnv, Vertex * vertex1, Vertex * vertex2, double percent) {
    double a,
            b,
            c,
            d,
            n,
            tanimoto,
            dice,
            cosine,
            simple,
            alpha,
            beta,
            similarity;

    clearOccurenceCounter(imEnv, vertex1);
    clearOccurenceCounter(imEnv, vertex2);

    setOccurence(imEnv, vertex1, 0);
    setOccurence(imEnv, vertex2, 1);

    a = (double) getOccurence(imEnv, 0);
    b = (double) getOccurence(imEnv, 1);
    c = (double) getCommonOcc(imEnv);
    d = (double) getUnCommonOcc(imEnv);

    /*
     * Tversky Asymetric similarity
     *
     * Tanimoto: alpha=1.0, beta=1.0
     * Dice: alpha=0.5, beta=0.5
     */
    alpha = 1.0;
    beta = 1.0;

    similarity = c / (alpha * (a - c) + beta * (b - c) + c);
    similarity *= 100.;

    if (similarity > percent) {
        printf("NET %s and %s are %3.2f%% similar.\n",
                vertex1->name, vertex2->name, similarity);
    }

    clearOccurenceCounter(imEnv, vertex1);
    clearOccurenceCounter(imEnv, vertex2);
    return (0);
}

int determineSimilarity(IMEnv * imEnv, Graph * graph1, Graph * graph2,
        double percent) {
    Queue *queue1 = &graph1->badNets;
    Queue *queue2 = &graph2->badNets;
    Vertex *vertex1,
            *vertex2;
    int i,
            j,
            k,
            l;
    Vertex *child1,
            *child2;
    Vertex *device1,
            *device2;

    printf("\n\tSimilarity between the graphs\n");
    printf("\t-------------------------------\n");

    for (vertex1 = queue1->top; vertex1 != NULL; vertex1 = vertex1->next) {
        markVertices(imEnv, vertex1);
        for (vertex2 = queue2->top; vertex2 != NULL; vertex2 = vertex2->next) {
            markVertices(imEnv, vertex2);

            similar(imEnv, vertex1, vertex2, percent);
        }
    }

    printf("\n");
    return (0);
}
#endif

/**
 * MatchTheGraphs -- Attempt to match two graphs.
 *
 * @param imEnv is the compare environment
 * @param graph1 is the 1st graph
 * @param graph2 is the 2nd graph
 */
void matchTheGraphs(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    int suspectTry; /* Number of tries to disambiguate suspects with
                                 * no progress: 
                                 *  -1 : no progress seen yet 
                                 *   0 : means some progress seen
                                 *  >0 : number of tries since progress */
    int progress; /*  number of vertices that became unique */

    /*
     * Initialization:  Assign initial values to all vertices and run one pass. 
     */

    imEnv->pass = 0;
    ClearQueue(&graph1->evaluationQueue);
    ClearQueue(&graph2->evaluationQueue);

    /*
     * InitTwoGraphs may set up the evaluation queue with MATCHING vertices 
     */
    initTwoGraphs(imEnv, graph1, graph2);

    /*
     * Always start by characterizing the Nets.  Nets with initial bad values can
     * spread harm a long ways. 
     */
    imEnv->passType = NET; /* start with the Nets. */
    suspectTry = -1; /* -1 means we have not made any progress yet */

    /*
     * Distill the sections and re-characterize the graph until we can make no 
     * further progress and return how many vertices we were able to characterize
     * uniquely. 
     */
    while ((!DONE_GRAPH(graph1)) || (!DONE_GRAPH(graph2))) {
        progress = distillSections(imEnv, graph1, graph2);

        if (progress < 0)
            break; /* Signal that it is done */
        else if (progress > 0)
            suspectTry = 0; /* Made some progress */
        else if (suspectTry < 0)
            suspectTry = imEnv->suspectCutOff; /* No more characterizing will help */
        else
            suspectTry++;

        /*
         * Check if we've done enough 
         */
        if (NUM_VERTICES_LEFT(graph1) < imEnv->errorCutOff)
            break;

        /*
         * Redeem any suspect or bad vertices found when refining sections. 
         * Assign initial values and hash instead of re-characterizing in the usual way. 
         */
        if (graph1->suspectNets.size + graph1->suspectDevices.size +
                graph2->suspectNets.size + graph2->suspectDevices.size
                + graph1->badNets.size + graph1->badDevices.size +
                graph2->badNets.size + graph2->badDevices.size != 0) {
            ClearQueue(&graph1->evaluationQueue);
            ClearQueue(&graph2->evaluationQueue);
            resetSuspects(imEnv, graph1);
            resetSuspects(imEnv, graph2);
            resetBad(imEnv, graph1);
            resetBad(imEnv, graph2);
            if (imEnv->trace)
                fireProgressEvent(imEnv, "Releasing suspects in try number %d\n", suspectTry);

            imEnv->passType = DEVICE; /* Always start characterizing graph with NETS */
        }

        /*
         * If we have already tried refining the graph enough (SuspectCutOff) times,
         * then try to assign a match.  If forcing a match on one type of vertex 
         * doesn't work, then toggle pass types and try the other type. 
         */

        if (suspectTry >= imEnv->suspectCutOff) { /* Force a match */
            int result;
            int percent =
                    (NUM_VERTICES_LEFT(graph1) * 100) / (graph1->numNets +
                    graph1->numDevices);

            if (imEnv->trace) {
                fireProgressEvent(imEnv, "%d of %d (%d%%) vertices left to be matched",
                        NUM_VERTICES_LEFT(graph1),
                        graph1->numNets + graph1->numDevices, percent);
            }
            fireProgressEvent(imEnv,
                    "Some symmetry discovered in the circuits (%d%% vertices not yet matched).",
                    percent);

            debug(DBG_FORCE, PRINT_GRAPH_STATS(graph1);
                    PRINT_GRAPH_STATS(graph2););
            if (imEnv->trace)
                fireProgressEvent(imEnv, "Attempting to guess vertices that match.");

            /*
             * Choose NET, because there is less chance screwing up 
             */
            imEnv->passType = NET;
            result = assignMatch(imEnv, graph1, graph2);
            if (result <= 0) {
                /*
                 * Match failed? 
                 */
                imEnv->passType = TOGGLE_TYPE(imEnv->passType);
                result = assignMatch(imEnv, graph1, graph2);
                if (result <= 0) {
                    /*
                     * Match failed? 
                     */
                    if (imEnv->trace)
                        fireProgressEvent(imEnv, " none found.");
                    /*
                     * Return for now, later we try something stronger 
                     */
                    return;
                }
            }

            /*
             * We haven't made any progress 
             */
            suspectTry = -1;

            /*
             * Signal that we have forced a match 
             */
            imEnv->forcedMatch = TRUE;
            if (imEnv->trace)
                fireProgressEvent(imEnv, " success.");
        }
        imEnv->passType = TOGGLE_TYPE(imEnv->passType);
    }
}

/*
 * DistillSections -- Repeatedly characterize the two graphs until no progress
 * is made. Progress is measured by the	noProgressCutOff flag.
 *
 * We are runinng passes, alternating between NETs and DEVICEs, 
 * until no progress is detected.
 *
 * @param imEnv is the compare environment
 * @param graph1 is the 1st graph
 * @param graph2 is the 2nd graph
 * @return the total number of vertices that became unique or 
 *         -1 if it is done processing the graphs.
 */
int distillSections(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    int sizeNew; /*  number of new unique vertices this round */
    int noProgress = 0; /*  number of passes with no progress */
    int numLeft = NUM_VERTICES_LEFT(graph1);
    static int errorsFound = FALSE; /* Set as soon as errors are found */

    while ((!DONE_GRAPH(graph1)) || (!DONE_GRAPH(graph2))) {
        sizeNew = characterizeGraphs(imEnv, graph1, graph2);
        if (sizeNew > 0)
            noProgress = 0;
        else
            noProgress++;

        if ((imEnv->errors > 0) && (!errorsFound)) {
            errorsFound = TRUE; /* So we print message only once */
            if (imEnv->forcedMatch)
                fireProgressEvent(imEnv,
                    "Forced a match, the circuits are probably different.");
            else
                fireProgressEvent(imEnv, "The circuits are different.");
        }
        if (noProgress >= imEnv->noProgressCutOff) {
            debug(DBG_PROGRESS,
                    printf("DistillSections: %d new uniques\n",
                    numLeft - NUM_VERTICES_LEFT(graph1)););
            return (numLeft - NUM_VERTICES_LEFT(graph1)); /* measure of progress */
        }
        imEnv->passType = TOGGLE_TYPE(imEnv->passType); /*  toggle the pass type  */
    }
    debug(DBG_PROGRESS, printf("DistillSections: DONE\n");
            );
    return (-1); /* Signals that the graphs are uniquely characterized */
}

/*
 * CharacterizeGraphs -- Perform one re-characterizing step on the two graphs.
 * 
 *	1) Assign new values to vertices in both graphs (and set the 
 *	   evaluation queue to the frontier vertices)
 *	2) Match the new unique vertices (weeding out BAD & SUSPECT vertices)
 *	3) Match the non-singleton sections (weeding out SUSPECTs)
 *
 * @param imEnv is the compare environment
 * @param graph1 is the 1st graph
 * @param graph2 is the 2nd graph
 * @return the number of vertices found unique in the current pass
 */
int characterizeGraphs(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    int verticesLeft = NUM_VERTICES_LEFT(graph1);

    imEnv->matchedCount =
            graph1->uniqueDevices.size + graph1->uniqueNets.size;

    imEnv->pass++;

    if (imEnv->trace) {
        fireProgressEvent(imEnv,
                "Pass %d: recalculating %s: %d in evaluation queue\n",
                imEnv->pass,
                (imEnv->passType == NET) ? "nets" : "devices",
                graph1->evaluationQueue.size);
    }
    debug(DBG_TRACE, PRINT_GRAPH_STATS(graph1);
            PRINT_GRAPH_STATS(graph2););
    if (imEnv->findMatch &&
            (graph1->evaluationQueue.size != 0) &&
            (graph1->evaluationQueue.top->flag == MATCHING)) {
        localMatchUniques(imEnv, graph1, graph2);
    } else {
        /*
         * Reset hash table size => recalculated by 
         * first graph that initializes hash table 
         */
        imEnv->hashSize = 0;
        assignNewValues(imEnv, graph1);
        assignNewValues(imEnv, graph2);
        debug(DBG_CHECKSUM, printf("main: checksum for %s: %d\n",
                graph1->graphName, graph1->checkSum);
                );
        debug(DBG_CHECKSUM,
                printf
                ("main: checksum for %s: %d\n",
                graph2->graphName, graph2->checkSum);
                );
        matchUniques(imEnv, graph1, graph2);
        matchSections(imEnv, graph1, graph2);
    }

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
    verticesLeft = verticesLeft - NUM_VERTICES_LEFT(graph1); /*  measures the progress  */

    /*
     * If we did not match anything this time, check to see if we are about
     * to match
     */
    if ((verticesLeft == 0) &&
            (graph1->evaluationQueue.size != 0) &&
            (graph1->evaluationQueue.top->flag == MATCHING)) {
        fireProgressEvent(imEnv, "Matching vertices found");
        return 1; /* Progress will be made next pass */
    }
    return verticesLeft;
}

static void setDevices(IMEnv * imEnv, Graph * graph, Device * devices) {
    register int deviceIndex,
            i;

    Vertex *vertex;
    int numlinks;

    deviceIndex = -1;
    while (devices != NULL) {
        deviceIndex++;
        vertex = &graph->deviceVector[deviceIndex];
        vertex->vertexType = DEVICE;
        vertex->n.vertexDef = devices->type;
        vertex->userObject = devices->instObject;
        vertex->name = (char*)"*";

        if (!(((devices->index < graph->numDevices) && devices->type >= 0)))
            throwError(imEnv,
                "Device index out of range!\nInternal problem\n");

        numlinks = imEnv->deviceDefs[devices->type].numTerminals;
        vertex->connects.netList =
                (VertexPt *) fastAlloc((unsigned) numlinks * sizeof (VertexPt));
        for (i = 0; i < numlinks; i++) {
            vertex->connects.netList[i] =
                    &(graph->netVector[IndexOfNet(devices->connections[i])]);
        }
        devices = devices->next;
    }
}

static void setNets(IMEnv * imEnv, Graph * graph, Net * nets) {
    register NetDevConnect *conP;
    register int netIndex;
    register Vertex *vertex;
    int numlinks;
    int i;

    netIndex = -1;
    while (nets != NULL) {
        if (nets->numConnects >= 0) {
            netIndex++;
            vertex = &graph->netVector[netIndex];
            vertex->vertexType = NET;
            vertex->userObject = 0;
            vertex->name = nets->name;
            numlinks = nets->numConnects;
            vertex->n.netConnects = numlinks;
            debug(DBG_INPUT,
                    printf("NET:%20s, number of links: %4d\n", vertex->name,
                    numlinks);
                    );

            vertex->connects.devList =
                    (DeviceConnection *) fastAlloc((unsigned) numlinks *
                    sizeof (DeviceConnection));
            conP = nets->connections.list;
            for (i = 0; i < numlinks; i++) {
                Vertex *devVertex;
                int devTerminal;

                devTerminal = vertex->connects.devList[i].terminal =
                        conP->terminal;
                devVertex = vertex->connects.devList[i].vertex =
                        &graph->deviceVector[conP->dev];
                vertex->connects.devList[i].devClass =
                        TerminalList(imEnv, devVertex)[devTerminal];
                conP = conP->next;
            }
            nets = nets->nextInOrder;
        } else if (nets->numConnects < 0) {

            fireProgressEvent(imEnv, "Aliased net: %s\n", nets->name);
            nets = nets->nextInOrder;
            continue; /* Aliased net: ignore */
        }
    }
}

static void
InitializeGraphRuntime(IMEnv * imEnv, Graph * graph) {
    register int i;
    register int count;

    debug(DBG_INPUT,
            printf("Number of devices: %d, number of nets: %d\n",
            graph->numDevices, graph->numNets););

    graph->deviceVector = newVertexVector(imEnv, graph->numDevices);
    ClearQueue(&graph->devices);

    graph->netVector = newVertexVector(imEnv, graph->numNets);
    ClearQueue(&graph->nets);

    setDevices(imEnv, graph, imEnv->graphEnv->devices);
    setNets(imEnv, graph, imEnv->graphEnv->nets);

    debug(DBG_INPUT, PrintAllDevices(imEnv););

    /*
     * Get rid of vertices that are not used 
     */
    for (i = 0; i < graph->numNets; i++) {
        if (NumberOfLinksN(&graph->netVector[i]) == 0) {
            if (imEnv->printZeroNets) {
                fireProgressEvent(imEnv, "Ignoring %s -- no connection.",
                        graph->netVector[i].name);
            }
            deleteNet(imEnv, graph, &graph->netVector[i]);
            /*
             * Sleazy hack: the current net is replaced by DeleteNet 
             */
            i--;
        }
    }

    /*
     * Now put all devices and nets into the pending arrays 
     */
    graph->pendingDevices =
            (VertexPt *) fastAlloc((unsigned) graph->numDevices *
            sizeof (VertexPt));
    count = 0;
    for (i = 0; i < graph->numDevices; i++) {
        if (graph->deviceVector[i].flag == DELETED)
            continue;
        graph->pendingDevices[count++] = &graph->deviceVector[i];
    }

    /*
     * Exclude DELETED vertices 
     */
    graph->numDevices = graph->numPendingDevices = count;

    graph->numPendingNets = graph->numNets;
    graph->pendingNets =
            (VertexPt *) fastAlloc((unsigned) graph->numNets *
            sizeof (VertexPt));

    for (i = 0; i < graph->numNets; i++)
        graph->pendingNets[i] = &graph->netVector[i];

    debug(DBG_INPUT,
            printTypes(imEnv->deviceDefs, imEnv->numDeviceDefs);
            PRINT_GRAPH_STATS(graph);
            printf("Device vector\n");
            PrintVertices(imEnv, graph->deviceVector, graph->numDevices);
            printf("Net vector\n");
            PrintVertices(imEnv, graph->netVector, graph->numNets););
}

/**
 * SetupGraph -- Set up a graph fro processing.
 * 
 * @param imEnv is the compare environment
 * @param graph is the graph
 */
static void setupGraph(IMEnv * imEnv, Graph * graph) {
    fireProgressEvent(imEnv, "Graph \"%s\"", graph->graphName);
    InitializeGraphRuntime(imEnv, graph);

    ClearQueue(&graph->newUniques);
    ClearQueue(&graph->evaluationQueue);
    ClearQueue(&graph->uniqueDevices);
    ClearQueue(&graph->uniqueNets);

    /*  printf("Number of device types: %d\n", imEnv->numDeviceDefs); */
    fireProgressEvent(imEnv, "Devices count: %d", graph->numDevices);
    fireProgressEvent(imEnv, "Nets count: %d", graph->numNets - 1);
    fflush(stdout);
}

/**
 * InitTwoGraphs -- Initialize the two graphs.
 *  1) Setup the two graphs
 *  2) Allocate the hash tables.
 *  3) Assign the initial values to all vertices and detect unique ones.
 *
 * @param imEnv is the compare environment
 * @param graph1 is the 1st graph
 * @param graph2 is the 2nd graph
 */
void initTwoGraphs(IMEnv * imEnv, Graph * graph1, Graph * graph2) {
    imEnv->graphEnv = &imEnv->__graphEnv[0];
    setupGraph(imEnv, graph1);

    imEnv->graphEnv = &imEnv->__graphEnv[1];
    setupGraph(imEnv, graph2);

    imEnv->maxHashSize = Max(Max(graph1->numNets, graph1->numDevices),
            Max(graph2->numNets, graph2->numDevices));
    imEnv->maxHashSize = imEnv->maxHashSize / HASHRATIO;
    imEnv->maxHashSize += 1; /* Make sure there is at least one bucket */

    debug(DBG_TRACE, printf("Maximum hash size: %d\n", imEnv->maxHashSize);
            );

    allocHashTable(imEnv, graph1);
    allocHashTable(imEnv, graph2);

    /*
     * Assign initial values to devices  
     */
    initialDeviceValues(imEnv, graph1);
    initialDeviceValues(imEnv, graph2);

    /*
     * Assign initial values to Nets 
     */
    initialNetValues(imEnv, graph1);
    initialNetValues(imEnv, graph2);

    sortQueue(imEnv, &graph1->evaluationQueue);
    sortQueue(imEnv, &graph2->evaluationQueue);

    checkEquates(imEnv); /* Check whether all names were used */
}
