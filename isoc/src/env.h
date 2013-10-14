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
typedef struct GraphEnv {
    /*
     * The graph
     */
    Graph *graph;

    /*
     * Scratch pad
     */
    Device *devices;
    Net *nets;
    int pIndex;

    /*
     */
    int netIndex;

    Net *firstNet;
    Net *lastNet;
    Device *firstDevice;
    Device *lastDevice;

    Net **hashTable;
    int hashSize;

} GraphEnv;

typedef struct IMEnv {
    /*
     * Trace the labeling progress
     */
    int trace;

    int ignoreCase;

    /*
     * Only use local matching if less than this number
     * of neighbors 
     */
    int deduceNeighbors;

    int errorCutOff;

    /*
     * Number of times to try and clear the suspects 
     */
    int suspectCutOff;

    /*
     * boolean, deduce matches where possible 
     */
    int findMatch;

    /*
     * be verbose when printing information 
     */
    int verbose;
    /*
     * Only print net connections if net is small 
     */
    int netPrintLimit;

    /*
     * Flag whether to optimize by looking at 
     * neighbors only 
     */
    int noOpt;

    /*
     * Number of passes with no work before stopping 
     */
    int noProgressCutOff;

    /*
     * Print nets with zero connections 
     */
    int printZeroNets;

    /*
     * Use suffixes to guess at matching names 
     */
    int useSuffix;

    /*
     * run-time stuff
     */

    int runComplete;

    int maxHashSize;
    int hashSize;

    /*
     * vertices matched so far 
     */
    int matchedCount;

    /*
     * Flags whether we have stepped in to disambiguate 
     * * sections 
     */
    int forcedMatch;
    int errors;
    int deducedMatches;
    char translate[256];

    /*
     * Number of different devices 
     */
    int numDeviceDefs;
    DeviceDefinition *deviceDefs;

    /*
     * Number of user defined devices 
     */
    int numUserDefs;

    /*
     * The current pass number 
     */
    int pass;

    /*
     * The current pass type: NET/DEVICE 
     */
    int passType;

    /*
     * pointer to comparison function 
     */
    int (*streq) (IMEnv *, char *, char *);

    GraphEnv *graphEnv;
    GraphEnv __graphEnv[2];

    /*
     * Tie us back to the Java environment
     */
    JNIEnv *jniEnv;
    jobject jobj;

} IMEnv;
