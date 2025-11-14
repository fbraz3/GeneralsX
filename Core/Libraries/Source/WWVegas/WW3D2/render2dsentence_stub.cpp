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

/*
**	Cross-platform stub implementation of FontCharsClass and Render2DSentenceClass
**	This provides basic rendering for text on non-Windows platforms.
**	On Windows, the native GDI implementation (render2dsentence.cpp) is preferred.
*/

#include "render2dsentence.h"
#include "surfaceclass.h"
#include "wwstring.h"
#include "always.h"

/*
** FontCharsClassCharDataStruct implementation (trivial - data struct)
*/

/*
** FontCharsBuffer implementation (trivial - data struct)
*/

/*
** FontCharsClass implementation
*/

FontCharsClass::FontCharsClass(void) 
	: RefCountClass(),
	  AlternateUnicodeFont(NULL),
	  CurrPixelOffset(0),
	  CharHeight(16),
	  CharAscent(12),
	  CharOverhang(0),
	  PixelOverlap(0),
	  PointSize(12),
	  OldGDIFont(NULL),
	  OldGDIBitmap(NULL),
	  GDIBitmap(NULL),
	  GDIFont(NULL),
	  GDIBitmapBits(NULL),
	  MemDC(NULL),
	  FirstUnicodeChar(0x100),
	  LastUnicodeChar(0),
	  IsBold(false),
	  UnicodeCharArray(NULL)
{
	// Initialize ASCII array
	for (int i = 0; i < 256; i++) {
		ASCIICharArray[i] = NULL;
	}
	Name = "DefaultFont";
}

FontCharsClass::~FontCharsClass()
{
	Free_GDI_Font();
	Free_Character_Arrays();
	if (AlternateUnicodeFont) {
		AlternateUnicodeFont->Release_Ref();
	}
}

bool FontCharsClass::Initialize_GDI_Font(const char *font_name, int point_size, bool is_bold)
{
	if (!font_name || point_size <= 0) {
		return false;
	}
	
	Name = font_name;
	PointSize = point_size;
	IsBold = is_bold;
	GDIFontName = font_name;
	
	// On Windows, Create_GDI_Font() uses actual GDI. On other platforms, it's a stub.
	bool success = Create_GDI_Font(font_name);
	
	return success;
}

bool FontCharsClass::Is_Font(const char *font_name, int point_size, bool is_bold)
{
	if (!font_name) {
		return false;
	}
	return (Name == font_name && PointSize == point_size && IsBold == is_bold);
}

int FontCharsClass::Get_Char_Width(WCHAR ch)
{
	const FontCharsClassCharDataStruct *char_data = Get_Char_Data(ch);
	if (char_data) {
		return char_data->Width;
	}
	// Default width if not found
	return CharHeight / 2;
}

int FontCharsClass::Get_Char_Spacing(WCHAR ch)
{
	return Get_Char_Width(ch) + PixelOverlap;
}

void FontCharsClass::Blit_Char(WCHAR ch, uint16 *dest_ptr, int dest_stride, int x, int y)
{
	// Stub: actual character blitting would go here
	// For now, this is a no-op on non-Windows platforms
}

bool FontCharsClass::Create_GDI_Font(const char *font_name)
{
	// Cross-platform stub implementation
	// On Windows, this is overridden by the native version in render2dsentence.cpp
	// On other platforms, we use sensible defaults
	
	if (!font_name) {
		return false;
	}
	
	CharHeight = PointSize;
	CharAscent = PointSize - 2;
	CharOverhang = 0;
	PixelOverlap = PointSize / 8;
	
	if (PixelOverlap < 0) PixelOverlap = 0;
	if (PixelOverlap > 4) PixelOverlap = 4;
	
	return true;
}

void FontCharsClass::Free_GDI_Font(void)
{
	// Cleanup GDI resources (if any were allocated on Windows)
	if (OldGDIBitmap) {
		// On Windows, this would call DeleteObject(OldGDIBitmap)
		OldGDIBitmap = NULL;
	}
	if (OldGDIFont) {
		// On Windows, this would call DeleteObject(OldGDIFont)
		OldGDIFont = NULL;
	}
	if (GDIBitmap) {
		GDIBitmap = NULL;
	}
	if (GDIFont) {
		GDIFont = NULL;
	}
	if (GDIBitmapBits) {
		delete[] GDIBitmapBits;
		GDIBitmapBits = NULL;
	}
	if (MemDC) {
		MemDC = NULL;
	}
}

const FontCharsClassCharDataStruct *FontCharsClass::Store_GDI_Char(WCHAR ch)
{
	// Stub: storing a character would go here
	return NULL;
}

void FontCharsClass::Update_Current_Buffer(int char_width)
{
	// Stub: buffer management would go here
}

const FontCharsClassCharDataStruct *FontCharsClass::Get_Char_Data(WCHAR ch)
{
	if (ch < 256) {
		return ASCIICharArray[ch];
	} else if (UnicodeCharArray && ch >= FirstUnicodeChar && ch <= LastUnicodeChar) {
		return UnicodeCharArray[ch - FirstUnicodeChar];
	}
	return NULL;
}

