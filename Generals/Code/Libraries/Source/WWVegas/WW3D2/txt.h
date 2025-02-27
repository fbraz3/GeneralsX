/*
**	Command & Conquer Generals(tm)
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

#ifndef TXT_H
#define TXT_H

class FontClass;
class ConvertClass;
class TextureClass;
class TextTextureClass {
	public:
		TextTextureClass(void);
		~TextTextureClass(void);
	
		// function to rebuild the texture with the new parameters
		bool Build_Texture(FontClass &font, const char *str, int fore, int back, ConvertClass &conv);

		// returns a pointer to the texture that we created
		TextureClass	*Get_Texture(void) {return Texture;}

		// accessor method for the size of the texture we created
		int				Get_Texture_Size(void) {return TextureSize;}
	private:
		// function to determine if the texture needs to be rebuilt based on its parameters
		bool 	Is_Texture_Valid(FontClass &font, const char *str, int fore, int back, ConvertClass &conv);

		// all the parameters we need to remember to deterimine if the texture is changing
		// (changing any of these means the texture needs to be rebuilt)
		char				*TextureString;
		FontClass		*Font;
		ConvertClass	*Convert;
		int				ForegroundColor;
		int				BackgroundColor;

		// pointer to the created texture if we have created it so far
		TextureClass	*Texture;

		// the size of the texture we created (created textures are square)
		int				TextureSize;
	
};

#endif