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
static struct Vertex *__queueTMP;


/**
 * QueueOK -- Check the consistency of a queue.
 *
 * @param queue is the queue
 * @return TRUE if the queue is ok, FALSE otherwise
 */
int queueOK(Queue * queue) {
    register Vertex *vertex;
    register Vertex *bottomVertex = NULL;
    register int count;

    count = 0;
    for (vertex = queue->top; vertex != NULL; vertex = vertex->next) {
        bottomVertex = vertex;
        count++;
        if (count > queue->size)
            break;
    }

    if (count != queue->size)
        return (FALSE);
    if (bottomVertex != queue->bottom)
        return (FALSE);
    return (TRUE);
}

#define SUFFIXLENGTH 15

/**
 * cmpSuffix -- Compare the suffixes of two strings, ignoring special chars 
 * and case. This is used to divine whether two names from different circuits
 * might be the same.  (Why suffix?  Well it's easier than trying to compute 
 * the distance between two strings and suffixes tend to be same.).
 * 
 * @param str1 is the 1st string to compare
 * @param len1 is the length of that string
 * @param str2 is the 2nd string to compare
 * @param len2 is the length of that string
 * @return 0 for a match, 1 if str1>str2, and -1 if str1<str2
 */
static short cmpSuffix(char *str1, int len1, char *str2, int len2) {
    register char *s1,
            *s2;
    char c1,
            c2;

    s1 = str1 + len1;
    s2 = str2 + len2;
    while (TRUE) {
        c1 = 0;
        while (len1--) { /* Get next rightmost significant char */
            if (isalnum(*(--s1))) {
                c1 = isupper(*s1) ? tolower(*s1) : *s1;
                break;
            }
        }
        c2 = 0;
        while (len2--) { /* Get next rightmost significant char */
            if (isalnum(*(--s2))) {
                c2 = isupper(*s2) ? tolower(*s2) : *s2;
                break;
            }
        }
        if ((c1 == 0) || (c2 == 0) || (c1 == c2))
            return 0; /* Matched to end of one string */
        if (c1 < c2)
            return -1;
        if (c1 > c2)
            return 1;
    }
}

/**
 * MatchBySuffix -- Look through the two queues for two 
 * suffixes that match.
 * This is quadratic in the length of the queues!!
 *
 * @param queue1 is the 1st queue
 * @param queue2 is the 2nd queue
 * @return TRUE if a match is found and FALSE otherwise.
 */
