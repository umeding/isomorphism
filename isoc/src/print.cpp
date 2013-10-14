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

/*
 * Various print routines for the graphs and queues.
 */

#include "compare.h"

/**
 * PrintType -- A debug routine to print the device types.
 *
 * @param defs are the device definitions
 * @param numDefs are the number of definitions
 */
void printTypes(DeviceDefinition * defs, int numDefs) {
    register int i;

    for (i = 0; i < numDefs; i++) {
        int j;

        printf("Type '%s' (%d):", defs[i].name, i);
        printf(" Number of terminals: %d\n", defs[i].numTerminals);
        printf("	terminal classes: ");
        for (j = 0; j < defs[i].numTerminals; j++) {
            printf("%d ", defs[i].terminals[j]);
        }
        printf("\n");
    }
}

/**
 * PrintDevice -- Print a device vertex.
 *
 * @param vertex is the device vertex
 */
void
PrintDevice(IMEnv * imEnv, Vertex * vertex) {
    for (int j = 0; j < NumberOfLinksD(imEnv, vertex); j++)
        printf(" %s(%d),", vertex->connects.netList[j]->name,
            vertex->connects.netList[j]->matched);
    printf("\n");

}

/**
 * PrintVertex -- Print a vertex.
 *
 * @param imEnv is the compare environment
 * @param vertex is the vertex we are printing
 */
void
PrintVertex(IMEnv * imEnv, Vertex * vertex) {
    register int i;

    if (!(vertex != NULL))
        throwError(imEnv, "Empty vertex to PrintVertex");

    switch (vertex->vertexType) {
        case DEVICE:
            if (!strcmp(imEnv->deviceDefs[vertex->n.vertexDef].name, ""))
                throwError(imEnv,
                    "Fatal internal error in PrintVertex: device has empty string for name\n");

            printf("DEVICE %s: ", imEnv->deviceDefs[vertex->n.vertexDef].name);
            fflush(stdout);
            if ((vertex->name[0] != '*') || (vertex->name[1] != '\0'))
                printf("\"%s\" ", vertex->name);
            if (imEnv->verbose) {
                printf("[%d,%d", vertex->vertexValue, vertex->pass);
                switch (vertex->flag) {
                    case UNIQUE:
                        printf(" UNIQUE");
                        break;
                    case SUSPECT:
                        printf(" SUSPECT");
                        break;
                    case BAD:
                        printf(" BAD");
                        break;
                    default:
                        printf(" PENDING");
                        break;
                }
                printf("]\n");
            }
            if (vertex->match)
                printf(" (MATCHED %s)",
                    imEnv->deviceDefs[vertex->match->n.vertexDef].name);
            printf("    connections: ");
            PrintDevice(imEnv, vertex);
            break;

        case NET:
            printf("NET ");
            if ((vertex->name[0] != '*') || (vertex->name[1] != '\0'))
                printf("\"%s\"", vertex->name);
            if (imEnv->verbose) {
                printf("[%d,%d,", vertex->vertexValue, vertex->pass);
                switch (vertex->flag) {
                    case UNIQUE:
                        printf(" UNIQUE");
                        break;
                    case SUSPECT:
                        printf(" SUSPECT");
                        break;
                    case BAD:
                        printf(" BAD");
                        break;
                    default:
                        printf(" PENDING");
                }
                printf("]");
            }
            if (vertex->match)
                printf(" (MATCHED %s)", vertex->match->name);
            printf(" %d connections\n", vertex->n.netConnects);
            if (vertex->n.netConnects <= imEnv->netPrintLimit) {
                for (i = 0; i < vertex->n.netConnects; i++) {
                    register Vertex *nVertex = vertex->connects.devList[i].vertex;

                    printf("  %s (%d,%d): ",
                            imEnv->deviceDefs[nVertex->n.vertexDef].name,
                            imEnv->deviceDefs[nVertex->n.vertexDef].occ[0],
                            imEnv->deviceDefs[nVertex->n.vertexDef].occ[1]);
                    PrintDevice(imEnv, nVertex);
                }
            }
            break;
    }
}

/**
 * PrintVertices -- Print a vector of vertices.
 *
 * @param imEnv is the compare environment
 * @param vertices is the array of vertices we a printing
 * @param numVertices is the number of vertices in the array
 */
void
PrintVertices(IMEnv * imEnv, Vertex * vertices, int numVertices) {
    register int i;

    for (i = 0; i < numVertices; i++)
        PrintVertex(imEnv, &vertices[i]);
}

/**
 * PrintVertexNeighbors -- Print the neighbors of a vertex.
 *
 * @param imEnv is the compare environment
 * @param vertex is the vertex we looking at
 */
void
PrintVertexNeighbors(IMEnv * imEnv, Vertex * vertex) {
    register int i;

    switch (vertex->vertexType) {
        case DEVICE:
            for (i = 0; i < NumberOfLinksD(imEnv, vertex); i++)
                PrintVertex(imEnv, vertex->connects.netList[i]);
            break;

        case NET:
            for (i = 0; i < vertex->n.netConnects; i++)
                PrintVertex(imEnv, vertex->connects.devList[i].vertex);
    }
}

/**
 * PrintQueue -- Print out an entire Queue.
 *
 * @param imEnv is the compare environment
 * @param queue is the queue we are printing
 */
void
PrintQueue(IMEnv * imEnv, Queue * queue) {
    Vertex *vertexPt;

    for (vertexPt = queue->top; vertexPt != NULL; vertexPt = vertexPt->next) {
        PrintVertex(imEnv, vertexPt);
    }
}

/**
 * PrintAllDevices -- Print all the devices.
 * 
 * @param imEnv is the compare environment
 */
void
PrintAllDevices(IMEnv * imEnv) {
    register Device *device;
    register int i;

    for (device = imEnv->graphEnv->firstDevice; device != NULL;
            device = device->next) {
        printf("*	%d", device->type);
        for (i = 0; i < imEnv->deviceDefs[device->type].numTerminals; i++) {
            printf("	%d", IndexOfNet(device->connections[i]));
        }

        printf("\n");
    }
}

/**
 * PrintAllNets -- Print all the nets in the hash table
 *
 * @param imEnv is the compare environment
 */
void printAllNets(IMEnv * imEnv) {
    register Net *netP;
    register NetDevConnect *conP;
    register int count;

    for (netP = imEnv->graphEnv->firstNet; netP != NULL;
            netP = netP->nextInOrder) {
        if (netP->index >= 0) {
            /*
             * Only print non-equal nets 
             */
            printf("%s	%d", netP->name, netP->numConnects);
            count = 0;
            for (conP = netP->connections.list; conP != NULL;
                    conP = conP->next) {
                if (count++ >= 7) {
                    printf("\n");
                    count = 0;
                }
                printf("	%d,%d", conP->dev, conP->terminal);
            }
            printf("\n");
        }
    }
}
