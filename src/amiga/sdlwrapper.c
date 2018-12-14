/* AF, GWD, Nov,21 2018 */
/* wraps certain SDL-Calls to my portaudio implementation */

/* leftovers from portaudio... */
unsigned int global_benchmark_flag=0;
char *global_ProgramName="sam_globaler_name";
unsigned int global_bufsize_factor=1;

/************************************************************** */
#include <unistd.h>
#include <SDL.h>

#include "portaudio18.h"

LONG KPrintF(STRPTR format, ...);
void Abort_Pa_CloseStream_audev (void);

static PortAudioStream *pa_stream=NULL;


/************************************************************** */


void SDL_CloseAudio(void)
{
//	KPrintF("%s()\n",__FUNCTION__);
	Pa_CloseStream(pa_stream);
}
/************************************************************** */

void SDL_Delay(Uint32 ms)
{
//	KPrintF("%s(%ld)\n",__FUNCTION__,ms);
	__chkabort();
	usleep(ms*1000);
}
/************************************************************** */


char *SDL_Error_String="SDL Dummy_Error";

DECLSPEC char * SDL_GetError(void)
{
//	KPrintF("%s()\n",__FUNCTION__);
	return SDL_Error_String;
}
/************************************************************** */

//flags=SDL_INIT_AUDIO The file I/O and threading subsystems are initialized
int SDL_Init(Uint32 flags)
{
//	KPrintF("%s(0x%lx)\n",__FUNCTION__,flags);
	if(flags & SDL_INIT_AUDIO)
	{
		return Pa_Initialize();
	}
	else
	{
		return paNoError;
	}
}
/************************************************************** */

void (SDLCALL *SDL_CallbackFunction)(void *userdata, Uint8 *stream, int len);

int PortAudioCallbackFunction(
    void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    PaTimestamp outTime, void *userData )
{
//KPrintF("*****************%s(%ld)*********************\n",__FUNCTION__,framesPerBuffer);
	SDL_CallbackFunction(userData, outputBuffer,framesPerBuffer);
	return 0;  /* continue until somebody Aborts the stream */
}


int SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained)
{
	PaError Error;
	PaSampleFormat SampleFormat;

//	KPrintF("%s()\n",__FUNCTION__);
//	KPrintF("channels %ld\n",desired->channels);
//	KPrintF("format %ld\n",desired->format);
//	KPrintF("freq %ld\n",desired->freq);
//	KPrintF("Padding %ld\n",desired->padding);
//	KPrintF("Samples %ld\n",desired->samples);
//	KPrintF("Silence %ld\n",desired->silence);
//	KPrintF("Size %ld\n",desired->size);

	if(desired->format!=AUDIO_U8)
	{
		printf("Error: only SDL-Format AUDIO_U8 supported\n");
	    return paSampleFormatNotSupported;     // < 0 means error
	}
	else
	{
		SampleFormat=paUInt8;
	}

	SDL_CallbackFunction=desired->callback;


	Error=Pa_OpenDefaultStream( &pa_stream,
	                            0,  /*int numInputChannels*/
								desired->channels,
								SampleFormat,
								desired->freq,
								desired->samples, /* unsigned long framesPerBuffer*/
	                            2, /*unsigned long numberOfBuffers*/
								PortAudioCallbackFunction, /* PortAudioCallback *callback */
	                            desired->userdata  /*  void *userData*/ );

	if(obtained)
	{
		*obtained=*desired;
	}

	return Error;
}
/************************************************************** */

// 1=audio callback is stopped when this returns.  0=start ausio callback
void SDL_PauseAudio(int pause_on)
{
//	KPrintF("%s(%ld)\n",__FUNCTION__,pause_on);

	if(0==pause_on)
	{
		Pa_StartStream(pa_stream);
	}
	else
	{
		Pa_AbortStream(pa_stream);
	}

}
/************************************************************** */

void SDL_Quit(void)  // useful with atexit()
{
	KPrintF("%s()\n",__FUNCTION__);
	Abort_Pa_CloseStream();
}
