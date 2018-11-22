/***************************************************************************
 *   Copyright (C) 2017 by Alexander Fritsch                               *
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

/* Amiga ahi.device part */

#include "../portaudio18.h"
#include <stdio.h>
#include <stdlib.h>

/* AF, Gwd, 28.Nov 2017 */

#include <exec/types.h>
#include "subtask_support.h"

#include <proto/dos.h>

#include <proto/exec.h>
#include <graphics/gfxbase.h>

#include <proto/alib.h>  /* for CreatePort */

#include <devices/ahi.h>
#include <proto/ahi.h>

#include <string.h>
#include <unistd.h>    // sleep

#include "portaudio_ahidev.h"

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
#define __FPU__ "mc68881"
#else
#define __FPU__ ""
#endif

/*extern "C"*/ LONG KPrintF(STRPTR format, ...);

extern unsigned int global_bufsize_factor;   // AF Test einstellbare Audio Buffergroesse n mal 512 Bytes
extern unsigned int global_benchmark_flag;   // AF Einschalten der Ausgabe der durchschnittlichen Zeit fuer die portaudio-Callbackfunktion
extern char *global_ProgramName;             // AF argv[0]


FILE *File_ahi=NULL;    // testweise samples in File schreiben

unsigned int g_AHI_Unit=0; // can be changed via command line (speak.cpp)

//UBYTE chans[] ={1,2,4,8};  /* get any of the four channels RKRM Audio-example */
UBYTE chans_ahi[] ={3,5,10,12};  /* get a pair of stereo channels, RKRM Devices */
//UBYTE chans[] ={1,8};   /* test, nur rechts */
//UBYTE chans[] ={2,4};   /* test, nur links */

typedef struct
{
	int StreamActive;
	unsigned int AF_StreamID;   // just a number to see which stream is referred to

	struct SubTask *st;
	struct SignalSemaphore sema;    /* data item protection      */

	int numInputChannels;
	int numOutputChannels;          // 1 for mono, 2 for stereo
	PaSampleFormat sampleFormat;
	ULONG AhiSampleType;
	double sampleRate;
	unsigned long framesPerBuffer;  // a frames is one complete sample from each channel
	unsigned long numberOfBuffers;  // suggestion for (double) bufering
	PortAudioCallback *callback;
	void *userData;
	struct AHIRequest *AHIio;       // audio I/O block
	struct AHIRequest *AHIio2;      // audio I/O block
	struct AHIRequest *AIOptrStartStop;// for CMD_START and CMD_STOP
	struct MsgPort *AHImp;
	struct MsgPort *portStartStop;
	ULONG device;                   // Audio device handle
	BYTE *ChipMemBuf1;              // Amiga-Audiobuffer
	BYTE *ChipMemBuf2;              // Amiga-Audiobuffer
	SHORT *FastMemBuf1;             // Espeak-AudioBuffer used in Callback
	SHORT *FastMemBuf2;             // Espeak-AudioBuffer used in Callback
	unsigned int LeftChannel;       // left Audiochannel we got allocated in OpenDevice() or 0
	unsigned int RightChannel;      // right Audiochannel we got allocated in OpenDevice() or 0


}PortAudioStreamStruct;

//static UWORD Convert16SSamples(SHORT *Source16S, BYTE *Dest8S, ULONG SampleCount);  /* reads 16Bit Signed Samples from Source and writes 8Bit Signed Samples to Dest.  DestLen Samples will be read */

STATIC_FUNC ULONG getAHISampleType(PaSampleFormat format );


// Loeschen!
struct MsgPort    *AHImp     = NULL;

extern struct Device* TimerBase;           // nur zum Benchmarking
static struct IORequest timereq;    // nur zum Benchmarking

