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
#include "compare.h"
#include "signal.h"

extern IMEnv *g_imEnv;

/** 
 * Throw an exception into the Java environment.
 * @param env is the Java environment
 * @param msg is the message we want to pass along
 */
static void
throwException(JNIEnv * env, char *msg) {
    jclass clazz = env->FindClass("com/uwemeding/connectivity/CompareException");

    env->ThrowNew(clazz, msg);
}

/**
 * Fire a progress event.
 * @param imEnv is the compare environment
 * @param eventName is the event we want to fire;
 * @param message is the message we want to send
 */
void
jni_fireAnEvent(IMEnv * imEnv, const char *eventName, char *message) {
    JNIEnv *env = imEnv->jniEnv;
    jclass clazz = env->FindClass("com/uwemeding/connectivity/CompareAPI");
    jmethodID mid = env->GetMethodID(clazz, "createEvent",
            "(Ljava/lang/String;)Lcom/uwemeding/connectivity/CompareEvent;");
    jobject jevent;

    if (mid) {
        jstring jmessage = env->NewStringUTF(message);

        jevent = env->CallObjectMethod(imEnv->jobj, mid, jmessage, NULL);
        jmethodID mid = env->GetMethodID(clazz, eventName,
                "(Lcom/uwemeding/connectivity/CompareEvent;)V");

        if (mid)
            env->CallVoidMethod(imEnv->jobj, mid, jevent, NULL);
        else
            throwError(imEnv, "Unable to fire event - cannot find '%s'.",
                eventName);
    } else
        throwException(imEnv->jniEnv,
            (char*) "Unable to fire event - cannot find 'createEvent'.");

}

/**
 * Fire a progress event
 * @param imEnv is the compare environment
 * @param eventName is the event we want to fire
 * @param type is the node type the event is referring to
 * @param reportType is the report classification for this event
 * @param graphNumber is the graph to which this event pertains
 * @param message is the message we want to pass along
 * @param vertices are the vertices involved
 */
void
jni_fireAnEvent(IMEnv * imEnv, const char *eventName, int type, int reportType,
        int graphNumber, char *message, jobjectArray vertices) {
    JNIEnv *env = imEnv->jniEnv;
    jclass clazz = env->FindClass("com/uwemeding/connectivity/CompareAPI");
    jmethodID mid = env->GetMethodID(clazz, "createEvent",
            "(IIILjava/lang/String;[Ljava/lang/Object;)Lcom/uwemeding/connectivity/CompareEvent;");
    jobject jevent;

    if (mid) {
        jstring jmessage = env->NewStringUTF(message);

        jevent =
                env->CallObjectMethod(imEnv->jobj, mid, (jint) type,
                (jint) reportType, (jint) graphNumber,
                jmessage, vertices, NULL);
        jmethodID mid = env->GetMethodID(clazz, eventName,
                "(Lcom/uwemeding/connectivity/CompareEvent;)V");

        if (mid)
            env->CallVoidMethod(imEnv->jobj, mid, jevent, NULL);
        else
            throwError(imEnv, "Unable to fire event - cannot find '%s'.",
                eventName);
    } else
        throwException(imEnv->jniEnv,
            (char*) "Unable to fire event - cannot find 'createEvent'.");

}

/**
 * Get the compare environment.
 * @param env is the Java environment
 * @param obj is the Java instance of the CompareAPI class
 * @param is the mode we are expecting the environment to be
 * @throws error is the mode is not what we are expecting
 */
static IMEnv *
getIMEnv(JNIEnv * env, jobject obj, int mode) {
    jclass clazz = env->FindClass("com/uwemeding/connectivity/CompareAPI");

    if (clazz) {
        jfieldID fid = env->GetFieldID(clazz, "env", "J");
        jlong j_imEnv = env->GetLongField(obj, fid);

        g_imEnv = (IMEnv *) j_imEnv;
        /*
         * Make sure we preserve the current Java environment 
         */
        g_imEnv->jniEnv = env;
        g_imEnv->jobj = obj;
        if (g_imEnv->runComplete != mode)
            throwError(g_imEnv,
                "Need to re-initialize before running again.\n");
        return (g_imEnv);
    }
    return (NULL);
}

/**
 * Set the compare environment.
 * @param env is the Java environment
 * @param obj is the Java instance of the CompareAPI class
 * @param imEnv is the native compare environment
 */
static void
setIMEnv(JNIEnv * env, jobject obj, IMEnv * imEnv) {
    jclass clazz = env->FindClass("com/uwemeding/connectivity/CompareAPI");

    if (clazz) {
        jfieldID fid = env->GetFieldID(clazz, "env", "J");

        env->SetLongField(obj, fid, (jlong) imEnv);
    }
}


