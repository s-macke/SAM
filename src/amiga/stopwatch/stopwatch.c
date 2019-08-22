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

#include <devices/timer.h>
#include <proto/timer.h>
#include "stopwatch.h"
#include <malloc.h>
#include <stdio.h>

struct StopWatch *AllocStopWatch(void)
{
	struct StopWatch *sw;

	sw=malloc(sizeof(struct StopWatch));
	if(sw)
	{
		ResetStopWatch(sw);
		sw->OpenDevRetVal=1;

		sw->OpenDevRetVal=OpenDevice((CONST_STRPTR)TIMERNAME, UNIT_MICROHZ, &sw->timereq, 0);
		if(0==sw->OpenDevRetVal)
		{
			sw->TimerBase = (struct TimeBase*)sw->timereq.io_Device;
			return sw;
		}
		else
		{
			printf("%s(): OpenDevice(%s) failed.\n",__FUNCTION__,TIMERNAME);
		}
		free(sw);
		sw=NULL;
	}
	else
	{
		printf("%s(): malloc failed.\n",__FUNCTION__);
	}
	return sw;
}

void FreeStopWatch(struct StopWatch *sw)
{
	if(!sw)
	{
		printf("%s(): NULL-Pointer",__FUNCTION__);
		return;
	}
	if(sw->OpenDevRetVal==0)
	{
		CloseDevice(&sw->timereq);
	}
	free(sw);
}


void StartStopWatch(struct StopWatch *sw)
{
	if(0==sw->running)
	{
		struct TimeBase    *TimerBase=sw->TimerBase;   /* GetSysTime and SubTime need a valid variable TimerBase */
		GetSysTime(&sw->StartTime);
		sw->running=1;
	}
	else
	{
		printf("%s(): already running\n");
	}
}

void StopStopWatch(struct StopWatch *sw)
{
	if(1==sw->running)
	{
		struct TimeBase    *TimerBase=sw->TimerBase;   /* GetSysTime and SubTime need a valid variable TimerBase */
		GetSysTime(&sw->StopTime);
		SubTime(&sw->StopTime, &sw->StartTime);

		AddTime(&sw->TotalTime,&sw->StopTime);
		sw->running=0;
	}
	else
	{
		printf("%S(): already stopped\n",__FUNCTION__);
	}
}

unsigned long StopWatchGetMilliSeconds(struct StopWatch *sw)
{
	if(0==sw->running)
	{
		return (sw->StopTime.tv_secs * 1000 + sw->StopTime.tv_micro / 1000);
	}
	else
	{
		printf("%s(): currently running\n");
		return 0;
	}
}

unsigned long StopWatchGetTotalMilliSeconds(struct StopWatch *sw)
{
	if(0==sw->running)
	{
		return (sw->TotalTime.tv_secs * 1000 + sw->TotalTime.tv_micro / 1000);
	}
	else
	{
		printf("%s(): currently running\n");
		return 0;
	}
}

unsigned long long StopWatchGetMicroSeconds(struct StopWatch *sw)
{
	if(0==sw->running)
	{
		return (sw->StopTime.tv_secs * 1000000 + sw->StopTime.tv_micro);
	}
	else
	{
		printf("%s(): currently running\n");
		return 0;
	}
}

unsigned long long StopWatchGetTotalMicroSeconds(struct StopWatch *sw)
{
	if(0==sw->running)
	{
		return (sw->TotalTime.tv_secs * 1000000 + sw->TotalTime.tv_micro);
	}
	else
	{
		printf("%s(): currently running\n");
		return 0;
	}
}


void ResetStopWatch(struct StopWatch *sw)
{
	memset(&sw->StartTime,0,sizeof(sw->StartTime));
	memset(&sw->StopTime,0,sizeof(sw->StopTime));
	memset(&sw->TotalTime,0,sizeof(sw->TotalTime));
	sw->running=0;
}