VOID /*__asm __saveds*/ EspeakAudioTask_AHI(VOID)
{
	//	KPrintF("%s() called\n",__FUNCTION__);



	struct Task *me = FindTask(NULL);
	struct SubTask *st;
	struct SubTaskMsg *stm;

	//KPrintF("Begin EspeakAudioTask_AHI\n");


	/*
	 ** Wait for our startup message from the SpawnSubTask() function.
	 */

	WaitPort(&((struct Process *)me)->pr_MsgPort);
	stm  = (struct SubTaskMsg *)GetMsg(&((struct Process *)me)->pr_MsgPort);
	st   = (struct SubTask *)stm->stm_Parameter;




	{
		//		struct Data *data = (struct Data *)st->st_Data;
		BOOL running = TRUE;
		BOOL worktodo = FALSE;

		PortAudioStreamStruct *PortAudioStreamData=(PortAudioStreamStruct*)(st->st_Data);
		int Error=paInternalError;

		if(0==OpenDevice((CONST_STRPTR)TIMERNAME, UNIT_MICROHZ, &timereq, 0))   // nur zum test
		{
			TimerBase = timereq.io_Device;	            // nur zum test

			/* Make four Reply-Ports */
			PortAudioStreamData->AHImp=CreatePort(0,0);
			if(PortAudioStreamData->AHImp)
			{

				PortAudioStreamData->portStartStop=CreatePort(0,0);
				if(PortAudioStreamData->portStartStop)
				{
					//KPrintF("if(PortAudioStreamData->portStartStop)\n");

					/* Now Open AHI Device */
					//				KPrintF("Attempting to open Audio.device\n");
					/* set up audio I/O block for channel   */
					/* allocation and Open the audio.device */
					PortAudioStreamData->AHIio->ahir_Std.io_Message.mn_ReplyPort=PortAudioStreamData->AHImp;
					PortAudioStreamData->AHIio->ahir_Std.io_Message.mn_Length=sizeof(struct AHIRequest);
					PortAudioStreamData->AHIio->ahir_Version = 4;


					//					((struct IORequest*)(PortAudioStreamData->AHIio))->io_Message.mn_ReplyPort=PortAudioStreamData->AHImp;
					//			PortAudioStreamData->AHIio=(struct AHIRequest *)CreateIORequest(PortAudioStreamData->AHImp,sizeof(struct AHIRequest));
					//			PortAudioStreamData->AHIio->ahir_Version = 4;

					//KPrintF("Vor Open AHI-Device()\n");
					PortAudioStreamData->device=OpenDevice((CONST_STRPTR)AHINAME,g_AHI_Unit,(struct IORequest*)PortAudioStreamData->AHIio,0L);
					//KPrintF("Nach Open AHI-Device()\n");

					if(PortAudioStreamData->device==0)
					{
						/*
					//	PortAudioStreamData->AHIio->ahir_Std.io_Message.mn_Node.ln_Pri = pri;
					//	PortAudioStreamData->AHIio->ahir_Std.io_Data     = ;
					//	PortAudioStreamData->AHIio->ahir_Std.io_Length   = ;
						PortAudioStreamData->AHIio->ahir_Std.io_Offset   = 0;
						PortAudioStreamData->AHIio->ahir_Frequency       = (ULONG) PortAudioStreamData->sampleRate;
					//	PortAudioStreamData->AHIio->ahir_Type            = TYPE;   // AHIST_M16S
						PortAudioStreamData->AHIio->ahir_Volume          = 0x10000;          // Full volume
						PortAudioStreamData->AHIio->ahir_Position        = 0x8000;           // Centered
					//	PortAudioStreamData->AHIio->ahir_Link            = ;
						 */
						// Make a copy of the request (for double buffer
						*(PortAudioStreamData->AHIio2)=*(PortAudioStreamData->AHIio);

						PortAudioStreamData->AHIio->ahir_Std.io_Command=CMD_WRITE;



						/* Set Up Audio IO Blocks for Sample Playing */



						// Data...

						if ((st->st_Port = CreateMsgPort()))
						{
							/*
							 ** Reply startup message, everything ok.
							 ** Note that if the initialization fails, the code falls
							 ** through and replies the startup message with a stm_Result
							 ** of 0 after a Forbid(). This tells SpawnSubTask() that the
							 ** sub task failed to run.
							 */

							/* variables are declared here because they should be initialized to Zero */
							/* in the for loop they would become initialized at the beginning of every turn - which is wrong */
							ULONG CallbackTime=0;       /* For Callback benchmarking */
							ULONG CallBackCount=0;
							ULONG CallBackMaxTime=0;

							ULONG BufferLength=PortAudioStreamData->framesPerBuffer * Pa_GetSampleSize_ahidev(PortAudioStreamData->sampleFormat);



							//							struct IOAudio *Aptr=NULL;  /* for double buffer switching */
							//							struct MsgPort *port=NULL;  /* for double buffer switching */
							//							struct MsgPort *port_2=NULL; /* for double buffer switching for the second audio channel (we do 2-channel(MONO) playback)*/
							//							BYTE *ChipMemBuf=NULL;      /* Amiga-Audiobuffer for double buffer switching */
							//							SHORT *FastMemBuf=NULL;     /* Espeak-AudioBuffer used in Callback for double buffer switching */

							struct AHIRequest *AHIio =PortAudioStreamData->AHIio;
							struct AHIRequest *AHIio2=PortAudioStreamData->AHIio2;
							struct AHIRequest *tmpReq=NULL;
							struct AHIRequest *link = NULL;
							WORD *tmpBuf=NULL;
							WORD *p1=PortAudioStreamData->FastMemBuf1;
							WORD *p2=PortAudioStreamData->FastMemBuf2;


							stm->stm_Result = TRUE;
							ReplyMsg((struct Message *)stm);
							// Ok, all opened successfully
							//					KPrintF("Opened Audio.device successfully\n");

							/*
							 ** after the sub task is up and running, we go into
							 ** a loop and process the messages from the main task.
							 */
							for (;;)
							{
								while ((stm = (struct SubTaskMsg *)GetMsg(st->st_Port)))
								{
									switch (stm->stm_Command)
									{
									case STC_SHUTDOWN:
										/*
										 ** This is the shutdown message from KillSubTask().
										 */
										//								printf("Task STC_SHUTDOWN\n");
										//KPrintF("STC_SHUTDOWN\n");
										running = FALSE;
										break;

									case STC_START:
										/*
										 ** we received a start message with a fractal description.
										 ** clear the rastport and the line update array and start
										 ** rendering.
										 */
										//								printf("Task STC_START\n");
										//KPrintF("STC_START\n");
										worktodo = TRUE;
										link=NULL;                      // Start with link=NULL
										break;

									case STC_STOP:
										/* this message is not used in this example */
										//								printf("Task STC_STOP\n");
										//KPrintF("STC_STOP\n");
										worktodo = FALSE;
										break;
									}

									/*
									 ** If we received a shutdown message, we do not reply it
									 ** immediately. First, we need to free our resources.
									 */
									if (!running) break;

									ReplyMsg((struct Message *)stm);
								}

								if (!running) break;

								if (worktodo)
								{
									//#ifdef ALEXANDER
									int LastBuf;
									ULONG signals;

									/* if there is work to do,...
									 */
									// Fill buffer
									if( 0!=PortAudioStreamData->callback(NULL,p1,PortAudioStreamData->framesPerBuffer,0,NULL))  /* Last Buffer */
									{
										PortAudioStreamData->StreamActive=0;
										worktodo=FALSE;
										LastBuf=TRUE;
									}
									else
									{
										// more buffers to come
										LastBuf=FALSE;
									}

									// Play buffer
									AHIio->ahir_Std.io_Message.mn_Node.ln_Pri = 75;   /* speech, prio of narrator.device */
									AHIio->ahir_Std.io_Command  = CMD_WRITE;
									AHIio->ahir_Std.io_Data     = p1;
									AHIio->ahir_Std.io_Length   = BufferLength;
									AHIio->ahir_Std.io_Offset   = 0;
									AHIio->ahir_Frequency       = (ULONG)PortAudioStreamData->sampleRate;
									AHIio->ahir_Type            = PortAudioStreamData->AhiSampleType;
									AHIio->ahir_Volume          = 0x10000;          // Full volume
									AHIio->ahir_Position        = 0x8000;           // Centered
									AHIio->ahir_Link            = link;
									SendIO((struct IORequest *) AHIio);

									if(link) {

										// Wait until the last buffer is finished (== the new buffer is started)
										signals=Wait(SIGBREAKF_CTRL_C | (1L << PortAudioStreamData->AHImp->mp_SigBit));
#ifdef ALEXANDER
										// Check for Ctrl-C and abort if pressed
										if(signals & SIGBREAKF_CTRL_C) {
											SetIoErr(ERROR_BREAK);
											break;
										}
#endif
// Remove the reply and abort on error
										if(WaitIO((struct IORequest *) link)) {
											SetIoErr(ERROR_WRITE_PROTECTED);
											//									        break;
											worktodo=FALSE;   // AF

										}
									}

									// Check for end-of-sound, and wait until it is finished before aborting
									if(LastBuf) {
										WaitIO((struct IORequest *) AHIio);
										//									      break;
										worktodo=FALSE;   // AF
									}

									link = AHIio;

									// Swap buffer and request pointers, and restart
									tmpBuf = p1;
									p1     = p2;
									p2     = tmpBuf;

									tmpReq    = AHIio;
									AHIio  = AHIio2;
									AHIio2 = tmpReq;
									//#endif
								}
								else
								{
									/* We have nothing to do, just sit quietly and wait for something to happen */
									//KPrintF("Nothing to do\n");
									WaitPort(st->st_Port);
								}
							}

							//KPrintF("stm=%lx\n",stm);
							Error=paNoError;
							//					return;// Error;

							if(global_benchmark_flag)   /* if Benchmarking was done */
							{
								printf("%s, Average BufferTime %lums, Max Buffertime %lums, (Buffer is %lums), Compiled for " __CPU__ " " __FPU__ "\n",global_ProgramName,CallbackTime/CallBackCount,CallBackMaxTime,(ULONG)(global_bufsize_factor*512*1000/PortAudioStreamData->sampleRate));
							}
							//KPrintF("Calling DeletePort st->st_Port\n");
							DeletePort(st->st_Port);
							st->st_Port=NULL;
						}
						else  // st->st_Port = CreateMsgPort() failed
						{
							printf("    Subtask CreateMsgPort() failed\n");
							Error=paInsufficientMemory;
						}
						//KPrintF("Calling CloseDevice\n");
						CloseDevice((struct IORequest*)PortAudioStreamData->AHIio);
						PortAudioStreamData->device=1;
					}
					else
					{
						printf("    Could not open " AHINAME " version 4\n");
						Error=paDeviceUnavailable;
					}

					//KPrintF("Calling DeletePortStartStop\n");
					DeletePort(PortAudioStreamData->portStartStop);
					PortAudioStreamData->portStartStop=NULL;
				}
				else
				{
					printf("    Could not create portStartStop\n");
					Error=paInsufficientMemory;
				}
				//KPrintF("Calling DeleteAHImp\n");
				DeletePort(PortAudioStreamData->AHImp);
				PortAudioStreamData->AHImp=NULL;

			}
			else
			{
				printf("    Could not create AHImp\n");
				Error=paInsufficientMemory;
			}
			CloseDevice(&timereq);
		}
		else
		{
			printf("    Could not open " TIMERNAME"\n");
		}

		// Error uebergeben?
		//KPrintF("Calling ExitSubtask. stm=%lx\n",(ULONG)stm);
		ExitSubTask(st,stm);
	}
}


