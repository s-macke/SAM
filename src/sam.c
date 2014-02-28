#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "sam.h"
#include "SamTabs.h"

char input[256]; //tab39445
//standard sam sound
unsigned char speed = 72;
unsigned char pitch = 64;
unsigned char mouth = 128;
unsigned char throat = 128;
static int singmode = 0;

extern int debug;



unsigned char wait1 = 7;
unsigned char wait2 = 6;

unsigned char mem39;
unsigned char mem44;
unsigned char mem47;
unsigned char mem49;
unsigned char mem50;
unsigned char mem51;
unsigned char mem53;
unsigned char mem56;

unsigned char mem59=0;

unsigned char A, X, Y;

unsigned char stress[256]; //numbers from 0 to 8
unsigned char phonemeLength[256]; //tab40160
unsigned char phonemeindex[256];

unsigned char phonemeIndexOutput[60]; //tab47296
unsigned char stressOutput[60]; //tab47365
unsigned char phonemeLengthOutput[60]; //tab47416

unsigned char tab44800[256];

unsigned char pitches[256]; // tab43008

unsigned char frequency1[256];
unsigned char frequency2[256];
unsigned char frequency3[256];

unsigned char amplitude1[256];
unsigned char amplitude2[256];
unsigned char amplitude3[256];

//timetable for more accurate c64 simulation
int timetable[5][5] =
{
	{162, 167, 167, 127, 128},
	{226, 60, 60, 0, 0},
	{225, 60, 59, 0, 0},
	{200, 0, 0, 54, 55},
	{199, 0, 0, 54, 54}
};

// contains the final soundbuffer
int bufferpos=0;
char *buffer = NULL;

void Output(int index, unsigned char A)
{
	static unsigned oldtimetableindex = 0;
	int k;
	bufferpos += timetable[oldtimetableindex][index];
	oldtimetableindex = index;
	// write a little bit in advance
	for(k=0; k<5; k++)
		buffer[bufferpos/50 + k] = (A & 15)*16;
}

void SetInput(char *_input)
{
	int i, l;
	l = strlen(_input);
	if (l > 254) l = 254;
	for(i=0; i<l; i++)
		input[i] = _input[i];
	input[l] = 0;
}

void SetSpeed(unsigned char _speed) {speed = _speed;};
void SetPitch(unsigned char _pitch) {pitch = _pitch;};
void SetMouth(unsigned char _mouth) {mouth = _mouth;};
void SetThroat(unsigned char _throat) {throat = _throat;};
void EnableSingmode() {singmode = 1;};
char* GetBuffer(){return buffer;};
int GetBufferLength(){return bufferpos;};

void Init();
int Parser1();
void Parser2();
int SAMMain();
void CopyStress();
void SetPhonemeLength();
void Code48619();
void Code41240();
void Insert(unsigned char position, unsigned char mem60, unsigned char mem59, unsigned char mem58);
void Code48431();
void Render();
void PrepareOutput();
void Code48227();
void SetMouthThroat(unsigned char mouth, unsigned char throat);

// 168=pitches 
// 169=frequency1
// 170=frequency2
// 171=frequency3
// 172=amplitude1
// 173=amplitude2
// 174=amplitude3


void Init()
{
	int i;
	SetMouthThroat( mouth, throat);

	bufferpos = 0;
	// TODO, check for free the memory, 10 seconds of output should be more than enough
	buffer = malloc(22050*10); 

	/*
	freq2data = &mem[45136];
	freq1data = &mem[45056];
	freq3data = &mem[45216];
	*/
	//pitches = &mem[43008];
	/*
	frequency1 = &mem[43264];
	frequency2 = &mem[43520];
	frequency3 = &mem[43776];
	*/
	/*
	amplitude1 = &mem[44032];
	amplitude2 = &mem[44288];
	amplitude3 = &mem[44544];
	*/
	//phoneme = &mem[39904];
	/*
	ampl1data = &mem[45296];
	ampl2data = &mem[45376];
	ampl3data = &mem[45456];
	*/

	for(i=0; i<256; i++)
	{
		pitches[i] = 0;
		amplitude1[i] = 0;
		amplitude2[i] = 0;
		amplitude3[i] = 0;
		frequency1[i] = 0;
		frequency2[i] = 0;
		frequency3[i] = 0;
		stress[i] = 0;
		phonemeLength[i] = 0;
		tab44800[i] = 0;
	}
	
	for(i=0; i<60; i++)
	{
		phonemeIndexOutput[i] = 0;
		stressOutput[i] = 0;
		phonemeLengthOutput[i] = 0;
	}
	phonemeindex[255] = 255; //to prevent buffer overflow // ML : changed from 32 to 255 to stop freezing with long inputs

}

//written by me because of different table positions.
// mem[47] = ...
// 168=pitches
// 169=frequency1
// 170=frequency2
// 171=frequency3
// 172=amplitude1
// 173=amplitude2
// 174=amplitude3
unsigned char Read(unsigned char p, unsigned char Y)
{
	switch(p)
	{
	case 168: return pitches[Y];
	case 169: return frequency1[Y];
	case 170: return frequency2[Y];
	case 171: return frequency3[Y];
	case 172: return amplitude1[Y];
	case 173: return amplitude2[Y];
	case 174: return amplitude3[Y];
	}
	printf("Error reading to tables");
	return 0;
}

void Write(unsigned char p, unsigned char Y, unsigned char value)
{

	switch(p)
	{
	case 168: pitches[Y] = value; return;
	case 169: frequency1[Y] = value;  return;
	case 170: frequency2[Y] = value;  return;
	case 171: frequency3[Y] = value;  return;
	case 172: amplitude1[Y] = value;  return;
	case 173: amplitude2[Y] = value;  return;
	case 174: amplitude3[Y] = value;  return;
	}
	printf("Error writing to tables\n");
}

//int Code39771()
int SAMMain()
{
	Init();
	phonemeindex[255] = 32; //to prevent buffer overflow

	if (!Parser1()) return 0;
	if (debug)
		PrintPhonemes(phonemeindex, phonemeLength, stress);
	Parser2();
	CopyStress();
	SetPhonemeLength();
	Code48619();
	Code41240();
	do
	{
		A = phonemeindex[X];
		if (A > 80)
		{
			phonemeindex[X] = 255;
			break; // error: delete all behind it
		}
		X++;
	} while (X != 0);

	//pos39848:
	Code48431();

	//mem[40158] = 255;

	PrepareOutput();

	if (debug) 
	{
		PrintPhonemes(phonemeindex, phonemeLength, stress);
		PrintOutput(tab44800, frequency1, frequency2, frequency3, amplitude1, amplitude2, amplitude3, pitches);
	}
	return 1;
}


