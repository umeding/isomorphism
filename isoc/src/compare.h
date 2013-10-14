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
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

// The JAVA stuff
#include <jni.h>

/*
 * Include the machine specific stuff
 */
#include "md.h"

#include "queue.h"
#include "graph.h"
#include "debug.h"
#include "connectivity.h"
#include "env.h"

#undef NOSTATIC

#define BUFSIZE 30000
#define MAXNAMELEN 256
#define WHITESPACE (char *) " \n\t"

#define FALSE 	0
#define TRUE 	!FALSE

#define MAXINT	0x7fffffff
#define MININT  0x80000000
#define Max(a,b) ((a > b) ? a : b)

/**
 * long long int = 64 bit integer
 */

#ifdef _LONGLONG
typedef unsigned long long bigint;
#else
typedef unsigned long bigint;
#endif

/**
 * Toggle the type of vertex we are looking at
 */
#define TOGGLE_TYPE(type)	(type == DEVICE ? NET : DEVICE)

/**
 * Calculate the number of vertices that have not been uniquely labelled.
 */
#define NUM_VERTICES_LEFT(graph) \
	(((graph)->numDevices - (graph)->uniqueDevices.size) + \
	 ((graph)->numNets - (graph)->uniqueNets.size))

/**
 * Determine whether graph has been completely uniquely labelled.
 */
#define DONE_GRAPH(graph) \
	(DONE_NETS(graph) && (DONE_DEVICES(graph)))

/**
 * Determine if nets are all uniquely labelled
 */
#define DONE_NETS(graph)	\
	( (graph)->uniqueNets.size == (graph)->numNets )

/**
 * Determine if devices are all uniquely labelled
 */
#define DONE_DEVICES(graph) \
	( (graph)->uniqueDevices.size == (graph)->numDevices )

/**
 * Print relevant statistics on the two graphs for debugging purposes
 */
#define PRINT_GRAPH_STATS(graph) \
  printf("Graph %s:\n", graph->graphName); \
  printf("%d nets, %d pending, %d suspect, %d bad, %d unique\n",\
         graph->numNets, \
	 graph->numPendingNets, \
	 graph->suspectNets.size,\
         graph->badNets.size, \
	 graph->uniqueNets.size);\
  printf("%d devices, %d pending, %d suspect, %d bad, %d unique\n",\
         graph->numDevices, \
	 graph->numPendingDevices, \
	 graph->suspectDevices.size,\
         graph->badDevices.size, \
	 graph->uniqueDevices.size);\
  printf("%d vertices in evaluationQueue\n", graph->evaluationQueue.size);

/**
 * The hash table sizes must be the same for all graphs
 */
#define HASHRATIO 4             /*  # elements / # buckets in hash table */
#define MIN_NUM_BUCKETS 1       /*  Minimum number of buckets in the hash table */
#define MAX_NUM_BUCKETS 10000000 /*  Maximum number of buckets in the table */
#define DEDUCEHTSIZE 310

/**
 * Set upt he prototype definitions for all the functions
 */

#ifdef __cplusplus
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

/** 
 * Include the prototypes
 */
#include "protos.h"