/* ##################################################################################################### */

PortAudioStream *GlobalPaStreamPtr_ahi=NULL;  /* used for atexit(). Functions there have void parameter -> global Pointer needed */


/*
	Pa_StartStream() and Pa_StopStream() begin and terminate audio processing.
	Pa_StopStream() waits until all pending audio buffers have been played.
	Pa_AbortStream() stops playing immediately without waiting for pending
	buffers to complete.
 */

PaError Pa_StartStream_ahidev( PortAudioStream *stream )
{
	int Error;
	//	KPrintF("%s() called\n",__FUNCTION__);
	//KPrintF("Zeile %ld\n",__LINE__);
	if(stream)
	{
		//		printf(" %s called for Stream %u\n",__FUNCTION__,((PortAudioStreamStruct*)stream)->AF_StreamID);
		((PortAudioStreamStruct*)stream)->StreamActive=1;
		SendSubTaskMsg(((PortAudioStreamStruct*)stream)->st,STC_START,NULL);
		//KPrintF("Zeile %ld\n",__LINE__);
		Error=paNoError;
	}
	else
	{
		//		printf(" %s called with NULL-Ptr\n",__FUNCTION__);
		Error=paBadStreamPtr;

	}

	return Error;
}

/* ########################################################################## */


//PaError Pa_StopStream( PortAudioStream *stream );