//void Code48547()
void PrepareOutput()
{
	A = 0;
	X = 0;
	Y = 0;

	//pos48551:
	while(1)
	{
		A = phonemeindex[X];
		if (A == 255)
		{
			A = 255;
			phonemeIndexOutput[Y] = 255;
			Render();
			return;
		}
		if (A == 254)
		{
			X++;
			int temp = X;
			//mem[48546] = X;
			phonemeIndexOutput[Y] = 255;
			Render();
			//X = mem[48546];
			X=temp;
			Y = 0;
			continue;
		}

		if (A == 0)
		{
			X++;
			continue;
		}

		phonemeIndexOutput[Y] = A;
		phonemeLengthOutput[Y] = phonemeLength[X];
		stressOutput[Y] = stress[X];
		X++;
		Y++;
	}
}

void Code48431()
{
	unsigned char mem54;
	unsigned char mem55;
	unsigned char index; //variable Y
	mem54 = 255;
	X++;
	mem55 = 0;
	unsigned char mem66 = 0;
	while(1)
	{
		//pos48440:
		X = mem66;
		index = phonemeindex[X];
		if (index == 255) return;
		mem55 += phonemeLength[X];

		if (mem55 < 232)
		{
			if (index != 254) // ML : Prevents an index out of bounds problem		
			{
				A = flags2[index]&1;
				if(A != 0)
				{
					X++;
					mem55 = 0;
					Insert(X, 254, mem59, 0);
					mem66++;
					mem66++;
					continue;
				}
			}
			if (index == 0) mem54 = X;
			mem66++;
			continue;
		}
		X = mem54;
		phonemeindex[X] = 31;   // 'Q*' glottal stop
		phonemeLength[X] = 4;
		stress[X] = 0;
		X++;
		mem55 = 0;
		Insert(X, 254, mem59, 0);
		X++;
		mem66 = X;
	}

}

// Iterates through the phoneme buffer, copying the stress value from
// the following phoneme under the following circumstance:
       
//     1. The current phoneme is voiced, excluding plosives and fricatives
//     2. The following phoneme is voiced, excluding plosives and fricatives, and
//     3. The following phoneme is stressed
//
//  In those cases, the stress value+1 from the following phoneme is copied.
//
// For example, the word LOITER is represented as LOY5TER, with as stress
// of 5 on the dipthong OY. This routine will copy the stress value of 6 (5+1)
// to the L that precedes it.


//void Code41883()
void CopyStress()
{
    // loop thought all the phonemes to be output
	unsigned char pos=0; //mem66
	while(1)
	{
        // get the phomene
		Y = phonemeindex[pos];
		
	    // exit at end of buffer
		if (Y == 255) return;
		
		// if CONSONANT_FLAG set, skip - only vowels get stress
		if ((flags[Y] & 64) == 0) {pos++; continue;}
		// get the next phoneme
		Y = phonemeindex[pos+1];
		if (Y == 255) //prevent buffer overflow
		{
			pos++; continue;
		} else
		// if the following phoneme is a vowel, skip
		if ((flags[Y] & 128) == 0)  {pos++; continue;}

        // get the stress value at the next position
		Y = stress[pos+1];
		
		// if next phoneme is not stressed, skip
		if (Y == 0)  {pos++; continue;}

		// if next phoneme is not a VOWEL OR ER, skip
		if ((Y & 128) != 0)  {pos++; continue;}

		// copy stress from prior phoneme to this one
		stress[pos] = Y+1;
		
		// advance pointer
		pos++;
	}

}


//void Code41014()
void Insert(unsigned char position/*var57*/, unsigned char mem60, unsigned char mem59, unsigned char mem58)
{
	int i;
	for(i=253; i >= position; i--) // ML : always keep last safe-guarding 255	
	{
		phonemeindex[i+1] = phonemeindex[i];
		phonemeLength[i+1] = phonemeLength[i];
		stress[i+1] = stress[i];
	}

	phonemeindex[position] = mem60;
	phonemeLength[position] = mem59;
	stress[position] = mem58;
	return;
}

// The input[] buffer contains a string of phonemes and stress markers along
// the lines of:
//
//     DHAX KAET IHZ AH5GLIY. <0x9B>
//
// The byte 0x9B marks the end of the buffer. Some phonemes are 2 bytes 
// long, such as "DH" and "AX". Others are 1 byte long, such as "T" and "Z". 
// There are also stress markers, such as "5" and ".".
//
// The first character of the phonemes are stored in the table signInputTable1[].
// The second character of the phonemes are stored in the table signInputTable2[].
// The stress characters are arranged in low to high stress order in stressInputTable[].
// 
// The following process is used to parse the input[] buffer:
// 
// Repeat until the <0x9B> character is reached:
//
//        First, a search is made for a 2 character match for phonemes that do not
//        end with the '*' (wildcard) character. On a match, the index of the phoneme 
//        is added to phonemeIndex[] and the buffer position is advanced 2 bytes.
//
//        If this fails, a search is made for a 1 character match against all
//        phoneme names ending with a '*' (wildcard). If this succeeds, the 
//        phoneme is added to phonemeIndex[] and the buffer position is advanced
//        1 byte.
// 
//        If this fails, search for a 1 character match in the stressInputTable[].
//        If this succeeds, the stress value is placed in the last stress[] table
//        at the same index of the last added phoneme, and the buffer position is
//        advanced by 1 byte.
//
//        If this fails, return a 0.
//
// On success:
//
//    1. phonemeIndex[] will contain the index of all the phonemes.
//    2. The last index in phonemeIndex[] will be 255.
//    3. stress[] will contain the stress value for each phoneme

