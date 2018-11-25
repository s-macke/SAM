/***************************************************************************
 *   Copyright (C) 2018 by Alexander Fritsch                               *
 *   email: selco@t-online.de                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write see:                           *
 *               <http://www.gnu.org/licenses/>.                           *
 ***************************************************************************/

#ifdef mc68060
#define __CPU__ "mc68060"
#elif defined  mc68040
#define __CPU__ "mc68040"
#elif  defined mc68030
#define __CPU__ "mc68030"
#elif  defined mc68020
#define __CPU__ "mc68020"
#elif  defined mc68000
#define __CPU__ "mc68000"
#else
#define __CPU__ "???????"
#endif

#ifdef __HAVE_68881__
#define __FPU__ "+mc68881"
#else
#define __FPU__ ""
#endif



#define      VERSION       "1"
#define      REVISION      "1"                     /* Revision always starts with 1 ! */
//#define      DATE          "15.07.2017"   /* comes from make-command line as CXXFLAGS+=-DDATE=\\\"$(date +'%d.%m.%Y')\\\" */
#define      PROGNAME      "sam"
#define      COMMENT       "BETA-Version, Alexander Fritsch, selco, based on SAM C-conversion by Sebastian Macke"
//#define      COMMENT                   "Alexander Fritsch, selco, based on SAM C-conversion by Sebastian Macke"

#define      VERS          PROGNAME" " VERSION "." REVISION
#define      VERSTAG       "\0$VER: " PROGNAME " " VERSION "." REVISION " (" DATE ") " COMMENT ", compiled for " __CPU__  __FPU__

char versiontag[] = VERSTAG;

char toolchain_ver[] = "\0$TOOLCHAIN_VER: " TOOLCHAIN_VER;   /* This macro comes from Makefile.amiga and contains git hash */