PaError Pa_AbortStream_ahidev( PortAudioStream *stream )
{
	//	KPrintF("%s() called\n",__FUNCTION__);
	//	printf(" %s called for Stream %u\n",__FUNCTION__,((PortAudioStreamStruct*)stream)->AF_StreamID);
	//KPrintF("Zeile %ld\n",__LINE__);
	((PortAudioStreamStruct*)stream)->StreamActive=0;
	return paNoError;
}

/* ########################################################################## */
/*
	Pa_StreamActive() returns one when the stream is playing audio,
	zero when not playing, or a negative error number if the
	stream is invalid.
	The stream is active between calls to Pa_StartStream() and Pa_StopStream(),
	but may also become inactive if the callback returns a non-zero value.
	In the latter case, the stream is considered inactive after the last
	buffer has finished playing.
 */


PaError Pa_StreamActive_ahidev( PortAudioStream *stream )
{
	//	KPrintF("%s() called\n",__FUNCTION__);

	if(stream)
	{
		//		printf(" %s called for Stream %u\n returning %u\n",__FUNCTION__,((PortAudioStreamStruct*)stream)->AF_StreamID,((PortAudioStreamStruct*)stream)->StreamActive);
		//KPrintF("Zeile %ld\n",__LINE__);
		return ((PortAudioStreamStruct*)stream)->StreamActive;   //paNoError;
	}
	else
	{
		//		printf(" %s called with NULL-Ptr\n",__FUNCTION__);
		//		printf("    returning paBadStreamPtr\n");
		//KPrintF("Zeile %ld\n",__LINE__);
		return paBadStreamPtr;   // <0 is error

	}
}