// input[] holds the string of phonemes, each two bytes wide
// signInputTable1[] holds the first character of each phoneme
// signInputTable2[] holds te second character of each phoneme
// phonemeIndex[] holds the indexes of the phonemes after parsing input[]
//
// The parser scans through the input[], finding the names of the phonemes
// by searching signInputTable1[] and signInputTable2[]. On a match, it
// copies the index of the phoneme into the phonemeIndexTable[].
//
// The character <0x9B> marks the end of text in input[]. When it is reached,
// the index 255 is placed at the end of the phonemeIndexTable[], and the
// function returns with a 1 indicating success.
int Parser1()
{
	int i;
	unsigned char sign1;
	unsigned char sign2;
	unsigned char position = 0;
	X = 0;
	A = 0;
	Y = 0;
	
	// CLEAR THE STRESS TABLE
	for(i=0; i<256; i++)
		stress[i] = 0;

  // THIS CODE MATCHES THE PHONEME LETTERS TO THE TABLE
	// pos41078:
	while(1)
	{
        // GET THE FIRST CHARACTER FROM THE PHONEME BUFFER
		sign1 = input[X];
		// TEST FOR 155 (›) END OF LINE MARKER
		if (sign1 == 155)
		{
           // MARK ENDPOINT AND RETURN
			phonemeindex[position] = 255;      //mark endpoint
			// REACHED END OF PHONEMES, SO EXIT
			return 1;       //all ok
		}
		
		// GET THE NEXT CHARACTER FROM THE BUFFER
		X++;
		sign2 = input[X];
		
		// NOW sign1 = FIRST CHARACTER OF PHONEME, AND sign2 = SECOND CHARACTER OF PHONEME

       // TRY TO MATCH PHONEMES ON TWO TWO-CHARACTER NAME
       // IGNORE PHONEMES IN TABLE ENDING WITH WILDCARDS

       // SET INDEX TO 0
		Y = 0;
pos41095:
         
         // GET FIRST CHARACTER AT POSITION Y IN signInputTable
         // --> should change name to PhonemeNameTable1
		A = signInputTable1[Y];
		
		// FIRST CHARACTER MATCHES?
		if (A == sign1)
		{
           // GET THE CHARACTER FROM THE PhonemeSecondLetterTable
			A = signInputTable2[Y];
			// NOT A SPECIAL AND MATCHES SECOND CHARACTER?
			if ((A != '*') && (A == sign2))
			{
               // STORE THE INDEX OF THE PHONEME INTO THE phomeneIndexTable
				phonemeindex[position] = Y;
				
				// ADVANCE THE POINTER TO THE phonemeIndexTable
				position++;
				// ADVANCE THE POINTER TO THE phonemeInputBuffer
				X++;

				// CONTINUE PARSING
				continue;
			}
		}
		
		// NO MATCH, TRY TO MATCH ON FIRST CHARACTER TO WILDCARD NAMES (ENDING WITH '*')
		
		// ADVANCE TO THE NEXT POSITION
		Y++;
		// IF NOT END OF TABLE, CONTINUE
		if (Y != 81) goto pos41095;

// REACHED END OF TABLE WITHOUT AN EXACT (2 CHARACTER) MATCH.
// THIS TIME, SEARCH FOR A 1 CHARACTER MATCH AGAINST THE WILDCARDS

// RESET THE INDEX TO POINT TO THE START OF THE PHONEME NAME TABLE
		Y = 0;
pos41134:
// DOES THE PHONEME IN THE TABLE END WITH '*'?
		if (signInputTable2[Y] == '*')
		{
// DOES THE FIRST CHARACTER MATCH THE FIRST LETTER OF THE PHONEME
			if (signInputTable1[Y] == sign1)
			{
                // SAVE THE POSITION AND MOVE AHEAD
				phonemeindex[position] = Y;
				
				// ADVANCE THE POINTER
				position++;
				
				// CONTINUE THROUGH THE LOOP
				continue;
			}
		}
		Y++;
		if (Y != 81) goto pos41134; //81 is size of PHONEME NAME table

// FAILED TO MATCH WITH A WILDCARD. ASSUME THIS IS A STRESS
// CHARACTER. SEARCH THROUGH THE STRESS TABLE

        // SET INDEX TO POSITION 8 (END OF STRESS TABLE)
		Y = 8;
		
       // WALK BACK THROUGH TABLE LOOKING FOR A MATCH
		while( (sign1 != stressInputTable[Y]) && (Y>0))
		{
  // DECREMENT INDEX
			Y--;
		}

        // REACHED THE END OF THE SEARCH WITHOUT BREAKING OUT OF LOOP?
		if (Y == 0)
		{
			//mem[39444] = X;
			//41181: JSR 42043 //Error
           // FAILED TO MATCH ANYTHING, RETURN 0 ON FAILURE
			return 0;
		}
// SET THE STRESS FOR THE PRIOR PHONEME
		stress[position-1] = Y;
	} //while
}




//change phonemelength depedendent on stress
//void Code41203()
void SetPhonemeLength()
{
	unsigned char A;
	int position = 0;
	while(phonemeindex[position] != 255 )
	{
		A = stress[position];
		//41218: BMI 41229
		if ((A == 0) || ((A&128) != 0))
		{
			phonemeLength[position] = phonemeLengthTable1[phonemeindex[position]];
		} else
		{
			phonemeLength[position] = phonemeLengthTable2[phonemeindex[position]];
		}
		position++;
	}
}


void Code41240()
{
	unsigned char pos=0;

	while(phonemeindex[pos] != 255)
	{
		unsigned char index; //register AC
		X = pos;
		index = phonemeindex[pos];
		if ((flags[index]&2) == 0)
		{
			pos++;
			continue;
		} else
		if ((flags[index]&1) == 0)
		{
			Insert(pos+1, index+1, phonemeLengthTable1[index+1], stress[pos]);
			Insert(pos+2, index+2, phonemeLengthTable1[index+2], stress[pos]);
			pos += 3;
			continue;
		}

		do
		{
			X++;
			A = phonemeindex[X];
		} while(A==0);

		if (A != 255)
		{
			if ((flags[A] & 8) != 0)  {pos++; continue;}
			if ((A == 36) || (A == 37)) {pos++; continue;} // '/H' '/X'
		}

		Insert(pos+1, index+1, phonemeLengthTable1[index+1], stress[pos]);
		Insert(pos+2, index+2, phonemeLengthTable1[index+2], stress[pos]);
		pos += 3;
	};

}

