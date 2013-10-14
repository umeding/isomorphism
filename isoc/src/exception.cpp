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
 * throwError -- returns to a save place for program resumtion
 *
 * This function works with the printf formatting rules to create
 * messages that are passed back from the system.
 *
 * @param imEnv is the environment
 * @param format is the format spec we use the message
 */
void throwError(IMEnv * imEnv, const char *format, ...) {
    static char buffer[BUFSIZ];
    char *buf = buffer;
    va_list argp;

    va_start(argp, format);

    char *p,
            *bp,
            *ta;

    bp = buf;
    p = (char *) format;
    while (*p) {
        if (*p != '%') {
            *bp++ = *p++;
        } else {
            char spec[BUFSIZ];
            char *ps,
                    *beginspec;

            beginspec = spec;

            beginspec = ps = spec;

            *ps++ = *p++;
            while (1)
                if ((isdigit(*p) || (*p == '-') || (*p == '.')
                        || (*p == 'l')))
                    *ps++ = *p++;
                else
                    break;

            *ps++ = *p;
            *ps = '\0';

            switch (*p) {
                case 'q':
                    if ((errno > 0) && (errno <= sys_nerr)) {
                        (void) (strcpy(bp, sys_errlist[errno]));
                        bp += strlen(bp);
                    } else {
                        (void) (sprintf(bp, "(unknown errno %d)", errno));
                        bp += strlen(bp);
                    }
                    break;
                case 'd':
                case 'o':
                case 'x':
                case 'u':
                case 'c':
                    (void) sprintf(bp, beginspec, va_arg(argp, int));

                    bp += strlen(bp);
                    break;

                case ' ':
                {
                    int len = atoi(beginspec + 1);

                    bp[len] = '\0';
                    while (len--)
                        bp[len] = ' ';
                    bp += strlen(bp);
                }
                    break;

                case '%':
                    (void) sprintf(bp, "%%");
                    bp += strlen(bp);
                    break;

                case 's':
                    ta = va_arg(argp, char *);

                    (void) sprintf(bp, beginspec, ta ? ta : "(null)");
                    bp += strlen(bp);
                    break;

                case 'e':
                case 'f':
                case 'g':
                    (void) sprintf(bp, beginspec, va_arg(argp, double));

                    bp += strlen(bp);
                    break;

                default:
                    (void) sprintf(bp, beginspec, 0);

                    bp += strlen(bp);
                    break;
            }

            ++p;
        }
    }
    *bp = 0;

    va_end(argp);

    /*
     * Jump to whereever we catch the errors. we are using the
     * ability of longjmp to return a parameter to pass back
     * a message to the catch location.
     */
    throw buffer;
}
