#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "reciter.h"
#include "sam.h"
#include "debug.h"

#ifdef USESDL
#include <SDL.h>
#include <SDL_audio.h>
#endif

#include "endian.h"                                                 // AF, Endian

#include <time.h>  // AF, Test
#ifdef __AMIGA__                                                    // AF, use ahi.device instead of audio.device
	void set_ahi_devide(unsigned int unit);
    void SetCpuSpecificFunctions(void);                             // AF, we compile some functions for 68000 and for 68020
#endif


void WriteWav(char* filename, char* buffer, int bufferlength)
{
    FILE *file = fopen(filename, "wb");
    unsigned int le_bufferlength;                                   // AF, Endian
    if (file == NULL) return;
    //RIFF header
    fwrite("RIFF", 4, 1,file);
    unsigned int filesize=htolew(bufferlength + 12 + 16 + 8 - 8);   // AF, Endian
    fwrite(&filesize, 4, 1, file);
    fwrite("WAVE", 4, 1, file);

    //format chunk
    fwrite("fmt ", 4, 1, file);
    unsigned int fmtlength = htolew(16);                            // AF, Endian
    fwrite(&fmtlength, 4, 1, file);
    unsigned short int format=htoles(1); //PCM                      // AF, Endian
    fwrite(&format, 2, 1, file);
    unsigned short int channels=htoles(1);                          // AF, Endian
    fwrite(&channels, 2, 1, file);
    unsigned int samplerate = htolew(22050);                        // AF, Endian
    fwrite(&samplerate, 4, 1, file);
    fwrite(&samplerate, 4, 1, file); // bytes/second
    unsigned short int blockalign = htoles(1);                      // AF, Endian
    fwrite(&blockalign, 2, 1, file);
    unsigned short int bitspersample=htoles(8);                     // AF, Endian
    fwrite(&bitspersample, 2, 1, file);

    //data chunk
    fwrite("data", 4, 1, file);
    le_bufferlength=htolew(bufferlength);                           // AF, Endian
    fwrite(&le_bufferlength, 4, 1, file);
    fwrite(buffer, bufferlength, 1, file);

    fclose(file);
}

void PrintUsage()
{
    printf("usage: sam [options] Word1 Word2 ....\n");
    printf("options\n");
    printf("    -phonetic         enters phonetic mode. (see below)\n");
    printf("    -pitch number        set pitch value (default=64)\n");
    printf("    -speed number        set speed value (default=72)\n");
    printf("    -throat number        set throat value (default=128)\n");
    printf("    -mouth number        set mouth value (default=128)\n");
    printf("    -wav filename        output to wav instead of libsdl\n");
    printf("    -sing            special treatment of pitch\n");
    printf("    -debug            print additional debug messages\n");
#ifdef __AMIGA__
    printf("    -ahi unit         use Amiga AHI-device\n");
#endif
    printf("\n");


    printf("     VOWELS                            VOICED CONSONANTS    \n");
    printf("IY           f(ee)t                    R        red        \n");
    printf("IH           p(i)n                     L        allow        \n");
    printf("EH           beg                       W        away        \n");
    printf("AE           Sam                       W        whale        \n");
    printf("AA           pot                       Y        you        \n");
    printf("AH           b(u)dget                  M        Sam        \n");
    printf("AO           t(al)k                    N        man        \n");
    printf("OH           cone                      NX       so(ng)        \n");
    printf("UH           book                      B        bad        \n");
    printf("UX           l(oo)t                    D        dog        \n");
    printf("ER           bird                      G        again        \n");
    printf("AX           gall(o)n                  J        judge        \n");
    printf("IX           dig(i)t                   Z        zoo        \n");
    printf("                       ZH       plea(s)ure    \n");
    printf("   DIPHTHONGS                          V        seven        \n");
    printf("EY           m(a)de                    DH       (th)en        \n");
    printf("AY           h(igh)                        \n");
    printf("OY           boy                        \n");
    printf("AW           h(ow)                     UNVOICED CONSONANTS    \n");
    printf("OW           slow                      S         Sam        \n");
    printf("UW           crew                      Sh        fish        \n");
    printf("                                       F         fish        \n");
    printf("                                       TH        thin        \n");
    printf(" SPECIAL PHONEMES                      P         poke        \n");
    printf("UL           sett(le) (=AXL)           T         talk        \n");
    printf("UM           astron(omy) (=AXM)        K         cake        \n");
    printf("UN           functi(on) (=AXN)         CH        speech        \n");
    printf("Q            kitt-en (glottal stop)    /H        a(h)ead    \n");
}

#ifdef USESDL

