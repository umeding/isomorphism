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
 * These structures maintain a simple hash table that is used to equate net names
 * via a equivalence names.
 */
#include "compare.h"

typedef struct eqNamesStruct {
    char *name; /* The name equated */
    short circuit; /* Which circuit? */
    short used; /* Flags whether name was used */
    unsigned int hashValue; /* The hash value for speed */
    int value; /* The random value of the name */
    struct eqNamesStruct *next;
} EquateNames;

static EquateNames **hashTable;
static int hashSize;
static int anyEquates = FALSE; /* Turn on if there is an equate file */

/**
 * InitEqHash -- Initialize the equivalence hash table
 *
 * @param size is the size of the hash table
 */
void initEqHash(int size) {
    hashTable = (EquateNames **) fastAlloc((unsigned) size * sizeof (EquateNames *));
    hashSize = size;
    while (size) {
        hashTable[--size] = NULL;
    }
    anyEquates = TRUE;
}

/**
 * hash -- Calculate a hash value for a name.
 *
 * @param name is the name
 * @return the hash value
 */
unsigned int hash(IMEnv * imEnv, char *name) {
    register unsigned int value = 0;

    if (imEnv->ignoreCase) {
        while (*name)
            value = (value << 1) + imEnv->translate[(int)*name++];
    } else {
        while (*name)
            value = (value << 1) + *name++;
    }
    return (value % hashSize);
}

/** 
 * InsertEqName -- Insert an equivalence name into the hash tables.
 *
 * @param imEnv is the compare environment
 * @param name is the name we are inserting
 * @param circuit is the circuit it applies to
 * @param value of the assignment
 * @return TRUE if the insert succeeded, FALSE otherwise
 */
int insertEqName(IMEnv * imEnv, char *name, int circuit, int value) {
    register unsigned int hashValue = hash(imEnv, (char *) name);
    register EquateNames *p;

    debug(DBG_EQUATE, printf("Inserting %s into %d:", name, circuit);
            );

    for (p = hashTable[hashValue]; p != NULL; p = p->next) {
        if ((p->hashValue == hashValue) &&
                (p->circuit == circuit) && (*imEnv->streq) (imEnv, p->name, name)) {
            /*
             * Insert failed 
             */
            return (FALSE);
        }
    }
    p = (EquateNames *) fastAlloc((unsigned) sizeof (EquateNames));
    p->next = hashTable[hashValue];
    hashTable[hashValue] = p;
    p->hashValue = hashValue;
    p->name = copyString((char *) name);
    p->circuit = circuit;
    p->used = FALSE;
    p->value = value;
    debug(DBG_EQUATE, printf("OK\n");
            );

    /*
     * Insert succeeded 
     */
    return (TRUE);
}

/**
 * FindEqName -- Find an equated name in a circuit
 *
 * @param imEnv is the compare environment
 * @param name is the name we are looking for
 * @param circuit is the circuit it applies to
 * @return the value of the equated name
 */
int findEqName(IMEnv * imEnv, char *name, int circuit) {
    register unsigned int hashValue;
    register EquateNames *p;

    if (!anyEquates)
        return (FALSE);

    debug(DBG_EQUATE, printf("Finding %s in %d:", name, circuit););

    hashValue = hash(imEnv, (char *) name);
    for (p = hashTable[hashValue]; p != NULL; p = p->next) {
        if ((p->hashValue == hashValue) &&
                (p->circuit == circuit) && (*imEnv->streq) (imEnv, p->name, name)) {
            /*
             * Name was touched 
             */
            p->used = TRUE;
            debug(DBG_EQUATE, printf("OK\n"););
            return (p->value);
        }
    }
    debug(DBG_EQUATE, printf("Not found\n"););
    /*
     * No value 
     */
    return (FALSE);
}

/**
 * CheckEquates -- Check whether all the equated names were used.
 */
void checkEquates(IMEnv * imEnv) {
    register int i;
    register EquateNames *p;

    for (i = 0; i < hashSize; i++) {
        for (p = hashTable[i]; p != NULL; p = p->next) {
            if (!p->used) {
                fireWarningEvent(imEnv,
                        "Equivalence name name \"%s\" from circuit %d not used\n",
                        p->name, p->circuit);
            }
        }
    }
}

/** 
 * DefineEquate -- Equate net names in the graphs.
 *
 * @param imEnv the compare environment
 * @param name1 the net name in the 1st graph
 * @param name2 the net name in the 2nd graph
 */
void defineEquate(IMEnv * imEnv, char *name1, char *name2) {
    int randValue = Random();

    if (!insertEqName(imEnv, name1, 1, randValue))
        fireWarningEvent(imEnv,
            "Duplicate equivalence name for circuit 1: %s\n",
            name1);
    if (!insertEqName(imEnv, name2, 2, randValue))
        fireWarningEvent(imEnv,
            "Duplicate equivalence name for circuit 2: %s\n",
            name2);
}
