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

#define NUMTERMINALS 10000
#define LINESIZE 80

/* data structures */
typedef struct TAGGED_TYPE {
    long hval;
    short index;
    short weight;
    char *string;
}
TAGGED_TYPE;

/**
 * SimpleHash
 * this is a simple hash function that uses only xors and shifts
 */
unsigned long
SimpleHash(char *buf) {
    short i;
    unsigned long h1 = 0,
            h2 = 0;

    for (i = 0; buf[i]; i++) {
        h1 ^= (unsigned long) toupper(buf[i]);
        h1 <<= 1;
        h2 ^= (unsigned long) toupper(buf[i]);
    }
    return h1 ^ (h2 << 16);
}

/*
 * comphval and compindex are used by qsort to sort the argument list
 * in the master vertex definition.  If the strings hash to the same value,
 * the strings are compared to ensure that the strings are indeed equal
 * and not the result of a hash collision.
 * If they hash to different values, they must be different.
 */
static int
comphval(const void *xv, const void *yv) {
    TAGGED_TYPE *x = (TAGGED_TYPE *) xv;
    TAGGED_TYPE *y = (TAGGED_TYPE *) yv;
    int r = x->hval - y->hval;

    if (r)
        return (r);
    return (STRNCASECMP((char *) x->string, (char *) y->string, LINESIZE));
}

static int
compindex(const void *xv, const void *yv) {
    TAGGED_TYPE *x = (TAGGED_TYPE *) xv;
    TAGGED_TYPE *y = (TAGGED_TYPE *) yv;

    return (x->index - y->index);
}

/**
 * AddDevice -- Add a device to the linked list of devices.
 *
 * @param imEnv is the compare environment
 * @param device is the device
 */
static void
addDevice(IMEnv * imEnv, Device * device) {
    if (imEnv->graphEnv->firstDevice == NULL) {
        imEnv->graphEnv->firstDevice = imEnv->graphEnv->lastDevice = device;
    } else {
        imEnv->graphEnv->lastDevice->next = device;
        imEnv->graphEnv->lastDevice = device;
    }
}

/**
 * InitializeDeviceDefinitions -- Initialize the device processing.
 *
 * @param imEnv is the compare environment
 */
void
initializeDeviceDefinitions(IMEnv * imEnv) {
    unsigned int ndevices;

    if (imEnv->numDeviceDefs)
        return; /* Only do this once!! */
    imEnv->numDeviceDefs = NUMBERTYPES;
    ndevices = (unsigned) MAXDEVICETYPES * sizeof (DeviceDefinition);

    imEnv->deviceDefs = (DeviceDefinition *) fastAlloc(ndevices);

}

/**
 * DefineDeviceMaster -- Define the master vertices for the vertices. These
 * are the master views (typically symbols) of all the components we intent
 * to use in a circuit.
 *
 * @param imEnv is the compare environment
 * @param name is the name of the master vertex (typically the full cell name)
 * @param size is the size of the connections array
 * @param connections is an array of connections this master defines
 */