int matchBySuffix(Queue * queue1, Queue * queue2) {
    register Vertex *trail1,
            *trail2;
    register Vertex *vertex1,
            *vertex2;
    size_t len1;

    for (trail1 = NULL,
            vertex1 = queue1->top;
            vertex1 != NULL; trail1 = vertex1, vertex1 = vertex1->next) {
        len1 = strlen(vertex1->name); /* Do not recompute */
        for (trail2 = NULL,
                vertex2 = queue2->top;
                vertex2 != NULL; trail2 = vertex2, vertex2 = vertex2->next) {
            if (!cmpSuffix
                    (vertex1->name, len1, vertex2->name, strlen(vertex2->name))) {
                if (trail1 != NULL) {
                    trail1->next = vertex1->next;
                    /*
                     * Insert vertex1 at front of queue 
                     */
                    vertex1->next = queue1->top;
                    queue1->top = vertex1;
                    if (queue1->bottom == vertex1)
                        queue1->bottom = trail1;
                }
                if (trail2 != NULL) {
                    trail2->next = vertex2->next;
                    /*
                     * Insert vertex2 at front of queue 
                     */
                    vertex2->next = queue2->top;
                    queue2->top = vertex2;
                    if (queue2->bottom == vertex2)
                        queue2->bottom = trail2;
                }
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

/**
 * InsertionSortQ -- Sort a Queue using insertion sort.
 *
 * @param imEnv is the compare environment
 * @param queue is the queue
 */
void insertionSortQ(IMEnv * imEnv, Queue * queue) {
    register Vertex *uVertex;
    Queue newQueue;
    register Vertex *vertexPt;
    long attrib1,
            attrib2;
    short doit;

    if (queue->size == 0)
        return;
    newQueue.size = queue->size;
    newQueue.top = newQueue.bottom = PopQueue(queue);

    debug(DBG_ALWAYS,
    if (!(newQueue.top != NULL))
            throwError(imEnv, "InserSorting a NULL Queue!!"););
    newQueue.top->next = NULL;

    for (uVertex = PopQueue(queue); uVertex != NULL;
            uVertex = PopQueue(queue)) {

        /*
         * insert the vertex in the new queue in order 
         */
        attrib1 = 0;
        doit = (uVertex->vertexValue > newQueue.bottom->vertexValue);
        if (!doit && (uVertex->vertexValue == newQueue.bottom->vertexValue)) {
            attrib2 = 0;
            doit = (attrib1 <= attrib2);
        }
        if (doit) {
            newQueue.bottom->next = uVertex;
            newQueue.bottom = uVertex;
            uVertex->next = NULL;
            continue;
        }
        doit = (uVertex->vertexValue < newQueue.top->vertexValue);
        if (!doit && (uVertex->vertexValue == newQueue.top->vertexValue)) {
            attrib2 = 0;
            doit = (attrib1 > attrib2);
        }

        if (doit) {
            uVertex->next = newQueue.top;
            newQueue.top = uVertex;
            continue;
        }
        /*
         * always not NULL 
         */
        vertexPt = newQueue.top;
        while (1) {
            if (vertexPt->next == NULL)
                break;
            doit = (uVertex->vertexValue > vertexPt->next->vertexValue);
            if (!doit
                    && (uVertex->vertexValue == vertexPt->next->vertexValue)) {
                attrib2 = 0;
                doit = (attrib1 < attrib2);
            }
            if (!doit)
                break;
            vertexPt = vertexPt->next;
        }
        uVertex->next = vertexPt->next;
        vertexPt->next = uVertex;
    }
    queue->top = newQueue.top;
    queue->bottom = newQueue.bottom;
    queue->size = newQueue.size;

}

/**
 * SortQueue -- Sort a Queue using quick sort
 *
 * @param imEnv is the compare environment
 * @param queue is the queue
 */
void sortQueue(IMEnv * imEnv, Queue * queue) {

    int sorted1,
            sorted2;
    unsigned int lastValue1,
            lastValue2;
    register unsigned int sectionValue;
    Queue LessQ,
            MoreQ;
    register Vertex *aVertex;

    if (queue->size <= 1) {
        return;
    } else if (queue->size <= INSERT_SORT_SIZE) {
        insertionSortQ(imEnv, queue);
        return;
    } else {
        ClearQueue(&LessQ);
        ClearQueue(&MoreQ);

        /*
         * Sectioning is done by taking the average of the first and 
         * last values in the list, instead of simply the first value.
         * This should take care of an almost sorted list which blows 
         * quicksort away. Also check whether the split queues are 
         * already sorted.
         */
        sectionValue =
                queue->top->vertexValue / 2 + queue->bottom->vertexValue / 2;
        sorted1 = sorted2 = TRUE;
        lastValue1 = lastValue2 = 0;

        /*
         * If the values are the same, make sure that one of the queues is not empty 
         */
        if (queue->top->vertexValue == queue->bottom->vertexValue) {
            aVertex = PopQueue(queue);
            sectionValue = lastValue2 = aVertex->vertexValue;
            /*
             * Make sure MoreQ is not Empty 
             */
            insertQueue(aVertex, &MoreQ);
        }
        while ((aVertex = PopQueue(queue)) != NULL) {
            if (aVertex->vertexValue <= sectionValue) {
                if (aVertex->vertexValue < lastValue1)
                    sorted1 = FALSE;
                lastValue1 = aVertex->vertexValue;
                insertQueue(aVertex, &LessQ);
            } else {
                /*
                 * Insert into second queue 
                 */
                if (aVertex->vertexValue < lastValue2)
                    sorted2 = FALSE;
                lastValue2 = aVertex->vertexValue;
                insertQueue(aVertex, &MoreQ);
            }
        }
        if (!sorted1) {
            debug(DBG_ALWAYS,
            if (!(MoreQ.size > 0))
                    throwError(imEnv, "SortQueue does not converge"););

            if (LessQ.size > 1)
                sortQueue(imEnv, &LessQ);
        }
        if (!sorted2) {
            debug(DBG_ALWAYS,
            if (!(MoreQ.size > 0))
                    throwError(imEnv, "SortQueue does not converge"););

            if (MoreQ.size > 1)
                sortQueue(imEnv, &MoreQ);
        }
        AppendQueue(&LessQ, &MoreQ);
        queue->top = LessQ.top;
        queue->bottom = LessQ.bottom;
        queue->size = LessQ.size;

        /*
         * assert(QueueOK(queue), "SortQueue: Bad result Queue");
         */
        debug(DBG_SORT,
                printf("SortQueue: The sorted queue\n");
                PrintQueue(imEnv, queue););
    }
}

/**
 * InsertQueue -- Insert a vertex at the end of a Queue of vertices.
 *
 * @param newVertex is the vertex we are inserting
 */
void insertQueue(Vertex * newVertex, Queue * queue) {
    InsertQ(newVertex, queue);
}