/*
 * Avoid name mangling in C++, we need the names as they are for
 * the Java integration to work.
 */
#ifdef __cplusplus
#define EXTERN_C_START extern "C" {
#else
#define EXTERN_C_START
#endif

#ifdef __cplusplus
#define EXTERN_C_END } /* extern "C" */
#else
#define EXTERN_C_END
#endif

EXTERN_C_START

/**
 * Finalize the native environment
 */
JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_finalizeEnv(
        JNIEnv * env,
        jobject obj) {
    freeAll();
}

/*
 * Option: deduce neighbors
 */
JNIEXPORT jint JNICALL
Java_com_uwemeding_connectivity_CompareAPI_getDeduceNeighbors(
        JNIEnv * env,
        jobject obj) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    return ((jint) imEnv->deduceNeighbors);
}

JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_setDeduceNeighbors(
        JNIEnv * env,
        jobject obj,
        jint
        j_number) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);
    int number = (int) j_number;

    if (number > DEDUCEHTSIZE) {
        fireProgressEvent(imEnv,
                "Deduce neighborhood value too large, using default value: %d\n",
                DEDUCEHTSIZE / 10);
        imEnv->deduceNeighbors = DEDUCEHTSIZE / 10;
    } else
        imEnv->deduceNeighbors = number;
}

/*
 * Option: trace the execution
 */
JNIEXPORT jint JNICALL
Java_com_uwemeding_connectivity_CompareAPI_getTrace(
        JNIEnv *env,
        jobject obj) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    return ((jint) imEnv->trace);
}

JNIEXPORT void JNICALL
Java_com_uwemeding_connectivity_CompareAPI_setTrace(
        JNIEnv *env,
        jobject obj,
        jint value) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    imEnv->trace = (int) value;
}

/**
 * Initialize the isomorphism environment
 */
JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_initialize(
        JNIEnv * env,
        jobject obj) {
    freeAll();

    IMEnv *imEnv = initializeIMEnv();

    setIMEnv(env, obj, imEnv);
}

/**
 * execute the graph isomorphism
 */
JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_execute(
        JNIEnv * env,
        jobject obj) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    try {
        isomorphism(imEnv);
    } catch (char *message) {
        throwException(env, message);
    }
}

/** 
 * Define a net name equality between the two graphs
 */
JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_defineEquate(
        JNIEnv * env,
        jobject obj,
        jstring jname1,
        jstring jname2) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    jboolean isCopy1,
            isCopy2;
    const char *name1 = env->GetStringUTFChars(jname1, &isCopy1);
    const char *name2 = env->GetStringUTFChars(jname2, &isCopy2);

    defineEquate(imEnv, (char *) name1, (char *) name2);

    if (isCopy1 == JNI_TRUE)
        env->ReleaseStringUTFChars(jname1, name1);
    if (isCopy2 == JNI_TRUE)
        env->ReleaseStringUTFChars(jname2, name2);
}

/** 
 * Define net name aliases
 */
JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_defineNetAlias(
        JNIEnv * env,
        jobject obj,
        jint jgraphNumber,
        jstring jname,
        jobjectArray jaliases) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    // what graph are we working on
    int graphNumber = (int) jgraphNumber;

    if (graphNumber < 0 && graphNumber > 1)
        throwError(imEnv, "Graph number can only be 0 or 1.\n");
    // assign the current graph
    imEnv->graphEnv = &imEnv->__graphEnv[graphNumber];

    // Get the net we are aliasing
    jboolean isCopy;
    const char *c_str = env->GetStringUTFChars(jname, &isCopy);

    if (isCopy == JNI_TRUE)
        env->ReleaseStringUTFChars(jname, c_str);

    // loop thru the array and get all the net aliases
    unsigned int size = (unsigned int) env->GetArrayLength(jaliases);
    char **args = (char **) fastAlloc(size * sizeof (char *));

    for (unsigned int i = 0; i < size; i++) {
        jstring j_str =
                (jstring) env->GetObjectArrayElement(jaliases, i);

        c_str = env->GetStringUTFChars(j_str, &isCopy);
        args[i] = copyString((char *) c_str);
        if (isCopy == JNI_TRUE)
            env->ReleaseStringUTFChars(j_str, c_str);
        env->DeleteLocalRef(j_str);
    }

    defineNetAlias(imEnv, (char *) c_str, (int) size, args);
}

