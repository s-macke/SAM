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

#include "portaudio_audev.h"
#include "portaudio_ahidev.h"
#include <stdio.h>


/////////////////////////////////////////////////////////////////////////////////////////
// Functions are called via function pointers
// pointers point to audio.device or ahi.device functions

// defaults to amiga audio.device
PaError (*Pa_StartStream_FctPtr)( PortAudioStream *stream ) = Pa_StartStream_audev;
PaError (*Pa_AbortStream_FctPtr)( PortAudioStream *stream ) = Pa_AbortStream_audev;
PaError (*Pa_StreamActive_FctPtr)( PortAudioStream *stream ) =Pa_StreamActive_audev;
PaError (*Pa_OpenDefaultStream_FctPtr)( PortAudioStream** stream,
                                        int numInputChannels,
                                        int numOutputChannels,
                                        PaSampleFormat sampleFormat,
                                        double sampleRate,
                                        unsigned long framesPerBuffer,
                                        unsigned long numberOfBuffers,
                                        PortAudioCallback *callback,
                                        void *userData ) = Pa_OpenDefaultStream_audev;
PaError (*Pa_CloseStream_FctPtr)( PortAudioStream *stream ) =Pa_CloseStream_audev;
PaError (*Pa_Initialize_FctPtr)( void ) =Pa_Initialize_audev;
PaError (*Pa_GetSampleSize_FctPtr)( PaSampleFormat format ) =Pa_GetSampleSize_audev;
void (*Abort_Pa_CloseStream_FctPtr) (void) =Abort_Pa_CloseStream_audev;


void set_paula_devide(void)
{
   Pa_StartStream_FctPtr = Pa_StartStream_audev;
   Pa_AbortStream_FctPtr = Pa_AbortStream_audev;
   Pa_StreamActive_FctPtr = Pa_StreamActive_audev;
   Pa_OpenDefaultStream_FctPtr= Pa_OpenDefaultStream_audev;
   Pa_CloseStream_FctPtr = Pa_CloseStream_audev;
   Pa_Initialize_FctPtr = Pa_Initialize_audev;
   Pa_GetSampleSize_FctPtr = Pa_GetSampleSize_audev;
   Abort_Pa_CloseStream_FctPtr = Abort_Pa_CloseStream_audev;

   //printf("Setting portaudio to to audio.device\n");
}

void set_ahi_devide(unsigned int unit)
{
   Pa_StartStream_FctPtr = Pa_StartStream_ahidev;
   Pa_AbortStream_FctPtr = Pa_AbortStream_ahidev;
   Pa_StreamActive_FctPtr = Pa_StreamActive_ahidev;
   Pa_OpenDefaultStream_FctPtr= Pa_OpenDefaultStream_ahidev;
   Pa_CloseStream_FctPtr = Pa_CloseStream_ahidev;
   Pa_Initialize_FctPtr = Pa_Initialize_ahidev;
   Pa_GetSampleSize_FctPtr = Pa_GetSampleSize_ahidev;
   Abort_Pa_CloseStream_FctPtr = Abort_Pa_CloseStream_ahidev;
   g_AHI_Unit=unit;
   //printf("Setting portaudio to to ahi.device, Unit %d\n",unit);
}


PaError Pa_StartStream( PortAudioStream *stream )
{
	return (*Pa_StartStream_FctPtr)( stream);
}


PaError Pa_AbortStream( PortAudioStream *stream )
{
	return (*Pa_AbortStream_FctPtr)( stream);
}

PaError Pa_StreamActive( PortAudioStream *stream )
{
	return (*Pa_StreamActive_FctPtr)( stream );
}

PaError Pa_OpenDefaultStream( PortAudioStream** stream,
                              int numInputChannels,
                              int numOutputChannels,
                              PaSampleFormat sampleFormat,
                              double sampleRate,
                              unsigned long framesPerBuffer,
                              unsigned long numberOfBuffers,
                              PortAudioCallback *callback,
                              void *userData )
{
	return (*Pa_OpenDefaultStream_FctPtr)( stream,
                                           numInputChannels,
                                           numOutputChannels,
                                           sampleFormat,
                                           sampleRate,
                                           framesPerBuffer,
                                           numberOfBuffers,
                                           callback,
                                           userData );

}

PaError Pa_CloseStream( PortAudioStream *stream )
{
	return (*Pa_CloseStream_FctPtr)( stream );
}

PaError Pa_Initialize( void )
{
	return (*Pa_Initialize_FctPtr)();
}

PaError Pa_GetSampleSize( PaSampleFormat format )
{
	return (*Pa_GetSampleSize_FctPtr)( format );
}

void __attribute__((no_instrument_function)) Abort_Pa_CloseStream (void)
{
	(*Abort_Pa_CloseStream_FctPtr)();
}

