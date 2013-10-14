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
/* function definitions */


/* ../compare/alloc.cpp */
char *fastAlloc ARGS((unsigned int size));
void freeAll ARGS((void));
void freeSome ARGS((char *block));
Net *newNet ARGS((IMEnv *imEnv, char *name, int index));
NetDevConnect *newConnection ARGS((IMEnv *imEnv));
IMEnv *initializeIMEnv ARGS((void));
char *copyString ARGS((char *string));
int casestreq ARGS((IMEnv *imEnv, char *s1, char *s2));
int nocasestreq ARGS((IMEnv *imEnv, char *s1, char *s2));
Vertex *newVertexVector ARGS((IMEnv *imEnv, int size));
Device *newDevice ARGS((IMEnv *imEnv, int index, int deviceType, int numpins));

/* ../compare/compare.cpp */
void isomorphism ARGS((IMEnv *imEnv));
void reportStatus ARGS((IMEnv *imEnv, Graph *graph));
void markVertices ARGS((IMEnv *imEnv, Vertex *vertex));
int nConnections ARGS((IMEnv *imEnv, Vertex *vertex));
void clearOccurenceCounter ARGS((IMEnv *imEnv, Vertex *vertex));
void setOccurence ARGS((IMEnv *imEnv, Vertex *vertex, int n));
int getOccurence ARGS((IMEnv *imEnv, int n));
int getCommonOcc ARGS((IMEnv *imEnv));
int getUnCommonOcc ARGS((IMEnv *imEnv));
int similar ARGS((IMEnv *imEnv, Vertex *vertex1, Vertex *vertex2, double percent));
int determineSimilarity ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2, double percent));
void matchTheGraphs ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));
int distillSections ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));
int characterizeGraphs ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));
void initTwoGraphs ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));

/* ../compare/device.cpp */
unsigned long SimpleHash ARGS((char *buf));
void initializeDeviceDefinitions ARGS((IMEnv *imEnv));
void createDefineDeviceMaster ARGS((IMEnv *imEnv, char *name, int size, char *connections[]));
void createDefineDeviceVertex ARGS((IMEnv *imEnv, char *name, uint64_t instObject, int size, char *connections[]));

/* ../compare/equate.cpp */
void initEqHash ARGS((int size));
unsigned int hash ARGS((IMEnv *imEnv, char *name));
int insertEqName ARGS((IMEnv *imEnv, char *name, int circuit, int value));
int findEqName ARGS((IMEnv *imEnv, char *name, int circuit));
void checkEquates ARGS((IMEnv *imEnv));
void defineEquate ARGS((IMEnv *imEnv, char *name1, char *name2));

/* ../compare/event.cpp */
void fireProgressEvent ARGS((IMEnv *imEnv, const char *format, ...));
void fireWarningEvent ARGS((IMEnv *imEnv, const char *format, ...));
void fireStatusEvent ARGS((IMEnv *imEnv, Graph *graph, Queue *queue, int category, int vertexType, const char *format, ...));

/* ../compare/exception.cpp */
void throwError ARGS((IMEnv *imEnv, const char *format, ...));

/* ../compare/hash.cpp */
void allocHashTable ARGS((IMEnv *imEnv, Graph *graph));
void initHashTable ARGS((IMEnv *imEnv, Graph *graph, int numberVertices));
void printHashTable ARGS((IMEnv *imEnv, Graph *graph));
void enterHash ARGS((IMEnv *imEnv, Vertex *aVertex, Graph *graph));
void computeValue ARGS((IMEnv *imEnv, Vertex *aVertex));
void incrementValue ARGS((Vertex *aVertex, int value, int devClass));
void appendUniques ARGS((IMEnv *imEnv, Graph *graph));
void assignInitialValue ARGS((IMEnv *imEnv, Vertex *vertex, int circuit));
void initialDeviceValues ARGS((IMEnv *imEnv, Graph *graph));
void initialNetValues ARGS((IMEnv *imEnv, Graph *graph));

/* ../compare/jni.cpp */
void jni_fireAnEvent ARGS((IMEnv *imEnv, const char *eventName, char *message));
void jni_fireAnEvent ARGS((IMEnv *imEnv, const char *eventName, int type, int reportType, int graphNumber, char *message, jobjectArray vertices));

/* ../compare/match.cpp */
void processUniques ARGS((IMEnv *imEnv, Graph *graph));
void cleanPendingArray ARGS((IMEnv *imEnv, Graph *graph, int whichVertices, Queue *queue));
void resetSuspects ARGS((IMEnv *imEnv, Graph *graph));
void resetBad ARGS((IMEnv *imEnv, Graph *graph));
void assignNewValues ARGS((IMEnv *imEnv, Graph *graph));
void matchUniques ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));
void matchSections ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));
int assignMatch ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));
void localMatchUniques ARGS((IMEnv *imEnv, Graph *graph1, Graph *graph2));

/* ../compare/neighbors.cpp */
void matchNeighbors ARGS((IMEnv *imEnv, Graph *graph1, Vertex *vertex1, Graph *graph2, Vertex *vertex2));

/* ../compare/net.cpp */
void initHash ARGS((IMEnv *imEnv, int size));
Net *realNet ARGS((Net *net));
Net *findNet ARGS((IMEnv *imEnv, char *name));
Net *findOrAllocNet ARGS((IMEnv *imEnv, char *name));
void deleteNet ARGS((IMEnv *imEnv, Graph *graph, Vertex *net));
void defineNetAlias ARGS((IMEnv *imEnv, char *name, int size, char *aliases[]));

/* ../compare/print.cpp */
void printTypes ARGS((DeviceDefinition *defs, int numDefs));
void PrintDevice ARGS((IMEnv *imEnv, Vertex *vertex));
void PrintVertex ARGS((IMEnv *imEnv, Vertex *vertex));
void PrintVertices ARGS((IMEnv *imEnv, Vertex *vertices, int numVertices));
void PrintVertexNeighbors ARGS((IMEnv *imEnv, Vertex *vertex));
void PrintQueue ARGS((IMEnv *imEnv, Queue *queue));
void PrintAllDevices ARGS((IMEnv *imEnv));
void printAllNets ARGS((IMEnv *imEnv));

/* ../compare/queue.cpp */
int queueOK ARGS((Queue *queue));
int matchBySuffix ARGS((Queue *queue1, Queue *queue2));
void insertionSortQ ARGS((IMEnv *imEnv, Queue *queue));
void sortQueue ARGS((IMEnv *imEnv, Queue *queue));
void insertQueue ARGS((Vertex *newVertex, Queue *queue));