// Rewrites the phonemes using the following rules:
//
//       <DIPTHONG ENDING WITH WX> -> <DIPTHONG ENDING WITH WX> WX
//       <DIPTHONG NOT ENDING WITH WX> -> <DIPTHONG NOT ENDING WITH WX> YX
//       UL -> AX L
//       UM -> AX M
//       <STRESSED VOWEL> <SILENCE> <STRESSED VOWEL> -> <STRESSED VOWEL> <SILENCE> Q <VOWEL>
//       T R -> CH R
//       D R -> J R
//       <VOWEL> R -> <VOWEL> RX
//       <VOWEL> L -> <VOWEL> LX
//       G S -> G Z
//       K <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPTHONG NOT ENDING WITH IY>
//       G <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPTHONG NOT ENDING WITH IY>
//       S P -> S B
//       S T -> S D
//       S K -> S G
//       S KX -> S GX
//       <ALVEOLAR> UW -> <ALVEOLAR> UX
//       CH -> CH CH' (CH requires two phonemes to represent it)
//       J -> J J' (J requires two phonemes to represent it)
//       <UNSTRESSED VOWEL> T <PAUSE> -> <UNSTRESSED VOWEL> DX <PAUSE>
//       <UNSTRESSED VOWEL> D <PAUSE>  -> <UNSTRESSED VOWEL> DX <PAUSE>


