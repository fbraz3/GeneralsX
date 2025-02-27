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

#ifndef RC4_H
#define RC4_H

//
// RC4.h - Implementation of RC4 encryption
//
//	RC4 is a stream cypher.  This means that it basically produces a stream of
//		random bytes that you XOR with your data.  Each key is somewhat like a
//		one time pad.
//
//	Just as you should never re-use a one time pad, you should never re-use a key.
//
//	If you can't re-exchange a secret key before every message you could keep a
//		partial secret key and then include the other part of the key in plaintext.
//		The key would be the concatenation of the two parts of the key.	
//

class RC4Class
{
public:

	RC4Class();

	//
	// Key length can be 0..256 bytes
	// Key preparation takes about 0.015 Ms on a 1Ghz PC
	// It's 3x faster if your keylen is 8 or 16 bytes!
	//
	void Prepare_Key(const unsigned char *key_data_ptr, int key_data_len);

	//
	// In-place encryption.  Call Prepare_Key first!
	// Only a few clock cycles per byte (9 or so...)
	//
	void RC4(unsigned char *buffer_ptr, int buffer_len);


 	//
	// Copy state & key
	//
	RC4Class &operator=(const RC4Class &other);


	void Print_State(void);

private:
	void Prepare_Key_16bytes(const unsigned char *key_data_ptr);
	void Prepare_Key_8bytes(const unsigned char *key_data_ptr);

	struct RC4Key
	{      
		unsigned char State[256];       
		unsigned char X;        
		unsigned char Y;
	};

	RC4Key	Key;
};

#endif
