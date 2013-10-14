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
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "compare.h"

/** 
 * Format from a string, containing the printf-like formatting 
 * instuctions.
 * BEWARE - there be dragons here.
 */
#define FORMAT(argp, buf) \
{ \
    char   *p, \
           *bp, \
           *ta; \
\
 \
    bp = buf; \
    p = (char *) format; \
    while (*p) \
    { \
        if (*p != '%') \
        { \
            *bp++ = *p++; \
        } \
        else \
        { \
            char    spec[BUFSIZ]; \
            char   *ps, \
                   *beginspec; \
 \
            beginspec = spec; \
 \
            beginspec = ps = spec; \
 \
            *ps++ = *p++; \
            while (1) \
                if ((isdigit (*p) || (*p == '-') || (*p == '.') \
                     || (*p == 'l'))) \
                    *ps++ = *p++; \
                else \
                    break; \
 \
            *ps++ = *p; \
            *ps = '\0'; \
 \
            switch (*p) \
            { \
            case 'q': \
                if ((errno > 0) && (errno <= sys_nerr)) \
                { \
                    (void) (strcpy (bp, (char*)sys_errlist[errno])); \
                    bp += strlen (bp); \
                } \
                else \
                { \
                    (void) (sprintf (bp, "(unknown errno %d)", errno)); \
                    bp += strlen (bp); \
                } \
                break; \
            case 'd': \
            case 'o': \
            case 'x': \
            case 'u': \
            case 'c': \
                (void) sprintf (bp, beginspec, va_arg (argp, int)); \
 \
                bp += strlen (bp); \
                break; \
 \
            case ' ': \
            { \
                int     len = atoi (beginspec + 1); \
 \
                bp[len] = '\0'; \
                while (len--) \
                    bp[len] = ' '; \
                bp += strlen (bp); \
            } \
                break; \
 \
            case '%': \
                (void) sprintf (bp, "%%"); \
                bp += strlen (bp); \
                break; \
 \
            case 's': \
                ta = va_arg (argp, char *); \
 \
                (void) sprintf (bp, beginspec, ta ? ta : "(null)"); \
                bp += strlen (bp); \
                break; \
 \
            case 'e': \
            case 'f': \
            case 'g': \
                (void) sprintf (bp, beginspec, va_arg (argp, double)); \
 \
                bp += strlen (bp); \
                break; \
 \
            default: \
                (void) sprintf (bp, beginspec, 0); \
 \
                bp += strlen (bp); \
                break; \
            } \
 \
            ++p; \
        } \
    } \
    *bp = 0; \
}

/**
 * fireProgressEvent -- Fires an event so we can monitor the
 * progress of the matching process.
 *
 * This function works with the printf formatting rules to create
 * messages that are passed back from the system.
 *
 * @param imEnv is the environment
 * @param format is the format spec we use the message
 */
void fireProgressEvent(IMEnv * imEnv, const char *format, ...) {
    static char buffer[BUFSIZ];
    char *buf = buffer;
    va_list argp;

    va_start(argp, format);

    FORMAT(argp, buf);

    va_end(argp);

    jni_fireAnEvent(imEnv, "fireProgress", buffer);
}

/**
 * fireWarningEvent -- Fires an event so we can monitor the
 * warnings of the matching process.
 *
 * This function works with the printf formatting rules to create
 * messages that are passed back from the system.
 *
 * @param imEnv is the environment
 * @param format is the format spec we use the message
 */
void
fireWarningEvent(IMEnv * imEnv, const char *format, ...) {
    static char buffer[BUFSIZ];
    char *buf = buffer;
    va_list argp;

    va_start(argp, format);

    FORMAT(argp, buf);

    va_end(argp);

    jni_fireAnEvent(imEnv, "fireWarning", buffer);

}

/**
 * fireProgressEvent -- Fires an event so we can monitor the
 * progress of the matching process.
 *
 * This function works with the printf formatting rules to create
 * messages that are passed back from the system.
 *
 * @param imEnv is the environment
 * @param format is the format spec we use the message
 */
void fireStatusEvent(IMEnv * imEnv, Graph * graph, Queue * queue, int category,
        int vertexType, const char *format, ...) {
    static char buffer[BUFSIZ];
    char *buf = buffer;
    va_list argp;

    va_start(argp, format);

    FORMAT(argp, buf);

    va_end(argp);

    /*
     * Create the JAVA array right here, that way we only have to do it once
     */
    JNIEnv *env = imEnv->jniEnv;
    const char *clazzName = "java/lang/Object";
    jclass clazz = env->FindClass(clazzName);
    jobjectArray vertices = env->NewObjectArray(queue->size, clazz, NULL);
    jobject jname;
    int i = 0;

    for (Vertex * vertex = queue->top; vertex != NULL; vertex = vertex->next) {
        switch (vertex->vertexType) {
            case DEVICE:
                //jname = env->NewStringUTF(imEnv->deviceDefs[vertex->n.vertexDef].name);
                jname = (jobject) vertex->userObject;
                env->SetObjectArrayElement(vertices, i, jname);
                break;
            case NET:
                jname = env->NewStringUTF(vertex->name);
                env->SetObjectArrayElement(vertices, i, jname);
                env->DeleteLocalRef(jname);
                break;
            default:
                throwError(imEnv, "illegal entry in queue");
        }
        i++;
    }

    jni_fireAnEvent(imEnv, "fireStatus", vertexType, category,
            graph->graphNumber - 1, buffer, vertices);
}