//void Code41397()
void Parser2()
{
	if (debug) printf("Parser2\n");
	unsigned char pos = 0; //mem66;
	unsigned char mem58 = 0;


  // Loop through phonemes
	while(1)
	{
// SET X TO THE CURRENT POSITION
		X = pos;
// GET THE PHONEME AT THE CURRENT POSITION
		A = phonemeindex[pos];

// DEBUG: Print phoneme and index
		if (debug && A != 255) printf("%d: %c%c\n", X, signInputTable1[A], signInputTable2[A]);

// Is phoneme pause?
		if (A == 0)
		{
// Move ahead to the 
			pos++;
			continue;
		}
		
// If end of phonemes flag reached, exit routine
		if (A == 255) return;
		
// Copy the current phoneme index to Y
		Y = A;

// RULE: 
//       <DIPTHONG ENDING WITH WX> -> <DIPTHONG ENDING WITH WX> WX
//       <DIPTHONG NOT ENDING WITH WX> -> <DIPTHONG NOT ENDING WITH WX> YX
// Example: OIL, COW


// Check for DIPTHONG
		if ((flags[A] & 16) == 0) goto pos41457;

// Not a dipthong. Get the stress
		mem58 = stress[pos];
		
// End in IY sound?
		A = flags[Y] & 32;
		
// If ends with IY, use YX, else use WX
		if (A == 0) A = 20; else A = 21;    // 'WX' = 20 'YX' = 21
		//pos41443:
// Insert at WX or YX following, copying the stress

		if (debug) if (A==20) printf("RULE: insert WX following dipthong NOT ending in IY sound\n");
		if (debug) if (A==21) printf("RULE: insert YX following dipthong ending in IY sound\n");
		Insert(pos+1, A, mem59, mem58);
		X = pos;
// Jump to ???
		goto pos41749;



pos41457:
         
// RULE:
//       UL -> AX L
// Example: MEDDLE
       
// Get phoneme
		A = phonemeindex[X];
// Skip this rule if phoneme is not UL
		if (A != 78) goto pos41487;  // 'UL'
		A = 24;         // 'L'                 //change 'UL' to 'AX L'
		
		if (debug) printf("RULE: UL -> AX L\n");

pos41466:
// Get current phoneme stress
		mem58 = stress[X];
		
// Change UL to AX
		phonemeindex[X] = 13;  // 'AX'
// Perform insert. Note code below may jump up here with different values
		Insert(X+1, A, mem59, mem58);
		pos++;
// Move to next phoneme
		continue;

pos41487:
         
// RULE:
//       UM -> AX M
// Example: ASTRONOMY
         
// Skip rule if phoneme != UM
		if (A != 79) goto pos41495;   // 'UM'
		// Jump up to branch - replaces current phoneme with AX and continues
		A = 27; // 'M'  //change 'UM' to  'AX M'
		if (debug) printf("RULE: UM -> AX M\n");
		goto pos41466;
pos41495:

// RULE:
//       UN -> AX N
// Example: FUNCTION

         
// Skip rule if phoneme != UN
		if (A != 80) goto pos41503; // 'UN'
		
		// Jump up to branch - replaces current phoneme with AX and continues
		A = 28;         // 'N' //change UN to 'AX N'
		if (debug) printf("RULE: UN -> AX N\n");
		goto pos41466;
pos41503:
         
// RULE:
//       <STRESSED VOWEL> <SILENCE> <STRESSED VOWEL> -> <STRESSED VOWEL> <SILENCE> Q <VOWEL>
// EXAMPLE: AWAY EIGHT
         
		Y = A;
// VOWEL set?
		A = flags[A] & 128;

// Skip if not a vowel
		if (A != 0)
		{
// Get the stress
			A = stress[X];

// If stressed...
			if (A != 0)
			{
// Get the following phoneme
				X++;
				A = phonemeindex[X];
// If following phoneme is a pause

				if (A == 0)
				{
// Get the phoneme following pause
					X++;
					Y = phonemeindex[X];

// Check for end of buffer flag
					if (Y == 255) //buffer overflow
// ??? Not sure about these flags
     					A = 65&128;
					else
// And VOWEL flag to current phoneme's flags
     					A = flags[Y] & 128;

// If following phonemes is not a pause
					if (A != 0)
					{
// If the following phoneme is not stressed
						A = stress[X];
						if (A != 0)
						{
// Insert a glottal stop and move forward
							if (debug) printf("RULE: Insert glottal stop between two stressed vowels with space between them\n");
							// 31 = 'Q'
							Insert(X, 31, mem59, 0);
							pos++;
							continue;
						}
					}
				}
			}
		}


// RULES FOR PHONEMES BEFORE R
//        T R -> CH R
// Example: TRACK


// Get current position and phoneme
		X = pos;
		A = phonemeindex[pos];
		if (A != 23) goto pos41611;     // 'R'
		
// Look at prior phoneme
		X--;
		A = phonemeindex[pos-1];
		//pos41567:
		if (A == 69)                    // 'T'
		{
// Change T to CH
			if (debug) printf("RULE: T R -> CH R\n");
			phonemeindex[pos-1] = 42;
			goto pos41779;
		}


// RULES FOR PHONEMES BEFORE R
//        D R -> J R
// Example: DRY

// Prior phonemes D?
		if (A == 57)                    // 'D'
		{
// Change D to J
			phonemeindex[pos-1] = 44;
			if (debug) printf("RULE: D R -> J R\n");
			goto pos41788;
		}

// RULES FOR PHONEMES BEFORE R
//        <VOWEL> R -> <VOWEL> RX
// Example: ART


// If vowel flag is set change R to RX
		A = flags[A] & 128;
		if (debug) printf("RULE: R -> RX\n");
		if (A != 0) phonemeindex[pos] = 18;  // 'RX'
		
// continue to next phoneme
		pos++;
		continue;

pos41611:

// RULE:
//       <VOWEL> L -> <VOWEL> LX
// Example: ALL

// Is phoneme L?
		if (A == 24)    // 'L'
		{
// If prior phoneme does not have VOWEL flag set, move to next phoneme
			if ((flags[phonemeindex[pos-1]] & 128) == 0) {pos++; continue;}
// Prior phoneme has VOWEL flag set, so change L to LX and move to next phoneme
			if (debug) printf("RULE: <VOWEL> L -> <VOWEL> LX\n");
			phonemeindex[X] = 19;     // 'LX'
			pos++;
			continue;
		}
		
// RULE:
//       G S -> G Z
//
// Can't get to fire -
//       1. The G -> GX rule intervenes
//       2. Reciter already replaces GS -> GZ

// Is current phoneme S?
		if (A == 32)    // 'S'
		{
// If prior phoneme is not G, move to next phoneme
			if (phonemeindex[pos-1] != 60) {pos++; continue;}
// Replace S with Z and move on
			if (debug) printf("RULE: G S -> G Z\n");
			phonemeindex[pos] = 38;    // 'Z'
			pos++;
			continue;
		}

// RULE:
//             K <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPTHONG NOT ENDING WITH IY>
// Example: COW

// Is current phoneme K?
		if (A == 72)    // 'K'
		{
// Get next phoneme
			Y = phonemeindex[pos+1];
// If at end, replace current phoneme with KX
			if (Y == 255) phonemeindex[pos] = 75; // ML : prevents an index out of bounds problem		
			else
			{
// VOWELS AND DIPTHONGS ENDING WITH IY SOUND flag set?
				A = flags[Y] & 32;
				if (debug) if (A==0) printf("RULE: K <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPTHONG NOT ENDING WITH IY>\n");
// Replace with KX
				if (A == 0) phonemeindex[pos] = 75;  // 'KX'
			}
		}
		else

// RULE:
//             G <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPTHONG NOT ENDING WITH IY>
// Example: GO


// Is character a G?
		if (A == 60)   // 'G'
		{
// Get the following character
			unsigned char index = phonemeindex[pos+1];
			
// At end of buffer?
			if (index == 255) //prevent buffer overflow
			{
				pos++; continue;
			}
			else
// If dipthong ending with YX, move continue processing next phoneme
			if ((flags[index] & 32) != 0) {pos++; continue;}
// replace G with GX and continue processing next phoneme
			if (debug) printf("RULE: G <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPTHONG NOT ENDING WITH IY>\n");
			phonemeindex[pos] = 63; // 'GX'
			pos++;
			continue;
		}
		
// RULE:
//      S P -> S B
//      S T -> S D
//      S K -> S G
//      S KX -> S GX
// Examples: SPY, STY, SKY, SCOWL
		
		Y = phonemeindex[pos];
		//pos41719:
// Replace with softer version?
		A = flags[Y] & 1;
		if (A == 0) goto pos41749;
		A = phonemeindex[pos-1];
		if (A != 32)    // 'S'
		{
			A = Y;
			goto pos41812;
		}
		// Replace with softer version
		if (debug) printf("RULE: S* %c%c -> S* %c%c\n", signInputTable1[Y], signInputTable2[Y],signInputTable1[Y-12], signInputTable2[Y-12]);
		phonemeindex[pos] = Y-12;
		pos++;
		continue;


pos41749:
         
// RULE:
//      <ALVEOLAR> UW -> <ALVEOLAR> UX
//
// Example: NEW, DEW, SUE, ZOO, THOO, TOO

//       UW -> UX

		A = phonemeindex[X];
		if (A == 53)    // 'UW'
		{
// ALVEOLAR flag set?
			Y = phonemeindex[X-1];
			A = flags2[Y] & 4;
// If not set, continue processing next phoneme
			if (A == 0) {pos++; continue;}
			if (debug) printf("RULE: <ALVEOLAR> UW -> <ALVEOLAR> UX\n");
			phonemeindex[X] = 16;
			pos++;
			continue;
		}
pos41779:

// RULE:
//       CH -> CH CH' (CH requires two phonemes to represent it)
// Example: CHEW

		if (A == 42)    // 'CH'
		{
			//        pos41783:
			if (debug) printf("CH -> CH CH+1\n");
			Insert(X+1, A+1, mem59, stress[X]);
			pos++;
			continue;
		}

pos41788:
         
// RULE:
//       J -> J J' (J requires two phonemes to represent it)
// Example: JAY
         

		if (A == 44) // 'J'
		{
			if (debug) printf("J -> J J+1\n");
			Insert(X+1, A+1, mem59, stress[X]);
			pos++;
			continue;
		}
		
// Jump here to continue 
pos41812:

// RULE: Soften T following vowel
// NOTE: This rule fails for cases such as "ODD"
//       <UNSTRESSED VOWEL> T <PAUSE> -> <UNSTRESSED VOWEL> DX <PAUSE>
//       <UNSTRESSED VOWEL> D <PAUSE>  -> <UNSTRESSED VOWEL> DX <PAUSE>
// Example: PARTY, TARDY


// Past this point, only process if phoneme is T or D
         
		if (A != 69)    // 'T'
		if (A != 57) {pos++; continue;}       // 'D'
		//pos41825:


// If prior phoneme is not a vowel, continue processing phonemes
		if ((flags[phonemeindex[X-1]] & 128) == 0) {pos++; continue;}
		
// Get next phoneme
		X++;
		A = phonemeindex[X];
		//pos41841
// Is the next phoneme a pause?
		if (A != 0)
		{
// If next phoneme is not a pause, continue processing phonemes
			if ((flags[A] & 128) == 0) {pos++; continue;}
// If next phoneme is stressed, continue processing phonemes
// FIXME: How does a pause get stressed?
			if (stress[X] != 0) {pos++; continue;}
//pos41856:
// Set phonemes to DX
		if (debug) printf("RULE: Soften T or D following vowel or ER and preceding a pause -> DX\n");
		phonemeindex[pos] = 30;       // 'DX'
		} else
		{
			A = phonemeindex[X+1];
			if (A == 255) //prevent buffer overflow
				A = 65 & 128;
			else
// Is next phoneme a vowel or ER?
				A = flags[A] & 128;
			if (debug) if (A != 0) printf("RULE: Soften T or D following vowel or ER and preceding a pause -> DX\n");
			if (A != 0) phonemeindex[pos] = 30;  // 'DX'
		}

		pos++;

	} // while
}