/* ########################################################################## */
/*
	Pa_OpenDefaultStream() is a simplified version of Pa_OpenStream() that
	opens the default input and/or ouput devices. Most parameters have
	identical meaning to their Pa_OpenStream() counterparts, with the following
	exceptions:

	If either numInputChannels or numOutputChannels is 0 the respective device
	is not opened (same as passing paNoDevice in the device arguments to Pa_OpenStream() )

	sampleFormat applies to both the input and output buffers.
 */



PaError Pa_OpenDefaultStream_ahidev( PortAudioStream** stream,
		int numInputChannels,
		int numOutputChannels,
		PaSampleFormat sampleFormat,
		double sampleRate,
		unsigned long framesPerBuffer,
		unsigned long numberOfBuffers,
		PortAudioCallback *callback,
		void *userData )
{
	//KPrintF("Zeile %ld\n",__LINE__);
	//	KPrintF("%s() called\n",__FUNCTION__);
	/*
	File_ahi=fopen("Samples","w");
	if(!File_ahi)
	{
		printf("Kann File nicht oeffnen\n");
		return paInternalError;
	}
	 */

	static unsigned int StreamNr=1;
	PortAudioStreamStruct* StreamStruct;
	int Error=paInternalError;

	//	printf(" %s called\n",__FUNCTION__);
	//	printf("    numInputChannels=%d\n",numInputChannels);
	//	printf("    numOutputChannels=%d\n",numOutputChannels);
	//	printf("    sampleFormat=%lu (%u bytes)\n",sampleFormat,Pa_GetSampleSize(sampleFormat));
	//	printf("    sampleRate=%f\n",sampleRate);
	//	printf("    framesPerBuffer=%lu\n",framesPerBuffer);        // a frame is numOutputChannels * BytesPerSample
	//	printf("    numberOfBuffers=%lu\n",numberOfBuffers);        // this is only a suggestion how many buffers to use, e.g double buffering
	//	printf("    userData=$%p\n",userData);


	StreamStruct=(PortAudioStreamStruct*)malloc(sizeof(PortAudioStreamStruct));
	if(StreamStruct)
	{
		//KPrintF("Zeile %ld\n",__LINE__);
		memset(StreamStruct,0,sizeof(PortAudioStreamStruct));  /* initilize all entries */
		StreamStruct->device=1;                                /* device must be initialized to !=0 (for 0 is success) */

		//			printf("    Setting Stream to %u\n",StreamNr);
		StreamStruct->AF_StreamID=StreamNr;
		StreamNr++;

		StreamStruct->callback=callback;  // store ptr to callback function
		StreamStruct->framesPerBuffer=framesPerBuffer*global_bufsize_factor;   // AF Test mit n mal 512 Byte Puffern
		StreamStruct->numInputChannels=numInputChannels;
		StreamStruct->numOutputChannels=numOutputChannels;
		StreamStruct->sampleFormat=sampleFormat;
		StreamStruct->sampleRate=sampleRate;
		StreamStruct->AhiSampleType=getAHISampleType(sampleFormat);


		// ########################################

		*stream=StreamStruct;

		GlobalPaStreamPtr_ahi=StreamStruct;   /* in case of CTRL-C atexit()-Functions are void so a global pointer is needed */

		/* Allocate two Audio Buffers in ChipMem for Amiga-8Bit-Samples */
		StreamStruct->ChipMemBuf1=(BYTE*)AllocMem(StreamStruct->framesPerBuffer,MEMF_CHIP);
		if(StreamStruct->ChipMemBuf1)
		{
			//KPrintF("Zeile %ld\n",__LINE__);

			StreamStruct->ChipMemBuf2=(BYTE*)AllocMem(StreamStruct->framesPerBuffer,MEMF_CHIP);
			if(StreamStruct->ChipMemBuf2)
			{
				//KPrintF("Zeile %ld\n",__LINE__);

				/* Allocate two Audio Buffers in FastMem for Espeak-16Bit-Samples */
				StreamStruct->FastMemBuf1=(SHORT*)AllocMem(StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat),MEMF_PUBLIC);
				//KPrintF("Allocating %lu Bytes for FastMemBuf1, Ptr=$%08lx\n",(ULONG)StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat),(ULONG)StreamStruct->FastMemBuf1);
				if(StreamStruct->FastMemBuf1)
				{
					//KPrintF("Zeile %ld\n",__LINE__);

					StreamStruct->FastMemBuf2=(SHORT*)AllocMem(StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat),MEMF_PUBLIC);
					if(StreamStruct->FastMemBuf2)
					{
						//KPrintF("Zeile %ld\n",__LINE__);

						/* Allocate two audio IO Blocks */
						StreamStruct->AHIio=(struct AHIRequest*)AllocMem(sizeof(struct AHIRequest),MEMF_PUBLIC|MEMF_CLEAR);
						if(StreamStruct->AHIio)
						{
							//KPrintF("Zeile %ld, AHIio=%08lx\n",__LINE__,StreamStruct->AHIio);

							StreamStruct->AHIio2=(struct AHIRequest*)AllocMem(sizeof(struct AHIRequest),MEMF_PUBLIC|MEMF_CLEAR);
							if(StreamStruct->AHIio2)
							{
								//KPrintF("Zeile %ld\n",__LINE__);

								StreamStruct->AIOptrStartStop=(struct AHIRequest*)AllocMem(sizeof(struct AHIRequest),MEMF_PUBLIC|MEMF_CLEAR);
								if(StreamStruct->AIOptrStartStop)
								{
									//KPrintF("Zeile %ld\n",__LINE__);

									StreamStruct->st = SpawnSubTask("EspeakAudioTask_AHI",EspeakAudioTask_AHI,StreamStruct);
									if (StreamStruct->st)
									{
										//KPrintF("Zeile %ld\n",__LINE__);

										Error=paNoError;
										return Error;
									}
									Error=paInternalError;
								}
								else
								{
									printf("    not enough memory for allocating  AIOptrStartStop\n");
									Error=paInsufficientMemory;
								}
								FreeMem(StreamStruct->AHIio2,sizeof(struct AHIRequest));
								StreamStruct->AHIio2=NULL;
							}
							else
							{
								printf("    not enough memory for allocating  AHIio2\n");
								Error=paInsufficientMemory;
							}
							FreeMem(StreamStruct->AHIio,sizeof(struct AHIRequest));
							StreamStruct->AHIio=NULL;
						}
						else
						{
							printf("    not enough memory for allocating  AHIio\n");
							Error=paInsufficientMemory;
						}
						FreeMem(StreamStruct->FastMemBuf2,StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat));
						StreamStruct->FastMemBuf2=NULL;
					}
					else
					{
						printf("    Could not allocate FastMemBuf2 (%lu Bytes)\n",StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat));
						Error=paInsufficientMemory;
					}
					FreeMem(StreamStruct->FastMemBuf1,StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat));
					StreamStruct->FastMemBuf1=NULL;
				}
				else
				{
					printf("    Could not allocate FastMemBuf1 (%lu Bytes)\n",StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat));
					Error=paInsufficientMemory;
				}
				FreeMem(StreamStruct->ChipMemBuf2,StreamStruct->framesPerBuffer);
				StreamStruct->ChipMemBuf2=NULL;
			}
			else
			{
				printf("    Could not allocate ChipMemBuf2 (%lu Bytes)\n",StreamStruct->framesPerBuffer);
				Error=paInsufficientMemory;
			}
			FreeMem(StreamStruct->ChipMemBuf1,StreamStruct->framesPerBuffer);
			StreamStruct->ChipMemBuf1=NULL;
		}
		else
		{
			printf("    Could not allocate ChipMemBuf1 (%lu Bytes)\n",StreamStruct->framesPerBuffer);
			Error=paInsufficientMemory;
		}

		free(StreamStruct);
		StreamStruct=NULL;
		*stream=NULL;
		GlobalPaStreamPtr_ahi=NULL;   /* in case of CTRL-C atexit()-Functions are void so a global pointer is needed */

	}
	else
	{
		printf("    not enough memory for allocating stream %u\n",StreamNr);
		Error=paInsufficientMemory;
	}

	//	KPrintF("Finished %s\n",__FUNCTION__);
	return Error;
}


