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
 * This file maintains the hash table used to keep track of net names.
 */

#include "compare.h"

/**
 * InitHash -- Allocate a hash table with the appropriate size.
 *
 * @param imEnv is the compare environment
 * @param size is the size we are allocating
 */
void initHash(IMEnv * imEnv, int size) {

    /*
     * If the table has been already allocated, just reuse on the assumption it 
     * has about the right size 
     */
    if (imEnv->graphEnv->hashTable == NULL) {
        imEnv->graphEnv->hashTable =
                (Net **) fastAlloc((unsigned) size * sizeof (Net *));
        imEnv->graphEnv->hashSize = size;
    }
    size = imEnv->graphEnv->hashSize;
    while (size)
        imEnv->graphEnv->hashTable[--size] = NULL;

}

/**
 * popUpIndex -- Keep track of net indices.  When two nets are equated, 
 * all of the indices  of the nets following in order are pushed up 
 * (decremented) to fill in the gap.
 *
 * @param imEnv is the compare environment
 * @param net from which we start
 */
static void popUpIndex(IMEnv * imEnv, Net * net) {
    register Net *p;

    for (p = net; p != NULL; p = p->nextInOrder) {
        if (p->index == 0)
            throwError(imEnv,
                "Fatal internal error: changing net index from 0 to -1\n");

        if (p->index > 0)
            p->index--;
    }
    imEnv->graphEnv->netIndex--;
}

/**
 * nethash -- calculate a hash value for a net name
 *
 * @param imEnv is the compare environment
 * @param name is the net name
 * @return the hash value
 */
static int nethash(IMEnv * imEnv, char *name) {
    register int value;
    register char *p = name;

    value = 0;
    if (imEnv->ignoreCase)
        while (*p)
            value = (value << 1) + imEnv->translate[(int)*p++];
    else
        while (*p)
            value = (value + 337351) * *p++;
    value = (((unsigned int) value) % imEnv->graphEnv->hashSize);
    return (value);
}

/**
 * RealNet -- Returns the net, with non negative index that 
 * this net is equated to.
 *
 * @param net is the net we are investigating
 * @return the equated net
 */
Net * realNet(Net * net) {
    //    assert (net != NULL, "RealNet, Null Net passed.  FATAL!!!");
    while (net->index == -1) {
        net = net->connections.equalNet;
    }
    return net;
}

#define newIndex(imEnv) imEnv->graphEnv->netIndex++

/**
 * FindNet -- Finds the named net in the hashtable and returns in.
 * NULL is returned if not found.
 *
 * @param imEnv is the compare environment 
 * @param name is the net name we are trying to locate
 * @return the net (or NULL)
 */
Net * findNet(IMEnv * imEnv, char *name) {
    register int hashValue;
    register Net *netP;

    hashValue = nethash(imEnv, name);

    for (netP = imEnv->graphEnv->hashTable[hashValue]; netP != NULL;
            netP = netP->next) {
        if ((*imEnv->streq) (imEnv, netP->name, name)) {
            while (netP->index == -1)
                netP = netP->connections.equalNet;
            return (netP);
        }
    }
    return (NULL);
}

/**
 * FindOrAllocNet -- Finds the named net in the hashtable and returns the Net.
 * If the name is not found, then one is allocated and assigned a new index
 * Aliased names are always dealiased first.
 *
 * @param imEnv is the compare environment
 * @param name is name of the net we are dealing with
 * @return the net
 */
Net * findOrAllocNet(IMEnv * imEnv, char *name) {
    register int hashValue;
    register Net *netP;

    netP = findNet(imEnv, name);
    if (netP == NULL) {
        netP = newNet(imEnv, name, newIndex(imEnv));
        hashValue = nethash(imEnv, name);
        netP->next = imEnv->graphEnv->hashTable[hashValue];
        imEnv->graphEnv->hashTable[hashValue] = netP;
        if (imEnv->graphEnv->firstNet == NULL) {
            imEnv->graphEnv->firstNet = imEnv->graphEnv->lastNet = netP;
        } else {
            imEnv->graphEnv->lastNet->nextInOrder = netP;
            imEnv->graphEnv->lastNet = netP;
        }
    }
    return (netP);
}

/**
 * DeleteNet -- Delete a net in the netVector by replacing it 
 * by the last net in the vector.
 * @param imEnv is the compare environment 
 * @param graph is the graph we want to delete a net from
 * @param net is the net to be deleted
 */
void deleteNet(IMEnv * imEnv, Graph * graph, Vertex * net) {
    register Vertex *lastNet = &graph->netVector[--graph->numNets];
    register Vertex *device;
    register int d,
            n;

    /* For each device connected to the net:
        For each net conneted to the net:
          If the net is the same as the orignal net, swing the pointer.
     */
    for (d = 0; d < NumberOfLinksN(lastNet); d++) {
        device = lastNet->connects.devList[d].vertex;
        for (n = 0; n < NumberOfLinksD(imEnv, device); n++) {
            if (device->connects.netList[n] == lastNet) {
                device->connects.netList[n] = net;
            }
        }
    }
    *net = *lastNet; /* Move entire structure */
}

/**
 * EquateNets -- Equate two nets:  The index of one of the nets may have 
 * to be reused. On return, each net will either be aliased to a 
 * non-aliased net OR will be a normal net and not aliased at all.
 *
 * @param imEnv is the compare environment
 * @param graph is the graph the equivalence is applied to
 * @param net1 is the net to be aliased
 * @param net2 is the alias net
 */
static void equateNets(IMEnv * imEnv, Graph * graph, Net * net1, Net * net2) {

    /*
     * Make sure first that the nets are distinct 
     */
    if (net1 == net2)
        return;

    /*
     * Check first if nets are already aliased and find root net 
     */
    while (net1->index == -1)
        net1 = net1->connections.equalNet;

    while (net2->index == -1)
        net2 = net2->connections.equalNet;

    /*
     * If the eq file refers to one of the equate names, make sure we keep that
     * one since it will be the one the user wants to see 
     */
    if (findEqName(imEnv, net2->name, graph->graphNumber)) {
        Net *tmp = net1; /* Interchange nets */

        net1 = net2;
        net2 = tmp;
    }

    /*
     * Alias the second net to the first net 
     */
    if (net2->connections.list == NULL)
        ;
    else {
        register NetDevConnect *conP;

        if (net1->connections.list == NULL) {
            net1->connections.list = net2->connections.list;
            net1->numConnects = net2->numConnects;
        } else {
            /*
             * Find end of list 
             */
            for (conP = net1->connections.list;
                    conP->next != NULL; conP = conP->next);
            /*
             * Append net connections 
             */
            conP->next = net2->connections.list;
            net1->numConnects += net2->numConnects;
        }
    }
    net2->numConnects = -1;
    /*
     * Attach alias 
     */
    net2->connections.equalNet = net1;
    popUpIndex(imEnv, net2);
    net2->index = -1;
}

/**
 * DefineNetAlias -- Define net alias names within a graph
 *
 * @param imEnv is the compare environment
 * @param name is the net name to be aliased
 * @param size is the size of the array for net aliases
 * @param aliases is an array with the net alias names
 */
void defineNetAlias(IMEnv * imEnv, char *name, int size, char *aliases[]) {
    Net *net1 = findOrAllocNet(imEnv, name);

    for (int i = 0; i < size; i++) {
        Net *net2 = findOrAllocNet(imEnv, aliases[i]);

        equateNets(imEnv, imEnv->graphEnv->graph, net1, net2);
    }
}
