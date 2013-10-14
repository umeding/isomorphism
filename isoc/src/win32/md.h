/* 
 * WIN32 specific stuff here
 */

#define STRNCASECMP(a,b,n) strnicmp(a,b,n)
#define STRCASECMP(a,b)    stricmp(a,b)

/*
 * The random number generator functions to use
 */
#define Random             rand
#define sRandom            srand