/* ########################################################################## */
/*
	Pa_CloseStream() closes an audio stream, flushing any pending buffers.
 */

PaError __attribute__((no_instrument_function)) Pa_CloseStream_ahidev( PortAudioStream *stream )
{
	//KPrintF("%s() called\n",__FUNCTION__);
	//KPrintF("Zeile %ld\n",__LINE__);
	PortAudioStreamStruct *StreamStruct=(PortAudioStreamStruct*)stream;

	if(stream)
	{
		//		printf(" %s called for Stream %u\n",__FUNCTION__,StreamStruct->AF_StreamID);

		if(StreamStruct->st)
		{
			KillSubTask(StreamStruct->st);
			StreamStruct->st=NULL;
		}

		if(StreamStruct->device==0)
		{
			CloseDevice((struct IORequest*)StreamStruct->AHIio);
			StreamStruct->device=1;
		}

		if(StreamStruct->portStartStop)
		{
			DeletePort(StreamStruct->portStartStop);
			StreamStruct->portStartStop=0;
		}

		if(StreamStruct->AHImp)
		{
			DeletePort(StreamStruct->AHImp);
			StreamStruct->AHImp=0;
		}

		if(StreamStruct->AIOptrStartStop)
		{
			FreeMem(StreamStruct->AIOptrStartStop,sizeof(struct AHIRequest));
			StreamStruct->AIOptrStartStop=NULL;
		}

		if(StreamStruct->AHIio2)
		{
			FreeMem(StreamStruct->AHIio2,sizeof(struct AHIRequest));
			StreamStruct->AHIio2=NULL;
		}

		if(StreamStruct->AHIio)
		{
			FreeMem(StreamStruct->AHIio,sizeof(struct AHIRequest));
			StreamStruct->AHIio=NULL;
		}

		if(StreamStruct->FastMemBuf2)
		{
			FreeMem(StreamStruct->FastMemBuf2,StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat));
			StreamStruct->FastMemBuf2=NULL;
		}

		if(StreamStruct->FastMemBuf1)
		{
			FreeMem(StreamStruct->FastMemBuf1,StreamStruct->framesPerBuffer*Pa_GetSampleSize(StreamStruct->sampleFormat));
			StreamStruct->FastMemBuf1=NULL;
		}


		if(StreamStruct->ChipMemBuf2)
		{
			FreeMem(StreamStruct->ChipMemBuf2,StreamStruct->framesPerBuffer);
			StreamStruct->ChipMemBuf2=NULL;
		}

		if(StreamStruct->ChipMemBuf1)
		{
			FreeMem(StreamStruct->ChipMemBuf1,StreamStruct->framesPerBuffer);
			StreamStruct->ChipMemBuf1=NULL;
		}

		free(StreamStruct);
		GlobalPaStreamPtr_ahi=NULL;   /* in case of CTRL-C atexit()-Functions are void so a global pointer is needed */


		//		printf("    returning paNoError\n");
		return paNoError;
	}
	else
	{
		//		printf(" %s called with NULL-Ptr\n",__FUNCTION__);
		//		printf("    returning paBadStreamPtr\n");
		return paBadStreamPtr;   // <0 is error

	}

	if(File_ahi)
	{
		fclose(File_ahi);
		File_ahi=NULL;
	}

}


