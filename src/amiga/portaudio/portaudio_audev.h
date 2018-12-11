/***************************************************************************
 *   Copyright (C) 2017 by Alexander Fritsch                               *
 *   email: selco@t-online.de                                              *
 ***************************************************************************/
/*
 * portaudio_audev.h
 *
 *  Created on: Nov 28, 2017
 *      Author: Alexander Fritsch
 */

#ifndef SRC_AMIGA_PORTAUDIO_PORTAUDIO_AUDEV_H_
#define SRC_AMIGA_PORTAUDIO_PORTAUDIO_AUDEV_H_

#include "../portaudio18.h"

/* prototypes to be used in general portaudio.cpp only */

PaError Pa_StartStream_audev( PortAudioStream *stream );
PaError Pa_AbortStream_audev( PortAudioStream *stream );
PaError Pa_StreamActive_audev( PortAudioStream *stream );
PaError Pa_OpenDefaultStream_audev( PortAudioStream** stream,
		int numInputChannels,
		int numOutputChannels,
		PaSampleFormat sampleFormat,
		double sampleRate,
		unsigned long framesPerBuffer,
		unsigned long numberOfBuffers,
		PortAudioCallback *callback,
		void *userData );
PaError Pa_CloseStream_audev( PortAudioStream *stream );
PaError Pa_Initialize_audev( void );
PaError Pa_GetSampleSize_audev( PaSampleFormat format );
void Abort_Pa_CloseStream_audev (void);





#endif /* SRC_AMIGA_PORTAUDIO_PORTAUDIO_AUDEV_H_ */