/** 
 * Define a device master
 */
JNIEXPORT void JNICALL Java_com_uwemeding_connectivity_CompareAPI_defineDeviceMaster(
        JNIEnv * env,
        jobject obj,
        jstring
        jname,
        jobjectArray
        jconnections) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    jboolean isCopy;
    const char *c_str = env->GetStringUTFChars(jname, &isCopy);
    char *name = copyString((char *) c_str);

    if (isCopy == JNI_TRUE)
        env->ReleaseStringUTFChars(jname, c_str);

    // loop thru the array ant get all the connections
    unsigned int size = (unsigned int) env->GetArrayLength(jconnections);
    char **args = (char **) fastAlloc(size * sizeof (char *));

    for (unsigned int i = 0; i < size; i++) {
        jstring j_str =
                (jstring) env->GetObjectArrayElement(jconnections, i);
        c_str = env->GetStringUTFChars(j_str, &isCopy);
        args[i] = copyString((char *) c_str);
        if (isCopy == JNI_TRUE)
            env->ReleaseStringUTFChars(j_str, c_str);
        env->DeleteLocalRef(j_str);
    }

    createDefineDeviceMaster(imEnv, name, size, args);
}

/** 
 * Define a device master
 */
JNIEXPORT void JNICALL
Java_com_uwemeding_connectivity_CompareAPI_defineDeviceVertex(
        JNIEnv * env,
        jobject obj,
        jint
        jgraphNumber,
        jstring
        jname,
        jobject
        jinstObject,
        jobjectArray
        jconnections) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    /*
     * what graph are we working on 
     */
    int graphNumber = (int) jgraphNumber;

    if (graphNumber < 0 && graphNumber > 1)
        throwError(imEnv, "Graph number can only be 0 or 1.\n");
    /*
     * assign the current graph 
     */
    imEnv->graphEnv = &imEnv->__graphEnv[graphNumber];

    jboolean isCopy;
    const char *c_str = env->GetStringUTFChars(jname, &isCopy);
    char *name = copyString((char *) c_str);

    if (isCopy == JNI_TRUE)
        env->ReleaseStringUTFChars(jname, c_str);

    uint64_t instObject = (uint64_t) (env->NewGlobalRef(jinstObject));

    /*
     * loop thru the array ant get all the connections 
     */
    unsigned int size = (unsigned int) env->GetArrayLength(jconnections);
    char **args = (char **) fastAlloc(size * sizeof (char *));

    for (unsigned int i = 0; i < size; i++) {
        jstring j_str =
                (jstring) env->GetObjectArrayElement(jconnections, i);
        c_str = env->GetStringUTFChars(j_str, &isCopy);
        args[i] = copyString((char *) c_str);
        if (isCopy == JNI_TRUE)
            env->ReleaseStringUTFChars(j_str, c_str);
        env->DeleteLocalRef(j_str);
    }

    createDefineDeviceVertex(imEnv, name, instObject, size, args);
}

JNIEXPORT void JNICALL
Java_com_uwemeding_connectivity_CompareAPI_setGraphName(
        JNIEnv * env,
        jobject obj,
        jint jgraphNumber,
        jstring jname) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    /*
     * what graph are we working on 
     */
    int graphNumber = (int) jgraphNumber;

    if (graphNumber < 0 && graphNumber > 1)
        throwError(imEnv, "Graph number can only be 0 or 1.\n");
    /*
     * assign the current graph 
     */
    imEnv->graphEnv = &imEnv->__graphEnv[graphNumber];

    jboolean isCopy;
    const char *c_str = env->GetStringUTFChars(jname, &isCopy);

    imEnv->graphEnv->graph->graphName = copyString((char *) c_str);

    if (isCopy == JNI_TRUE)
        env->ReleaseStringUTFChars(jname, c_str);
}

JNIEXPORT jstring JNICALL
Java_com_uwemeding_connectivity_CompareAPI_getGraphName(
        JNIEnv * env,
        jobject obj,
        jint jgraphNumber) {
    IMEnv *imEnv = getIMEnv(env, obj, FALSE);

    /*
     * what graph are we working on 
     */
    int graphNumber = (int) jgraphNumber;

    if (graphNumber < 0 && graphNumber > 1)
        throwError(imEnv, "Graph number can only be 0 or 1.\n");
    /*
     * assign the current graph 
     */
    imEnv->graphEnv = &imEnv->__graphEnv[graphNumber];

    jstring jname = env->NewStringUTF(imEnv->graphEnv->graph->graphName);

    return (jname);
}

EXTERN_C_END