int pos = 0;
void MixAudio(void *unused, Uint8 *stream, int len)
{
    int bufferpos = GetBufferLength();
    char *buffer = GetBuffer();
    int i;

    if (pos >= bufferpos) return;
    if ((bufferpos-pos) < len) len = (bufferpos-pos);
    for(i=0; i<len; i++)
    {
        stream[i] = buffer[pos];
        pos++;
    }
}


void OutputSound()
{
    int bufferpos = GetBufferLength();
    bufferpos /= 50;
    SDL_AudioSpec fmt;

    fmt.freq = 22050;
    fmt.format = AUDIO_U8;
    fmt.channels = 1;
    fmt.samples = 2048;
    fmt.callback = MixAudio;
    fmt.userdata = NULL;

    /* Open the audio device and start playing sound! */
    if ( SDL_OpenAudio(&fmt, NULL) < 0 )
    {
        printf("Unable to open audio: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_PauseAudio(0);
    //SDL_Delay((bufferpos)/7);

    while (pos < bufferpos)
    {
        SDL_Delay(100);
    }

    SDL_CloseAudio();
}

#else

void OutputSound() {}

#endif

int debug = 0;


int main(int argc, char **argv)
{
    int i;
    int phonetic = 0;

    char* wavfilename = NULL;
    static char input[256];   // AF, save some stack

    printf("Los...\n");
    time_t StartZeit=time(NULL);

#ifdef USESDL
#ifndef __AMIGA__
//        freopen("CON", "w", stdout);
//        freopen("CON", "w", stderr);
#endif
#endif

#ifdef __AMIGA__
        SetCpuSpecificFunctions();
#endif

    for(i=0; i<256; i++) input[i] = 0;

    if (argc <= 1)
    {
        PrintUsage();
        return 1;
    }

    i = 1;
    while(i < argc)
    {
        if (argv[i][0] != '-')
        {
            strncat(input, argv[i], 255);
            strncat(input, " ", 255);
        } else
        {
            if (strcmp(&argv[i][1], "wav")==0)
            {
                wavfilename = argv[i+1];
                i++;
            } else
            if (strcmp(&argv[i][1], "sing")==0)
            {
                EnableSingmode();
            } else
            if (strcmp(&argv[i][1], "phonetic")==0)
            {
                phonetic = 1;
            } else
            if (strcmp(&argv[i][1], "debug")==0)
            {
                debug = 1;
            } else
            if (strcmp(&argv[i][1], "pitch")==0)
            {
                SetPitch(atoi(argv[i+1]));
                i++;
            } else
            if (strcmp(&argv[i][1], "speed")==0)
            {
                SetSpeed(atoi(argv[i+1]));
                i++;
            } else
            if (strcmp(&argv[i][1], "mouth")==0)
            {
                SetMouth(atoi(argv[i+1]));
                i++;
            } else
            if (strcmp(&argv[i][1], "throat")==0)
            {
                SetThroat(atoi(argv[i+1]));
                i++;
            } else
#ifdef __AMIGA__
			if (strcmp(&argv[i][1], "ahi")==0)
			{
				unsigned int unit;
				if(argc>i+1)
				{
					if(argv[i+1])
					{
						//unit=strtoul(argv[i+1],NULL,10);
						unit=atoi(argv[i+1]);
						if(unit<4)
						{
							set_ahi_devide(unit);
						}
						else
						{
							printf("AHI-unit %d out of range (0...3)\n",unit);
							return 1;
						}
					}
					else
					{
						printf("AHI-unit is NULL-Ptr!\n");
						return 1;
					}
				}
				else
				{
					printf("missing AHI unit\n");
					return 1;
				}
				i++;
			} else
#endif
            {
                PrintUsage();
                return 1;
            }
        }

        i++;
    } //while

    for(i=0; input[i] != 0; i++)
        input[i] = toupper((int)input[i]);

    if (debug)
    {
        if (phonetic) printf("phonetic input: %s\n", input);
        else printf("text input: %s\n", input);
    }

    if (!phonetic)
    {
        strncat(input, "[", 256);
        if (!TextToPhonemes((unsigned char *)input)) return 1;
        if (debug)
            printf("phonetic input: %s\n", input);
    } else strncat(input, "\x9b", 256);

#ifdef USESDL
    if ( SDL_Init(SDL_INIT_AUDIO) < 0 )
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
#endif

    SetInput(input);
    if (!SAMMain())
    {
        PrintUsage();
        return 1;
    }
{
printf("Sound berechnung nach %lld Sekunden\n",time(NULL)-StartZeit);
extern int bufferpos;
printf("%s(): BufferPos=%d\n",__FUNCTION__,bufferpos);
printf("%s(): GetBufferLength=%d\n",__FUNCTION__,GetBufferLength());
}
    if (wavfilename != NULL)
        WriteWav(wavfilename, GetBuffer(), GetBufferLength()/50);
    else
        OutputSound();


    return 0;

}
