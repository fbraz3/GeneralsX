/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//
// rc4.cpp
// RC4 encryption / decryption
//
#include "rc4.h"
#include <memory.h>
#include <stdio.h>

static unsigned char RC4_Temp_Byte;
#define RC4_SWAP_BYTE(a,b) RC4_Temp_Byte=a; a=b; b=RC4_Temp_Byte

//
// Used to init the key state vector in key setup
//
static unsigned char RC4_Table_Init[]={
	  0,   1,   2,   3,   4,
	  5,   6,   7,   8,   9,
	 10,  11,  12,  13,  14,
	 15,  16,  17,  18,  19,
	 20,  21,  22,  23,  24,
	 25,  26,  27,  28,  29,
	 30,  31,  32,  33,  34,
	 35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,
	 45,  46,  47,  48,  49,
	 50,  51,  52,  53,  54,
	 55,  56,  57,  58,  59,
	 60,  61,  62,  63,  64,
	 65,  66,  67,  68,  69,
	 70,  71,  72,  73,  74,
	 75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,
	 85,  86,  87,  88,  89,
	 90,  91,  92,  93,  94,
	 95,  96,  97,  98,  99,
	100, 101, 102, 103, 104,
	105, 106, 107, 108, 109,
	110, 111, 112, 113, 114,
	115, 116, 117, 118, 119,
	120, 121, 122, 123, 124,
	125, 126, 127, 128, 129,
	130, 131, 132, 133, 134,
	135, 136, 137, 138, 139,
	140, 141, 142, 143, 144,
	145, 146, 147, 148, 149,
	150, 151, 152, 153, 154,
	155, 156, 157, 158, 159,
	160, 161, 162, 163, 164,
	165, 166, 167, 168, 169,
	170, 171, 172, 173, 174,
	175, 176, 177, 178, 179,
	180, 181, 182, 183, 184,
	185, 186, 187, 188, 189,
	190, 191, 192, 193, 194,
	195, 196, 197, 198, 199,
	200, 201, 202, 203, 204,
	205, 206, 207, 208, 209,
	210, 211, 212, 213, 214,
	215, 216, 217, 218, 219,
	220, 221, 222, 223, 224,
	225, 226, 227, 228, 229,
	230, 231, 232, 233, 234,
	235, 236, 237, 238, 239,
	240, 241, 242, 243, 244,
	245, 246, 247, 248, 249,
	250, 251, 252, 253, 254,
	255
};





//
// Don't rely on this to zero the key
//
RC4Class::RC4Class()
{
	memset(Key.State, 0, 256);
	Key.X=0;
	Key.Y=0;
}



//
// Setup the encryption key.  This must be called before you encrypt/decrypt!
//
void RC4Class::Prepare_Key(const unsigned char *key_data_ptr, int key_data_len)
{
	switch(key_data_len) {
		case 8:
			Prepare_Key_8bytes(key_data_ptr);
			break;

		case 16:
			Prepare_Key_16bytes(key_data_ptr);
			break;

		default:
		{
			unsigned char	index1;
			unsigned char	index2;
			unsigned char	*state;
			unsigned short	counter;

			state = &Key.State[0];
			memcpy(state, RC4_Table_Init, 256);

			Key.X = 0;
			Key.Y = 0;
			index1 = 0;
			index2 = 0;
			for (counter = 0; counter < 256; counter++) {
				index2 = (key_data_ptr[index1] + state[counter] + index2);
				RC4_SWAP_BYTE(state[counter], state[index2]);
				index1 = (unsigned char)((index1 + 1) % key_data_len);
			}
		}
	} // switch
}


void RC4Class::Print_State(void) {
	unsigned char	*state;
	state = &Key.State[0];
	for (int i=0; i<256; i+=5) {
		printf("	%3d	%3d	%3d	%3d	%3d\n",state[i], state[i+1], state[i+2], state[i+3], state[i+4]);
	}
	printf("X = %d		Y = %d\n\n",Key.X,Key.Y);
}



//
// RC4 in standard mode.
//
// This will XOR the buffer with the RC4 stream (like a one time pad).
//
void RC4Class::RC4(unsigned char *buffer_ptr, int buffer_len)
{
	unsigned char x;
	unsigned char y;
	unsigned char *state;
	unsigned char *buffer_end=buffer_ptr+buffer_len;
	unsigned char *buffer_cur=buffer_ptr;

	x = Key.X;
	y = Key.Y;

	state = &Key.State[0];
	while(buffer_cur != buffer_end) {
		x++;
		y = (unsigned char)(y + state[x]);
		RC4_SWAP_BYTE(state[x], state[y]);
		*buffer_cur ^= state[(state[x] + state[y]) & 255];
		++buffer_cur;
	}

	Key.X = x;
	Key.Y = y;
}




//
// Copy state & key
//
RC4Class &RC4Class::operator=(const RC4Class &other) {
	if (this == &other)
		return(*this);

	Key.X=other.Key.X;
	Key.Y=other.Key.Y;
	memcpy(Key.State, other.Key.State, 256);

	return(*this);
}



//////////////////////////// private methods ////////////////////////////



//
// Setup the encryption key.  This must be called before you encrypt/decrypt!
// This version assumes the key len is 8 bytes (64 bits).
//
void RC4Class::Prepare_Key_8bytes(const unsigned char *key_data_ptr)
{
	unsigned char	index1;
	unsigned char	index2;
	unsigned char	*state;
	unsigned short	counter;

	state = &Key.State[0];
	memcpy(state, RC4_Table_Init, 256);

	Key.X = 0;
	Key.Y = 0;
	index1 = 0;
	index2 = 0;
	for (counter = 0; counter < 256; counter++) {
		index2 = (unsigned char)(key_data_ptr[index1] + state[counter] + index2);
		RC4_SWAP_BYTE(state[counter], state[index2]);
		++index1;
		index1 &= 0x07;
	}
}



//
// Setup the encryption key.  This must be called before you encrypt/decrypt!
// This version assumes the key len is 16 bytes (128 bits).
//
void RC4Class::Prepare_Key_16bytes(const unsigned char *key_data_ptr)
{
	unsigned char	index1;
	unsigned char	index2;
	unsigned char	*state;
	unsigned short	counter;

	state = &Key.State[0];
	memcpy(state, RC4_Table_Init, 256);

	Key.X = 0;
	Key.Y = 0;
	index1 = 0;
	index2 = 0;
	for (counter = 0; counter < 256; counter++) {
		index2 = (unsigned char)(key_data_ptr[index1] + state[counter] + index2);
		RC4_SWAP_BYTE(state[counter], state[index2]);
		++index1;
		index1 &= 0x0F;
	}
}
