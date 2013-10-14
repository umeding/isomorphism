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
 * Structure definition and macros for queues of vertices
 *
 * Structure for a queue of vertices.
 */
typedef struct Queue {
    struct Vertex *top, *bottom;
    int size;
} Queue;

/**
 * Define a queue iterator
 */
#define QForEach(element, queue)	\
  for (element = queue.top; element != NULL; element = element->next)

/**
 * Set a Queue to null
 */
#define ClearQueue(queue)			\
  (queue)->top = (queue)->bottom = NULL,	\
  (queue)->size = 0

/**
 * Insert a vertex at the end of a Queue of vertices.
 */
#define InsertQ(newVertex, queue)			\
{ (newVertex)->next = NULL;			\
  (queue)->size++;				\
  if ((queue)->bottom == NULL)			\
      (queue)->bottom = (queue)->top = newVertex;	\
  else 						\
    { (queue)->bottom->next = newVertex;		\
      (queue)->bottom = newVertex;		\
    }						\
}

/**
 * Returns the vertex at the head of the queue.
 * Temporary pointer to allow macro to return value
 */

#define PopQueue(queue)					\
( ((__queueTMP = (queue)->top) == NULL) ? __queueTMP :	\
  (((queue)->size--,					\
    ((queue)->top = (queue)->top->next) == NULL) ? 	\
     ((queue)->bottom = NULL, __queueTMP) : __queueTMP ))

/**
 * Append the second queue onto the end of the first queue.
 */
#define AppendQueue(first, second)		\
{ if ((second)->top != NULL)			\
  { (first)->size += (second)->size;		\
    if ((first)->top == NULL)			\
	 (first)->top = (second)->top;		\
    else (first)->bottom->next = (second)->top;	\
    (first)->bottom = (second)->bottom;		\
  }}
#define INSERT_SORT_SIZE 7
