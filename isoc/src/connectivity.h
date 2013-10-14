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
 * This file contains data definitions and declarations used to
 * represent the connectivity. The connectivity is created between
 * Devices and Nets with NetDevConnects.
 *
 * Devices are a linked list.  Each has an index, a pointer to a 
 * net for each terminal and a next pointer to the next device in the
 * list.  The devices are in index order.
 */
#define NUMBERTYPES 0

typedef struct Device {
    int index;
    int type;

    /** callback user object */
    uint64_t instObject;

    struct Device *next;
    struct Net * connections[4];
} Device;

/**
 * Connections to a device must give the device and terminal number 
 */
typedef struct NetDevConnect {
    int dev; /* Index of device to which net is connected */
    short unsigned int terminal; /* Terminal index of device for connection */
    short unsigned int devClass; /* Class of terminal */
    struct NetDevConnect *next; /* Next connection in list */
} NetDevConnect;

/**
 * The structure for net contains the information on each net.  Nets are kept
 * in the hash table.
 */
typedef struct Net {
    char *name;
    int index; /* Index (0 based) assigned to net */
    int numConnects; /* Number of connections: -1 ==> equated net */

    union {
        NetDevConnect *list; /* List of net connections */
        struct Net *equalNet; /* Net that is equal to this one */
    } connections;

    struct Net *next; /* Link nets in one hash bucket */
    struct Net *nextInOrder; /* For list of all nets in index order */
} Net;

/**
 * return the index of the net or the index of the alias
 *  if the index is -1.
 */
#define IndexOfNet(net)						\
	( ( (net)->index == -1) ? realNet(net)->index		\
	  : (net)->index)

#define nextarg() while (spaces[*line]) line++; arg = line; \
 while (*line && (spaces[*line] == 0)) line++; *line++ = '\0';

#define argerror() if (*arg == '\0') { \
 fprintf(stderr, "File format error: line %d\n", lineNumber); \
break;  }
