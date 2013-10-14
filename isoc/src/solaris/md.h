
/* 
 * Sun Solaris specific stuff
 */

#define STRNCASECMP(a,b,n) strncasecmp(a,b,n)
#define STRCASECMP(a,b)    strcasecmp(a,b)

/*
 * The random number generator to use
 */
#define Random             random
#define sRandom            srandom
