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
 * Structure declarations for the graphs used by Compare
 *
 * Indicate the vertex types. If this is changed, thje Java code has
 * to change as well.
 */
#define DEVICE 0                /* Vertex of type device */
#define NET 1                   /* Vertex of type net */

#define MAXDEVICETYPES 100000   /* Maximum number of device types */

/**
 * Reporting categories. If this is changed, the JAVA code has to be 
 * updated as well. (com/uwemeding/connectivity/CompareEvent)
 */
#define RPT_GENERAL           0
#define RPT_MATCH             1
#define RPT_BAD               2
#define RPT_NO_MATCH_OTHER    3
#define RPT_NO_MATCH_SYMMETRY 4

/**
 * Vertex flags
 */
#define PENDING 0
#define UNIQUE 	0x01
#define SUSPECT	0x02
#define BAD	0x04
#define MATCHING 0x08           /* Will be matched next time around */
#define DELETED 0x10            /* Deleted by chain compression */

#define suspectVertex(vertex) 	(((vertex)->flag)==SUSPECT)
#define uniqueVertex(vertex)	(((vertex)->flag)==UNIQUE)
#define badVertex(vertex)		(((vertex)->flag)==BAD)
#define pendingVertex(vertex)	(((vertex)->flag)==PENDING)

#define setSuspectVertex(imEnv, vertex)	\
   imEnv->errors++;			\
   vertex->flag = SUSPECT;	\
   vertex->vertexValue = 0;

#define setBadVertex(imEnv,vertex)	\
   imEnv->errors++;			\
   vertex->flag = BAD;		\
   vertex->vertexValue = 0;

/**
 * Terminal classes are small integers starting at 0
 */
typedef unsigned short TermClass;

/**
 * A device definition defines the number and type of connections of the
 * device.  The device ID is simply the index of the definition in the
 * array of device definitions: DeviceDefs
 */
typedef struct DeviceDefinition {
    /** Name of device type */
    char *name;

    /** Number of terminals on device */
    int numTerminals;

    /** Vector of terminal classes */
    TermClass *terminals;

    /** occurence counter */
    int occ[2];
} DeviceDefinition;

/**
 * Connections to a device must give the device and terminal number 
 */
typedef struct {
    /** Device to which net is connected */
    struct Vertex *vertex;

    /** Terminal index of device for connection */
    unsigned short int terminal;

    /** Class of terminal of device connection */
    unsigned short int devClass;
} DeviceConnection;

/**
 * Each device/net in the network is represented by a Vertex
 */
typedef struct Vertex {
    /** Name of vertex */
    char *name;

    /** Hashed value for vertex */
    unsigned int vertexValue;

    /** Device: user object */
    uint64_t userObject;

    union NUnion {
        /** Device: device definition index */
        int vertexDef;

        /** Net: number of connections */
        int netConnects;
    } n;

    /** Connect Union */
    union ConnectUnion {
        /** Device: array of net connections */
        struct Vertex **netList;

        /** Net: array of device connections */
        DeviceConnection *devList;
    } connects;

    /** Matched vertex */
    struct Vertex *match;
    int matched;

    /** Link used to keep vertices in queues and lists,
     * eg. the hash table.
     */
    struct Vertex *next;

    /** For unique vertices: the pass in which they
     * became unique. For non-unique vertices:
     * the pass in which they were put in the
     * queue.  This is used to determine if the
     * vertex is in the queue already in order not to
     * queue it twice.
     */
    short int pass;

    /** Device or Net */
    char vertexType;

    /** Flags vertex category: Unique, suspect, etc. */
    char flag;

    /** So we know how many in section */
    int sectionSize;
} Vertex, *VertexPt;

/**
 * Returns the number of links the vertex has.
 */
#define NumberOfLinksN(net) ((net)->n.netConnects)
#define NumberOfLinksD(imEnv,dev) (imEnv->deviceDefs[(dev)->n.vertexDef].numTerminals)
#define NumberOfLinks(imEnv,aVertex)	\
(((aVertex)->vertexType == DEVICE) ? NumberOfLinksD(imEnv,aVertex) : NumberOfLinksN(aVertex))

#define TerminalList(imEnv,dev) (imEnv->deviceDefs[(dev)->n.vertexDef].terminals)

/**
 * Each bucket in the hash table contains two disjoint lists of vertex entries.
 */
typedef struct Bucket {
    /** Check sum by bucket */
    int bucketSum;

    /** Smallest of non-singleton sections */
    int minPartSize;

    /** List of non-unique entries */
    Queue notUnique;

    /** Queue of unique entries */
    Queue Unique;

    /** Queue of 'extra' nonUnique entries */
    Queue overflow;
} Bucket;

/**
 * The Graph structure contains all the information for one graph.
 */
typedef struct Graph {
    /** First or second graph on command line */
    char graphNumber;

    /** Graph name */
    char *graphName;

    /** graph has private hash table */
    Bucket *hashTable;

    /** check sum of bucket check sums */
    int checkSum;

    /** Vertices that became unique in the 
     * current pass
     */
    Queue newUniques;

    /** List of vertices to be evaluated in the next 
     * pass
     */
    Queue evaluationQueue;

    /** The pass number when a unique vertex was last
     * found: used to detect progress 
     */
    int lastUniquePass;

    /** Nets and devices that are still pending */
    Queue devices,
            nets;

    /** Graph vertices are in two vectors */
    Vertex *deviceVector,
            *netVector;

    /** Length of the two vectors */
    int numDevices,
            numNets;

    /**
     * Vector of devices and nets currently
     * being processed.  Unique, bad, and suspect
     * are sometimes present, but are weeded out
     * periodically 
     */
    VertexPt *pendingDevices,
            *pendingNets;

    /** Number of pending nets/devices in the remainingDevices/Nets vectors */
    int numPendingDevices,
            numPendingNets;

    /** Nets and devices already found unique */
    Queue uniqueDevices,
            uniqueNets;

    /** Suspect vertices */
    Queue suspectDevices,
            suspectNets;

    /** Bad vertices: unique but do not match */
    Queue badDevices,
            badNets;
} Graph;

#define PendingNetsAreClean(graph)		\
(graph->numNets == graph->numPendingNets + graph->suspectNets.size + 	\
  		   graph->badNets.size + graph->uniqueNets.size)

#define PendingDevicesAreClean(graph)		\
(graph->numDevices == graph->numPendingDevices + graph->suspectDevices.size + \
  		      graph->badDevices.size + graph->uniqueDevices.size)