void FontCharsClass::Grow_Unicode_Array(WCHAR ch)
{
	// Stub: Unicode array growth would go here
}

void FontCharsClass::Free_Character_Arrays(void)
{
	// Free ASCII characters
	for (int i = 0; i < 256; i++) {
		if (ASCIICharArray[i]) {
			delete ASCIICharArray[i];
			ASCIICharArray[i] = NULL;
		}
	}
	
	// Free Unicode characters
	if (UnicodeCharArray) {
		int count = LastUnicodeChar - FirstUnicodeChar + 1;
		for (int i = 0; i < count; i++) {
			if (UnicodeCharArray[i]) {
				delete UnicodeCharArray[i];
				UnicodeCharArray[i] = NULL;
			}
		}
		delete[] UnicodeCharArray;
		UnicodeCharArray = NULL;
	}
}

/*
** Render2DSentenceClass implementation
*/

Render2DSentenceClass::Render2DSentenceClass(void)
	: Font(NULL),
	  TextureOffset(0, 0),
	  TextureStartX(0),
	  CurrTextureSize(256),
	  TextureSizeHint(0),
	  CurSurface(NULL),
	  MonoSpaced(false),
	  WrapWidth(0),
	  Centered(false),
	  IsClippedEnabled(false),
	  ParseHotKey(false),
	  useHardWordWrap(false),
	  LockedPtr(NULL),
	  LockedStride(0),
	  CurTexture(NULL)
{
	BaseLocation.Set(0, 0);
	Location.Set(0, 0);
	Cursor.Set(0, 0);
	ClipRect.Set(0, 0, 0, 0);
	DrawExtents.Set(0, 0, 0, 0);
}

Render2DSentenceClass::~Render2DSentenceClass()
{
	Reset();
	if (Font) {
		Font->Release_Ref();
		Font = NULL;
	}
}

void Render2DSentenceClass::Render(void)
{
	// Stub: text rendering would go here
}

void Render2DSentenceClass::Reset(void)
{
	Reset_Sentence_Data();
	Release_Pending_Surfaces();
}

void Render2DSentenceClass::Reset_Polys(void)
{
	// Stub: polygon reset would go here
}

void Render2DSentenceClass::Set_Font(FontCharsClass *font)
{
	if (Font) {
		Font->Release_Ref();
	}
	Font = font;
	if (Font) {
		Font->Add_Ref();
	}
}

void Render2DSentenceClass::Set_Location(const Vector2 &loc)
{
	Location = loc;
	Cursor = loc;
}

void Render2DSentenceClass::Set_Base_Location(const Vector2 &loc)
{
	BaseLocation = loc;
	Location = loc;
	Cursor = loc;
}

void Render2DSentenceClass::Make_Additive(void)
{
	// Stub: shader modification would go here
}

void Render2DSentenceClass::Set_Shader(ShaderClass shader)
{
	Shader = shader;
}

Vector2 Render2DSentenceClass::Get_Text_Extents(const WCHAR *text)
{
	Vector2 extents(0, 16);  // Default char height
	if (!Font || !text) {
		return extents;
	}
	
	int width = 0;
	for (const WCHAR *p = text; *p; ++p) {
		width += Font->Get_Char_Spacing(*p);
	}
	extents.Set((float)width, (float)Font->Get_Char_Height());
	
	return extents;
}

Vector2 Render2DSentenceClass::Get_Formatted_Text_Extents(const WCHAR *text)
{
	// Stub: formatted text would handle wrapping, etc.
	return Get_Text_Extents(text);
}

void Render2DSentenceClass::Build_Sentence(const WCHAR *text, int *hkX, int *hkY)
{
	Reset_Sentence_Data();
	if (!text || !Font) {
		return;
	}
	
	if (hkX) *hkX = -1;
	if (hkY) *hkY = -1;
	
	// Stub: sentence building would go here
}

void Render2DSentenceClass::Draw_Sentence(uint32 color)
{
	// Stub: actual drawing would go here
}

void Render2DSentenceClass::Reset_Sentence_Data(void)
{
	SentenceData.Delete_All();
	Cursor = Location;
}

void Render2DSentenceClass::Build_Textures(void)
{
	// Stub: texture building would go here
}

void Render2DSentenceClass::Record_Sentence_Chunk(void)
{
	// Stub: chunk recording would go here
}

void Render2DSentenceClass::Allocate_New_Surface(const WCHAR *text, bool justCalcExtents)
{
	// Stub: surface allocation would go here
}

void Render2DSentenceClass::Release_Pending_Surfaces(void)
{
	PendingSurfaces.Delete_All();
}

void Render2DSentenceClass::Build_Sentence_Centered(const WCHAR *text, int *hkX, int *hkY)
{
	// Stub: centered sentence building would go here
}

Vector2 Render2DSentenceClass::Build_Sentence_Not_Centered(const WCHAR *text, int *hkX, int *hkY, bool justCalcExtents)
{
	// Stub: non-centered sentence building would go here
	return Vector2(0, 0);
}