/* ########################################################################## */
/*
	Pa_Initialize() is the library initialisation function - call this before
	using the library.
 */


PaError Pa_Initialize_ahidev( void )
{
	int Error;
	//KPrintF("Zeile %ld\n",__LINE__);
	//	printf(" %s called\n",__FUNCTION__);
	//	KPrintF("%s() called\n",__FUNCTION__);

	Error=paNoError;

	return Error;
}



// ##############################################################################
// not needed by espeak but used by myself, AF 18.May2017

/*
	Return size in bytes of a single sample in a given PaSampleFormat
	or paSampleFormatNotSupported.
 */
PaError Pa_GetSampleSize_ahidev( PaSampleFormat format )
{
	//	KPrintF("%s() called\n",__FUNCTION__);
	//KPrintF("Zeile %ld\n",__LINE__);
	switch (format)
	{
	case paFloat32     : return 4;	/*always available*/
	case paInt16       : return 2;	/*always available*/
	case paInt32       : return 4;	/*always available*/
	case paInt24       : return 6;
	case paPackedInt24 : return 6;
	case paInt8        : return 1;
	case paUInt8       : return 1;  /* unsigned 8 bit, 128 is "ground" */
	default            : return paSampleFormatNotSupported;
	}
}



STATIC_FUNC ULONG getAHISampleType(PaSampleFormat format )
{
	//KPrintF("Zeile %ld\n",__LINE__);
	//	KPrintF("%s() called\n",__FUNCTION__);
	switch (format)
	{
	//	case paFloat32     : return ;	/*always available*/
	case paInt16       : return AHIST_M16S;	           /* Mono, 16 bit signed (WORD) */          /*always available*/
	case paInt32       : return AHIST_M32S;            /* Mono, 32 bit signed (LONG) */          /*always available*/
	//	case paInt24       : return ;
	//	case paPackedInt24 : return ;
	case paInt8        : return AHIST_M8S;             /* Mono, 8 bit signed (BYTE) */
	case paUInt8       : return AHIST_M8U;             /* OBSOLETE! */  /* unsigned 8 bit, 128 is "ground" */
	default            : return AHIE_BADSAMPLETYPE;    /* Unknown/unsupported sample type */;
	}

}


