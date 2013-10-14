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
#include <malloc.h>
/*
 * All the memory handling mechanism are contained here.
 */

unsigned int DebugLevel; /* string containing multiple attributes */

typedef struct Chunk {
    struct Chunk *next;
    struct Chunk *prev;
} Chunk;

static Chunk *mem = NULL;

/** 
 * FastAlloc -- allocate some memory.
 *
 * @param size is the desired size
 * @return a pointer to the new memory
 */
char * fastAlloc(unsigned int size) {
    Chunk *p;

    size += sizeof (Chunk);
    p = (Chunk *) malloc(size);
    memset((void *) p, 0, size);
    // keep in list
    p->next = mem;
    p->prev = NULL;
    if (mem)
        mem->prev = p;
    mem = p;
    // The real location is right after the Chunk
    char *loc = (char *) p;

    loc += sizeof (Chunk);
    return (loc);
}

/**
 * FreeAll -- Free all the allocated memory.
 */
void freeAll() {
    Chunk *p = mem;
    Chunk *q = NULL;

    for (p = mem; p; p = p->next) {
        if (q)
            free(q);
        q = p;
    }
    if (q)
        free(q);
    mem = NULL;
}

/**
 * freeSome -- free some previously allocated memory.
 *
 * The argument is expected to point to the user space which is
 * located right after the management structures.
 *
 * @param block is the pointer to the user block
 */
void freeSome(char *block) {
    Chunk *p = (Chunk *) block;

    // calculate the real location
    p = (Chunk *) (block - sizeof (Chunk));

    if (p->prev)
        p->prev->next = p->next;
    else
        mem = p->next;
    if (p->next)
        p->next->prev = p->prev;
    free(p);
}

/**
 * AllocGraph -- Allocate and initialize space for a graph.
 *
 * @return an empty graph
 */
static Graph * allocGraph() {
    Graph *result = (Graph *) fastAlloc((unsigned) sizeof (Graph));

    result->graphName = NULL;
    result->hashTable = NULL;
    result->checkSum = 0;
    ClearQueue(&result->newUniques);
    ClearQueue(&result->evaluationQueue);
    result->lastUniquePass = 0;
    ClearQueue(&result->devices);
    ClearQueue(&result->nets);
    result->deviceVector = result->netVector = NULL;
    result->numDevices = result->numNets = 0;
    result->pendingDevices = result->pendingNets = NULL;
    result->numPendingDevices = result->numPendingNets = 0;
    ClearQueue(&result->uniqueDevices);
    ClearQueue(&result->uniqueNets);
    ClearQueue(&result->suspectDevices);
    ClearQueue(&result->suspectNets);
    ClearQueue(&result->badDevices);
    ClearQueue(&result->badNets);
    return (result);
}

/** 
 * InitCharacterTranslation -- initialize the character translation for 
 * string equivalence functions.
 *
 * @param imEnv is the working environment
 */
static void initCharacterTranslation(IMEnv * imEnv) {
    register int i;

    if (imEnv->ignoreCase) /* set up pointer to comparison function */
        imEnv->streq = casestreq;
    else
        imEnv->streq = nocasestreq;
    for (i = 0; i < 256; i++) {
        if (imEnv->ignoreCase && (i >= 'A') && (i <= 'Z')) {
            imEnv->translate[i] = tolower(i);
        } else {
            imEnv->translate[i] = i;
        }
    }
}

/* ---------------------------------------------------------------------------- */

/* 
 * Net vertices.
 */

static Net *netBuffer = NULL;
static int netCount = 0;

#define NET_INCREMENT 256

/**
 * InitializeNetAlloc -- Initialize the net allocation functions
 */
static void initializeNetAlloc() {
    netBuffer = NULL;
    netCount = 0;
}

/**
 * NewNet -- Create a new net. Allocate nets in chunks
 * that way we avoid memory fragmentation somewhat.
 *
 * @param imEnv is the compare environment
 * @param name is the name of the net
 * @param index is the index
 */
Net * newNet(IMEnv * imEnv, char *name, int index) {
    register Net *result;

    if (netCount <= 0) {
        netBuffer =
                (Net *) fastAlloc((unsigned) NET_INCREMENT * sizeof (Net));
        if (netBuffer == NULL)
            throwError(imEnv,
                "NewNet unable to allocate sufficient memory\n");
        netCount = NET_INCREMENT;
    }

    result = netBuffer++;
    netCount--;
    result->name = copyString(name);
    result->index = index;
    result->numConnects = 0;
    result->connections.list = NULL;
    result->next = NULL;
    result->nextInOrder = NULL;
    return result;
}

/* ---------------------------------------------------------------------------- */

/*
 * Net/Device connections
 */

static NetDevConnect *conBuffer = NULL;
static int conCount = 0;

#define CON_INCREMENT 256

/**
 * InitializeNetDevAlloc
 */
static void initializeNetDevAlloc() {
    conBuffer = NULL;
    conCount = 0;
}

/**
 * NewConnection -- allocate room for a new connections. We are allocating 
 * larger chunks of memory for this so we avoid fragmentation somewhat.
 */
NetDevConnect * newConnection(IMEnv *imEnv) {
    register NetDevConnect *result;

    if (conCount <= 0) {
        conBuffer = (NetDevConnect *)
                fastAlloc((unsigned) CON_INCREMENT * sizeof (NetDevConnect));
        if (conBuffer == NULL) {
            throwError(imEnv, "Not enough memory for net/device connections");
        }
        conCount = CON_INCREMENT;
    }

    result = conBuffer++;
    conCount--;
    return result;
}