//change phoneme length
void Code48619()
{
	X = 0;
	unsigned char index;

	unsigned char mem66=0;
	while(1)
	{
		index = phonemeindex[X];
		if (index == 255) break;
		if((flags2[index] & 1) == 0)
		{
			X++;
			continue;
		}
		mem66 = X;
pos48644:
		X--;
		if(X == 0) break;
		index = phonemeindex[X];

		if (index != 255) //inserted to prevent access overrun
		if((flags[index] & 128) == 0) goto pos48644;

		//pos48657:
		do
		{
			index = phonemeindex[X];
			if (index != 255)//inserted to prevent access overrun
			if(((flags2[index] & 32) == 0) || ((flags[index] & 4) != 0))     //nochmal überprüfen
			{
				//A = flags[Y] & 4;
				//if(A == 0) goto pos48688;
				A = phonemeLength[X];
				A = (A >> 1) + A + 1;   // 3/2*A+1 ???
				phonemeLength[X] = A;
			}

			X++;
		} while (X != mem66);
		//	if (X != mem66) goto pos48657;

		X++;
	}  // while

	mem66 = 0;
	//pos48697

	while(1)
	{
		X = mem66;
		index = phonemeindex[X];
		if (index == 255) return;
		A = flags[index] & 128;
		if (A != 0)
		{

			X++;
			index = phonemeindex[X];
			if (index == 255) 
				mem56 = 65;
			else
				mem56 = flags[index];

			if ((flags[index] & 64) == 0)
			{
				if ((index == 18) || (index == 19))  // 'RX' & 'LX'
				{
					X++;
					index = phonemeindex[X];
					if ((flags[index] & 64) != 0)
					phonemeLength[mem66]--;
					mem66++;
					continue;
				}
				mem66++;
				continue;
			}

			if ((mem56 & 4) == 0)
			{
				if((mem56 & 1) == 0) {mem66++; continue;}
				X--;
				mem56 = phonemeLength[X] >> 3;
				phonemeLength[X] -= mem56;
				mem66++;
				continue;
			}
			A = phonemeLength[X-1];
			phonemeLength[X-1] = (A >> 2) + A + 1;     // 5/4*A + 1
			mem66++;
			continue;
			
		}

		//pos48821:

		if((flags2[index] & 8) != 0)
		{
			X++;
			index = phonemeindex[X];
			if (index == 255) A = 65&2;  //prevent buffer overflow
			else
			A = flags[index] & 2;
			if(A != 0)
			{
				phonemeLength[X] = 6;
				phonemeLength[X-1] = 5;
			}
			mem66++;
			continue;

		}


		if((flags[index] & 2) != 0)
		{
			do
			{
				X++;
				index = phonemeindex[X];
			} while(index == 0);
			if (index == 255) //buffer overflow
			{
				if ((65 & 2) == 0) {mem66++; continue;}
			} else
			if ((flags[index] & 2) == 0) {mem66++; continue;}
			
			phonemeLength[X] = (phonemeLength[X] >> 1) + 1;
			X = mem66;
			phonemeLength[mem66] = (phonemeLength[mem66] >> 1) + 1;
			mem66++;
			continue;
		}


		if ((flags2[index] & 16) != 0)
		{
			index = phonemeindex[X-1];
			if((flags[index] & 2) != 0) phonemeLength[X] -= 2;
		}

		mem66++;
		continue;
	}


	//	goto pos48701;
}

// -------------------------------------------------------------------------
// ML : Code47503 is division with remainder, and mem50 gets the sign
void Code47503(unsigned char mem52)
{

	Y = 0;
	if ((mem53 & 128) != 0)
	{
		mem53 = -mem53;
		Y = 128;
	}
	mem50 = Y;
	A = 0;
	for(X=8; X > 0; X--)
	{
		int temp = mem53;
		mem53 = mem53 << 1;
		A = A << 1;
		if (temp >= 128) A++;
		if (A >= mem52)
		{
			A = A - mem52;
			mem53++;
		}
	}

	mem51 = A;
	if ((mem50 & 128) != 0) mem53 = -mem53;

}

// -------------------------------------------------------------------------


void Code48227(unsigned char *mem66)
{
	int tempA;
	int i;
	mem49 = Y;
	A = mem39&7;
	X = A-1;
	mem56 = X;
	mem53 = tab48426[X];
	mem47 = X;      //46016+mem[56]*256
	A = mem39 & 248;
	if(A == 0)
	{
		Y = mem49;
		A = pitches[mem49] >> 4;
		goto pos48315;
	}
	Y = A ^ 255;
pos48274:
	mem56 = 8;
	A = randomtable[mem47*256+Y];
pos48280:

	tempA = A;
	A = A << 1;
	//48281: BCC 48290
	if ((tempA & 128) == 0)
	{
		X = mem53;
		//mem[54296] = X;
		Output(1, X);
		if(X != 0) goto pos48296;
	}
	
	Output(2, 5);

	//48295: NOP
pos48296:

	for(i=0; i<wait1; i++) //wait
	X = 0;

	mem56--;
	if (mem56 != 0) goto pos48280;
	Y++;
	if (Y != 0) goto pos48274;
	mem44 = 1;
	Y = mem49;
	return;


	unsigned char phase1;

pos48315:
	// Error Error Error

	phase1 = A ^ 255;
	Y = *mem66;
	do
	{
		//pos48321:

		mem56 = 8;
		//A = Read(mem47, Y);
		A = randomtable[mem47*256+Y];     //???


		//pos48327:
		do
		{
			//48327: ASL A
			//48328: BCC 48337
			tempA = A;
			A = A << 1;
			if ((tempA & 128) != 0)
			{
				X = 26;
				Output(3, X);
			} else
			{
				//timetable 4
				X=6;
				Output(4, X);
			}

			for(X = wait2; X>0; X--); //wait
			mem56--;
		} while(mem56 != 0);

		Y++;
		phase1++;

	} while (phase1 != 0);
	//	if (phase1 != 0) goto pos48321;
	A = 1;
	mem44 = 1;
	*mem66 = Y;
	Y = mem49;
	return;

	//exit(1);


	//Error Error Error
}

