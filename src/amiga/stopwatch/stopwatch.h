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

#include <proto/exec.h>
#include <devices/timer.h>

struct StopWatch
{
	struct timeval  StartTime;      /* start of current measurement */
	struct timeval  StopTime;       /* end of current measurement */
	struct timeval  TotalTime;      /* sum of all measurements */
	struct TimeBase    *TimerBase;  /* Device Base */
	struct IORequest timereq;       /* IORequest */
	unsigned int OpenDevRetVal;     /* Flag is OpenDevice was successful (0=success) */
	unsigned int running;           /* Flag if th stopwatch is running */
};

struct StopWatch *AllocStopWatch(void);
void FreeStopWatch(struct StopWatch *sw);
void StartStopWatch(struct StopWatch *sw);
void StopStopWatch(struct StopWatch *sw);
void ResetStopWatch(struct StopWatch *sw);
unsigned long StopWatchGetMilliSeconds(struct StopWatch *sw);
unsigned long long StopWatchGetMicroSeconds(struct StopWatch *sw);
unsigned long StopWatchGetTotalMilliSeconds(struct StopWatch *sw);
unsigned long long StopWatchGetTotalMicroSeconds(struct StopWatch *sw);