// ##############################################################################
/* reads 16Bit Signed Samples from Source and writes 8Bit Signed Samples to Dest.  */
/* 16 Bit Samples must be BIG_ENDIAN */
/* SampleCount Samples will be read  */
/* SampleCount must be multiple of 2 */
/* Returns number of copied samples  */
/* AF, 15.76.2017                    */

//static UWORD Convert16SSamples(SHORT *Source16S, BYTE *Dest8S, ULONG SampleCount)
//{
//	UWORD i=0;
//	ULONG *SourcePtr=(ULONG*)Source16S;   /* we read 2 16Bit-Samples at once */
//	USHORT *DestPtr=(USHORT*)Dest8S;      /* we write 2 8Bit-Samples at once */
//	ULONG SourceSamples;
//	USHORT DestSamples;
//	//KPrintF("Zeile %ld\n",__LINE__);
//	if(SampleCount%2)
//	{
//		printf("%s() SampleCount %lu is not even!\n",__FUNCTION__,SampleCount);
		/* we read probably one bad sample but continue anyway */
//	}

//	for(;i<SampleCount/2;i++)          /* two Samples at once */
//	{
//		SourceSamples=*SourcePtr++;
//		DestSamples=((SourceSamples & 0xff000000) >>16) | ((SourceSamples &0xff00) >> 8);  /* extract the two High-Bytes */
//		*DestPtr++=DestSamples;

		/*printf("SourceSamples=0x%0lx, DestSamples=0x%0hx\n",SourceSamples,DestSamples);*/
//	}
//	return i*2;
//}



/* used for atexit(). Therefore void parameter -> global Pointer needed */
void __attribute__((no_instrument_function)) Abort_Pa_CloseStream_ahidev (void)
{
	//KPrintF("Zeile %ld\n",__LINE__);
	if(GlobalPaStreamPtr_ahi)
	{
		//		KPrintF("%s() called with Stream=$%08lx\n",__FUNCTION__,GlobalPaStreamPtr_ahi);
		Pa_CloseStream(GlobalPaStreamPtr_ahi);
		GlobalPaStreamPtr_ahi=NULL;
	}
	else
	{
		//		KPrintF("%s() called but nothing to do\n",__FUNCTION__);
	}
}