void Special1(unsigned char mem48, unsigned char phase1)
{

	//pos48372:
	//	mem48 = 255;
//pos48376:
	mem49 = X;
	A = X;
	int Atemp = A;
	A = A - 30;
	if (Atemp <= 30) A=0; // ???
	X = A;

	// ML : A =, fixes a problem with invalid pitch with '.'
	while( (A=pitches[X]) == 127) X++;


pos48398:
	//48398: CLC
	//48399: ADC 48
	A += mem48;
	phase1 = A;
	pitches[X] = A;
pos48406:
	X++;
	if (X == mem49) return; //goto pos47615;
	if (pitches[X] == 255) goto pos48406;
	A = phase1;
	goto pos48398;
}

//void Code47574()
void Render()
{
	unsigned char phase1 = 0;  //mem43
	unsigned char phase2;
	unsigned char phase3;
	unsigned char mem66;
	unsigned char mem38;
	unsigned char mem40;
	unsigned char speedcounter; //mem45
	unsigned char mem48;
	int i;
	int carry;
	if (phonemeIndexOutput[0] == 255) return; //exit if no data

	A = 0;
	X = 0;
	mem44 = 0;

// pos47587:
do
{
	Y = mem44;
	A = phonemeIndexOutput[mem44];
	mem56 = A;
	if (A == 255) break;
	if (A == 1)
	{
		//pos48366:
		A = 1;
		mem48 = 1;
		//goto pos48376;
		Special1(mem48, phase1);
	}
	/*
	if (A == 2) goto pos48372;
	*/
	if (A == 2)
	{
		mem48 = 255;
		Special1(mem48, phase1);
	}
	//	pos47615:

	phase1 = tab47492[stressOutput[Y] + 1];
	phase2 = phonemeLengthOutput[Y];
	Y = mem56;
	do
	{
		frequency1[X] = freq1data[Y];
		frequency2[X] = freq2data[Y];
		frequency3[X] = freq3data[Y];
		amplitude1[X] = ampl1data[Y];
		amplitude2[X] = ampl2data[Y];
		amplitude3[X] = ampl3data[Y];
		tab44800[X] = tab45936[Y];
		pitches[X] = pitch + phase1;
		X++;
		phase2--;
	} while(phase2 != 0);

	mem44++;
} while(mem44 != 0);
// -------------------
//pos47694:

	A = 0;
	mem44 = 0;
	mem49 = 0;
	X = 0;
	while(1) //while No. 1
	{
		//pos47701:
		Y = phonemeIndexOutput[X];
		A = phonemeIndexOutput[X+1];
		X++;
		if (A == 255) break;//goto pos47970;
		X = A;
		mem56 = blendRank[A];
		A = blendRank[Y];
		if (A == mem56)
		{
			phase1 = outBlendLength[Y];
			phase2 = outBlendLength[X];
		} else
		if (A < mem56)
		{
			phase1 = inBlendLength[X];
			phase2 = outBlendLength[X];
		} else
		{
			phase1 = outBlendLength[Y];
			phase2 = inBlendLength[Y];
		}

		Y = mem44;
		A = mem49 + phonemeLengthOutput[mem44];
		mem49 = A;
		A = A + phase2; //Maybe Problem because of carry flag
		//47776: ADC 42
		speedcounter = A;
		mem47 = 168;
		phase3 = mem49 - phase1;
		A = phase1 + phase2;
		mem38 = A;
		X = A;
		X -= 2;
		//47805: BPL 47810
		if ((X & 128) == 0)
		do   //while No. 2
		{
			//pos47810:

			mem40 = mem38;
			if (mem47 == 168)     //for amplitude1
			{
				unsigned char mem36, mem37;
				mem36 = phonemeLengthOutput[mem44] >> 1;
				mem37 = phonemeLengthOutput[mem44+1] >> 1;
				mem40 = mem36 + mem37;
				mem37 += mem49;
				mem36 = mem49 - mem36;
				A = Read(mem47, mem37);
				//A = mem[address];
				Y = mem36;
				mem53 = A - Read(mem47, mem36);
			} else
			{
				A = Read(mem47, speedcounter);
				Y = phase3;
				mem53 = A - Read(mem47, phase3);
			}
			
			//Code47503(mem40);
			// ML : Code47503 is division with remainder, and mem50 gets the sign
			mem50 = (((char)(mem53) < 0) ? 128 : 0);
			mem51 = abs((char)mem53) % mem40;
			mem53 = (unsigned char)((char)(mem53) / mem40);

			X = mem40;
			Y = phase3;

			mem56 = 0;
			//47907: CLC
			//pos47908:
			while(1)     //while No. 3
			{
				A = Read(mem47, Y) + mem53; //carry alway cleared

				mem48 = A;
				Y++;
				X--;
				if(X == 0) break;

				mem56 += mem51;
				if (mem56 >= mem40)  //???
				{
					/*
			47927: CMP 40
			47927: BCC 47945
			*/
					//47931: SBC 40
					mem56 -= mem40; //carry? is set
					//if ((mem56 & 128)==0)
					if ((mem50 & 128)==0)
					{
						//47935: BIT 50
						//47937: BMI 47943
						if(mem48 != 0) mem48++;
					} else mem48--;
				}
				//pos47945:
				Write(mem47, Y, mem48);
				//47949: CLC
				//47950: BCC 47908

				//goto pos47908;

			} //while No. 3

			//pos47952:
			mem47++;
			//if (mem47 != 175) goto pos47810;
		} while (mem47 != 175);     //while No. 2
		//pos47963:
		mem44++;
		X = mem44;
	}  //while No. 1

	//goto pos47701;
	//pos47970:

	mem48 = mem49 + phonemeLengthOutput[mem44];
	if (!singmode)
	{
		for(i=0; i<256; i++)
		pitches[i] -= (frequency1[i] >> 1);
	}

	phase1 = 0;
	phase2 = 0;
	phase3 = 0;
	mem49 = 0;
	speedcounter = 72; //sam standard speed

	//amplitude rescaling
	for(i=255; i>=0; i--)
	{
		amplitude1[i] = amplitudeRescale[amplitude1[i]];
		amplitude2[i] = amplitudeRescale[amplitude2[i]];
		amplitude3[i] = amplitudeRescale[amplitude3[i]];
	}

	Y = 0;
	A = pitches[0];
	mem44 = A;
	X = A;
	mem38 = A - (A>>2);     // 3/4*A ???

	//finally the loop for sound output
	//pos48078:
	while(1)
	{
		A = tab44800[Y];
		mem39 = A;
		A = A & 248;
		if(A != 0)
		{
			Code48227(&mem66);
			Y += 2;
			mem48 -= 2;
		} else
		{
			mem56 = multtable[sinus[phase1] | amplitude1[Y]];

			carry = 0;
			if ((mem56+multtable[sinus[phase2] | amplitude2[Y]] ) > 255) carry = 1;
			mem56 += multtable[sinus[phase2] | amplitude2[Y]];
			A = mem56 + multtable[rectangle[phase3] | amplitude3[Y]] + (carry?1:0);
			A = ((A + 136) & 255) >> 4; //there must be also a carry
			//mem[54296] = A;
			Output(0, A);
			speedcounter--;
			if (speedcounter != 0) goto pos48155;
			Y++; //go to next amplitude
			mem48--;
		}
		if(mem48 == 0) return;
		speedcounter = speed;
pos48155:
		mem44--;
		if(mem44 == 0)
		{
pos48159:

			A = pitches[Y];
			mem44 = A;
			A = A - (A>>2);
			mem38 = A;
			phase1 = 0;
			phase2 = 0;
			phase3 = 0;
			continue;
		}
		mem38--;
		if((mem38 != 0) || (mem39 == 0))
		{
			phase1 += frequency1[Y];
			phase2 += frequency2[Y];
			phase3 += frequency3[Y];
			continue;
		}
		Code48227(&mem66);
		goto pos48159;
	} //while

	//--------------------------
	// I am sure, but I think the following code is never executed
	//pos48315:
	int tempA;
	phase1 = A ^ 255;
	Y = mem66;
	do
	{
		//pos48321:

		mem56 = 8;
		A = Read(mem47, Y);

		//pos48327:
		do
		{
			//48327: ASL A
			//48328: BCC 48337
			tempA = A;
			A = A << 1;
			if ((tempA & 128) != 0)
			{
				X = 26;
				// mem[54296] = X;
				bufferpos += 150;
				buffer[bufferpos/50] = (X & 15)*16;
			} else
			{
				//mem[54296] = 6;
				X=6; 
				bufferpos += 150;
				buffer[bufferpos/50] = (X & 15)*16;
			}

			for(X = wait2; X>0; X--); //wait
			mem56--;
		} while(mem56 != 0);

		Y++;
		phase1++;

	} while (phase1 != 0);
	//	if (phase1 != 0) goto pos48321;
	A = 1;
	mem44 = 1;
	mem66 = Y;
	Y = mem49;
	return;
}