void
createDefineDeviceMaster(IMEnv * imEnv, char *name, int size, char *connections[]) {
    TAGGED_TYPE *sortarray;
    int i = 0; /* all purpose index variables */
    int j = 0;
    int ac; /* arg count for pins; ac=numargs-2 */
    int nn;

    sortarray =
            (TAGGED_TYPE *) fastAlloc(NUMTERMINALS * sizeof (TAGGED_TYPE));

    if (imEnv->numDeviceDefs >= MAXDEVICETYPES)
        throwError(imEnv, "Too many user-defined devices.\n");

    if (strlen(name) < 2)
        throwError(imEnv,
            "User-defined device type '%s' illegal, must have 2 or more characters.",
            name);

    for (ac = 0; ac < size; ac++) {
        sortarray[ac].string = connections[ac];
        sortarray[ac].weight = ac;
        sortarray[ac].index = ac;
        sortarray[ac].hval = SimpleHash(sortarray[ac].string);
    }

    /*
     * sort the array to put similar strings next to each other 
     */
    qsort(sortarray, ac, sizeof (TAGGED_TYPE), comphval);

    /*
     * if the strings in adjacent entries match,
     * make sure the weights are identical
     */
    for (i = 0; i < ac - 1; i++) {
        if (!comphval((void *) &sortarray[i], (void *) &sortarray[i + 1]))
            sortarray[i + 1].weight = sortarray[i].weight;
    }

    /*
     * sort the array by index, to put it back into the original order 
     */
    qsort(sortarray, ac, sizeof (TAGGED_TYPE), compindex);

#ifdef DEBUG
    /*
     * print weights to verify proper functionality 
     */
    printf("Device Master Cell '%s': ", name);
    for (i = 0; i < ac; i++)
        printf("%d ", sortarray[i].weight);
    printf("\n");
#endif

    nn = imEnv->numUserDefs + NUMBERTYPES;

    /*
     * perform linear search through DeviceDefs to match device name 
     */
    for (i = NUMBERTYPES; i < nn; i++) {
        if (STRNCASECMP(name, imEnv->deviceDefs[i].name, LINESIZE)) {
            continue;
        } else { /* got a match: verify */
            if (imEnv->deviceDefs[i].numTerminals != ac)
                throwError(imEnv,
                    "Inconsistent pin counts for user-defined type '%s'.\n",
                    name);

            for (j = 0; j < ac; j++) {
                if (imEnv->deviceDefs[i].terminals[j] != sortarray[j].weight)
                    throwError(imEnv,
                        "Inconsistent pin names for user-defined type '%s'.\n",
                        name);
            }
        }
        freeSome((char *) sortarray);
        return; /* good match, no errors */
    }

    imEnv->deviceDefs[nn].name = copyString(name);
    imEnv->deviceDefs[nn].numTerminals = ac;
    imEnv->deviceDefs[nn].terminals = (TermClass *)
            fastAlloc((unsigned)
            imEnv->deviceDefs[nn].numTerminals * sizeof (TermClass));
    for (i = 0; i < ac; i++) {
        imEnv->deviceDefs[nn].terminals[i] = sortarray[i].weight;
    }

    fireProgressEvent(imEnv, "Defined device master cell '%s', %d %s", name,
            ac, ac == 1 ? "pin" : "pins");

    imEnv->numDeviceDefs++; /* to keep consistent with chains.c */
    imEnv->numUserDefs++;
    freeSome((char *) sortarray);
}

/**
 * createDefineDeviceVertex -- defines a device vertex instance. Note: the master
 * for this instance must be defined before usage.
 *
 * @param imEnv is the compare environment
 * @param name is the name of the master we are using (instantiating)
 * @param size is the size of the array of the connecting nets
 * @param connections are the names of the connecting nets
 */
void
createDefineDeviceVertex(IMEnv * imEnv, char *name, uint64_t instObject, int size,
        char *connections[]) {
    int nn = imEnv->numUserDefs + NUMBERTYPES;
    int i;
    int devtype;
    Device *device;
    Net *net;
    NetDevConnect *connect;

    if (imEnv->graphEnv->pIndex == 0) {
        initializeDeviceDefinitions(imEnv);
    }
    for (devtype = NUMBERTYPES; devtype < nn; devtype++) {
        if (!STRCASECMP(name, imEnv->deviceDefs[devtype].name)) {
            if (size != imEnv->deviceDefs[devtype].numTerminals)
                throwError(imEnv,
                    "%d nets declared for %d-pin device '%s'.\n",
                    size,
                    imEnv->deviceDefs[devtype].numTerminals, name);

            goto got_a_match;
        }
    }

    throwError(imEnv, "Device '%s' not defined.\n", name);

got_a_match:
    device = newDevice(imEnv, imEnv->graphEnv->pIndex, devtype, size);
    device->instObject = instObject;
    addDevice(imEnv, device);
    for (i = 0; i < size; i++) {
        net = findOrAllocNet(imEnv, connections[i]);
        device->connections[i] = net;
        while (net->index < 0) {
            net = net->connections.equalNet;
        }
        connect = newConnection(imEnv);
        connect->dev = imEnv->graphEnv->pIndex;
        connect->terminal = i;
        connect->next = net->connections.list;
        net->connections.list = connect;
        net->numConnects++;
    }

    imEnv->graphEnv->pIndex++;

    // update the working structure
    imEnv->graphEnv->graph->numDevices = imEnv->graphEnv->pIndex;
    imEnv->graphEnv->graph->numNets = imEnv->graphEnv->netIndex;
    imEnv->graphEnv->devices = imEnv->graphEnv->firstDevice;
    imEnv->graphEnv->nets = imEnv->graphEnv->firstNet;
}
