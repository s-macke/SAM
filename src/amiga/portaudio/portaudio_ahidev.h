/***************************************************************************
 *   Copyright (C) 2017 by Alexander Fritsch                               *
 *   email: selco@t-online.de                                              *
 ***************************************************************************/

/*
 *  * portaudio_ahidev.h
 *
 *  Created on: Nov 28, 2017
 *      Author: developer
 */

#ifndef SRC_AMIGA_PORTAUDIO_PORTAUDIO_AHIDEV_H_
#define SRC_AMIGA_PORTAUDIO_PORTAUDIO_AHIDEV_H_



#include "../portaudio18.h"

/* prototypes to be used in general portaudio.cpp only */

PaError Pa_StartStream_ahidev( PortAudioStream *stream );
PaError Pa_AbortStream_ahidev( PortAudioStream *stream );
PaError Pa_StreamActive_ahidev( PortAudioStream *stream );
PaError Pa_OpenDefaultStream_ahidev( PortAudioStream** stream,
		int numInputChannels,
		int numOutputChannels,
		PaSampleFormat sampleFormat,
		double sampleRate,
		unsigned long framesPerBuffer,
		unsigned long numberOfBuffers,
		PortAudioCallback *callback,
		void *userData );
PaError Pa_CloseStream_ahidev( PortAudioStream *stream );
PaError Pa_Initialize_ahidev( void );
PaError Pa_GetSampleSize_ahidev( PaSampleFormat format );
void Abort_Pa_CloseStream_ahidev (void);

extern unsigned int g_AHI_Unit;


#endif /* SRC_AMIGA_PORTAUDIO_PORTAUDIO_AHIDEV_H_ */
