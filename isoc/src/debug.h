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

/**
 * The global variable DebugLevel defines which debugging information is
 * printed. 
 */
extern unsigned int DebugLevel;

#ifdef DEBUG
#define debug(level, remaining)	if(DebugLevel & level){remaining}
#else
#define debug(level,remaining)
#endif

#define DBG_ALWAYS	    0xffffffff
#define DBG_FORCE		0x00000001
#define DBG_PROGRESS	0x00000002
#define DBG_CLEAN		0x00000004
#define DBG_TRACE		0x00000008
#define DBG_INITVALUE	0x00000010
#define DBG_TRACEALL	0x00000020
#define DBG_PRINTBAD	0x00000040
#define DBG_UNIQUES		0x00000080
#define DBG_SUSPECTS	0x00000100
#define DBG_CHECKSUM	0x00000200
#define DBG_ENTERHASH	0x00000400
#define DBG_COMPUTEVALUE 0x0000800
#define DBG_HASH		0x00001000
#define DBG_HASHTABLE	0x00002000
#define DBG_MATCH		0x00004000
#define DBG_SORT		0x00008000
#define DBG_INPUT		0x00010000
#define DBG_EQUATE		0x00020000
#define DBG_DEDUCE		0x00040000
#define DBG_DOUBLECHECK	0x00080000