/**
 * InitializeIMEnv -- Initialize the IM environment.
 * This is the main initialization function.
 *
 * @return an initialize compare environment
 */
IMEnv *g_imEnv;

IMEnv * initializeIMEnv() {
    IMEnv *imEnv = (IMEnv *) fastAlloc(sizeof (IMEnv));

    g_imEnv = imEnv;

    memset((void *) imEnv, 0, sizeof (IMEnv));

    imEnv->verbose = FALSE;
    imEnv->trace = FALSE;
    imEnv->useSuffix = TRUE;
    imEnv->ignoreCase = FALSE;
    imEnv->deduceNeighbors = DEDUCEHTSIZE / 10;
    imEnv->printZeroNets = TRUE;

    imEnv->hashSize = 0;
    imEnv->maxHashSize = 0;

    imEnv->noOpt = FALSE;

    imEnv->netPrintLimit = 10;
    imEnv->errors = 0;
    imEnv->matchedCount = 0;
    imEnv->forcedMatch = 0;
    imEnv->findMatch = TRUE;
    imEnv->errorCutOff = 0;
    imEnv->suspectCutOff = 0;
    imEnv->noProgressCutOff = 2;

    imEnv->numDeviceDefs = 0;
    imEnv->pass = 0;
    imEnv->passType = NET;

    imEnv->deducedMatches = 0;
    imEnv->numUserDefs = 0;

    imEnv->runComplete = FALSE;

    initCharacterTranslation(imEnv);
    initializeDeviceDefinitions(imEnv);
    initializeNetAlloc();
    initializeNetDevAlloc();
    initEqHash(1000);

    // reset the work area for the two graphs
    for (int i = 0; i < 2; i++) {
        static const char *defaultGraphName[2] = {"Graph1", "Graph2"};

        imEnv->__graphEnv[i].graph = allocGraph();
        imEnv->__graphEnv[i].graph->graphName = (char*) defaultGraphName[i];
        imEnv->__graphEnv[i].devices = NULL;
        imEnv->__graphEnv[i].nets = NULL;
        imEnv->__graphEnv[i].pIndex = 0;
        imEnv->__graphEnv[i].netIndex = 0;
        imEnv->__graphEnv[i].firstDevice = NULL;
        imEnv->__graphEnv[i].lastDevice = NULL;
        imEnv->__graphEnv[i].firstNet = NULL;
        imEnv->__graphEnv[i].lastNet = NULL;
        imEnv->__graphEnv[i].hashSize = 0;
        imEnv->__graphEnv[i].hashTable = NULL;
        imEnv->graphEnv = &imEnv->__graphEnv[i];
        initHash(imEnv, 11);

    }
    imEnv->graphEnv = NULL;

    return (imEnv);
}

/**
 * CopyString -- allocate space for and save a copy of a string.
 *
 * @param string is the string we are making a copy of
 * @return the copied string
 */
char * copyString(char *string) {
    char *save;

    if (string == NULL)
        return (NULL);
    save = fastAlloc(strlen(string) + 1);
    strcpy(save, string);
    return (save);
}

/**
 * casestreq -- compare strings using a translation table
 *
 * @param imEnv is the working environment
 * @param s1 is one of the strings we are comparing
 * @param s2 is the other string
 * @return TRUE/FALSE depending on whether we match
 */
int casestreq(IMEnv * imEnv, char *s1, char *s2) {
    while (*s1 || *s2) {
        if (*s1 != *s2) {
            /*
             * Assume case matches 
             */
            if ((imEnv->translate)[(unsigned int) *s1] != (imEnv->translate)[(unsigned int) *s2])
                return FALSE;
        }
        s1++, s2++;
    }
    return TRUE;
}

/**
 * nocasestreq -- compare strings. We are using the same interface
 * as casestreq because we are using the function through an indirection.
 *
 * @param imEnv is the working environment
 * @param s1 is one of the strings we are comparing
 * @param s2 is the other string
 * @return TRUE/FALSE depending on whether we match
 */
int nocasestreq(IMEnv * imEnv, char *s1, char *s2) {
    while (*s1 || *s2) {
        if (*s1++ != *s2++)
            return FALSE;
    }
    return TRUE;
}

/**
 * NewVertexVector -- Create a new vertex vector
 *
 * @param size is the size of the vector
 * @return the vertex vector
 */
Vertex * newVertexVector(IMEnv * imEnv, int size) {
    Vertex *vertexVector = (Vertex *) fastAlloc(size * sizeof (Vertex));

    if (vertexVector == NULL)
        throwError(imEnv,
            "Unable to allocate sufficient memory for %d vertices.",
            size);
    for (int i = 0; i < size; i++) {
        vertexVector[i].match = NULL;
    }
    return (vertexVector);
}

/**
 * NewDevice -- Allocate a Device.
 * 
 * @param index postion
 * @param deviceType is the type of device
 * @param numpins are the number of pins on the device
 */
Device * newDevice(IMEnv *imEnv, int index, int deviceType, int numpins) {
    register Device *result;
    register int i;

    /*
     * to accomodate user-defined devices with more than 4 terminals,
     * allocate extra space for pointers (one pointer per terminal)
     * Assume there are always at least 4 terminals.
     */
    if (numpins < 4)
        numpins = 4;
    result = (Device *) fastAlloc(sizeof (Device) +
            (sizeof (void *) * (numpins - 4)));
    if (result == NULL) {
        throwError(imEnv, "Not enough memory for a new device");
    }
    result->index = index;
    result->type = deviceType;
    result->next = NULL;
    for (i = 0; i < numpins; i++)
        result->connections[i] = NULL;
    return result;
}
