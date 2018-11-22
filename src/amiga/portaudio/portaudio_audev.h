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
