/***************************************************************************
 *   Copyright (C) 2018 by Alexander Fritsch                               *
 *   email: selco@t-online.de                                              *
***************************************************************************/
/*
 * copmile with
/home/osboxes/opt/m68k-amigaos_26Nov18/bin/m68k-amigaos-gcc stopwatch_test.c stopwatch.c -O0 -Wall -noixemul -o test && sync
*/

#include "stopwatch.h"
#include <stdio.h>
#include <unistd.h>


struct StopWatch *sw1=NULL;
struct StopWatch *sw2=NULL;

void CleanUp(void)
{
	if(sw2)
	{
		FreeStopWatch(sw2);
		sw2=NULL;
	}
	if(sw1)
	{
		FreeStopWatch(sw1);
		sw1=NULL;
	}
}

int main(void)
{
	sw1=AllocStopWatch();
	if(NULL==sw1)
	{
		printf("InitStopWatch 1 failed\n");
		CleanUp();
		return 1;
	}

	sw2=AllocStopWatch();
	if(NULL==sw2)
	{
		printf("InitStopWatch 2 failed\n");
		CleanUp();
		return 2;
	}


	printf("Waiting 1 second for StopWatch 1...\n");
	StartStopWatch(sw1);
	usleep(1000000);
	StopStopWatch(sw1);
	printf("StopWatch 1: %ldms elapsed\n",StopWatchGetMilliSeconds(sw1));
	printf("StopWatch 1: %lldus elapsed\n",StopWatchGetMicroSeconds(sw1));

	printf("Waiting 2 more seconds for StopWatch 1...\n");
	StartStopWatch(sw1);
	usleep(2000000);
	StopStopWatch(sw1);
	printf("StopWatch 1: %ldms elapsed\n",StopWatchGetMilliSeconds(sw1));
	printf("StopWatch 1: %lldus elapsed\n",StopWatchGetMicroSeconds(sw1));

	printf("%ldms in total elapsed\n",StopWatchGetTotalMilliSeconds(sw1));

	CleanUp();
}