//return = (mem39212*mem39213) >> 1
unsigned char trans(unsigned char mem39212, unsigned char mem39213)
{
	//pos39008:
	unsigned char carry;
	int temp;
	unsigned char mem39214, mem39215;
	A = 0;
	mem39215 = 0;
	mem39214 = 0;
	X = 8;
	do
	{
		carry = mem39212 & 1;
		mem39212 = mem39212 >> 1;
		if (carry != 0)
		{
			/*
						39018: LSR 39212
						39021: BCC 39033
						*/
			carry = 0;
			A = mem39215;
			temp = (int)A + (int)mem39213;
			A = A + mem39213;
			if (temp > 255) carry = 1;
			mem39215 = A;
		}
		temp = mem39215 & 1;
		mem39215 = (mem39215 >> 1) | (carry?128:0);
		carry = temp;
		//39033: ROR 39215
		X--;
	} while (X != 0);
	temp = mem39214 & 128;
	mem39214 = (mem39214 << 1) | (carry?1:0);
	carry = temp;
	temp = mem39215 & 128;
	mem39215 = (mem39215 << 1) | (carry?1:0);
	carry = temp;

	return mem39215;
}


/*
    SAM's voice can be altered by changing the frequencies of the
    mouth formant (F1) and the throat formant (F2). Only the voiced
    phonemes (5-29 and 48-53) are altered.
*/
void SetMouthThroat(unsigned char mouth, unsigned char throat)
{
	unsigned char initialFrequency;
	unsigned char newFrequency = 0;
	//unsigned char mouth; //mem38880
	//unsigned char throat; //mem38881

	// mouth formants (F1) 5..29
	unsigned char mouthFormants5_29[30] = {
		0, 0, 0, 0, 0, 10,
		14, 19, 24, 27, 23, 21, 16, 20, 14, 18, 14, 18, 18,
		16, 13, 15, 11, 18, 14, 11, 9, 6, 6, 6};

	// throat formants (F2) 5..29
	unsigned char throatFormants5_29[30] = {
	255, 255,
	255, 255, 255, 84, 73, 67, 63, 40, 44, 31, 37, 45, 73, 49,
	36, 30, 51, 37, 29, 69, 24, 50, 30, 24, 83, 46, 54, 86};

	// there must be no zeros in this 2 tables
	// formant 1 frequencies (mouth) 48..53
	unsigned char mouthFormants48_53[6] = {19, 27, 21, 27, 18, 13};
       
	// formant 2 frequencies (throat) 48..53
	unsigned char throatFormants48_53[6] = {72, 39, 31, 43, 30, 34};

	unsigned char pos = 5; //mem39216
//pos38942:
	// recalculate formant frequencies 5..29 for the mouth (F1) and throat (F2)
	while(pos != 30)
	{
		// recalculate mouth frequency
		initialFrequency = mouthFormants5_29[pos];
		if (initialFrequency != 0) newFrequency = trans(mouth, initialFrequency);
		freq1data[pos] = newFrequency;
               
		// recalculate throat frequency
		initialFrequency = throatFormants5_29[pos];
		if(initialFrequency != 0) newFrequency = trans(throat, initialFrequency);
		freq2data[pos] = newFrequency;
		pos++;
	}

//pos39059:
	// recalculate formant frequencies 48..53
	pos = 48;
	Y = 0;
    while(pos != 54)
    {
		// recalculate F1 (mouth formant)
		initialFrequency = mouthFormants48_53[Y];
		newFrequency = trans(mouth, initialFrequency);
		freq1data[pos] = newFrequency;
           
		// recalculate F2 (throat formant)
		initialFrequency = throatFormants48_53[Y];
		newFrequency = trans(throat, initialFrequency);
		freq2data[pos] = newFrequency;
		Y++;
		pos++;
	}
}


