/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 GeneralsX contributors
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

#pragma once

// One 2:1 box reduction for every platform, used to build each mip level (D3DXLoadSurfaceFromSurface).
// Averages each 2x2 block: (sum + 2) >> 2 for 8-bit and 5-bit channels; a 1-bit channel is set when 2+ of 4 are.
// gli's FILTER_LINEAR mipmaps point-sampled half of each level, which aliased and varied with atlas position.
// A 1-texel-wide or -high level repeats its single row or column. No D3D types, so it is testable offline.

#include <cstdint>

namespace d3dx8box
{

enum Layout
{
	LAYOUT_UNSUPPORTED,
	LAYOUT_8888,		// A8R8G8B8, X8R8G8B8: four 8-bit channels
	LAYOUT_1555,		// A1R5G5B5, X1R5G5B5
	LAYOUT_565,			// R5G6B5
	LAYOUT_4444,		// A4R4G4B4, X4R4G4B4
};

struct Channel
{
	uint32_t shift;
	uint32_t bits;
};

template <typename Pixel, int ChannelCount>
inline void reduce(const uint8_t *src, uint32_t srcPitch, uint32_t srcWidth, uint32_t srcHeight,
	uint8_t *dst, uint32_t dstPitch, uint32_t dstWidth, uint32_t dstHeight, const Channel (&channels)[ChannelCount])
{
	const uint32_t xStep = srcWidth > 1 ? 1 : 0;
	const uint32_t yStep = srcHeight > 1 ? 1 : 0;
	for (uint32_t y = 0; y < dstHeight; y++)
	{
		const Pixel *row0 = (const Pixel *)(src + (y * 2) * srcPitch);
		const Pixel *row1 = (const Pixel *)(src + (y * 2 + yStep) * srcPitch);
		Pixel *out = (Pixel *)(dst + y * dstPitch);
		for (uint32_t x = 0; x < dstWidth; x++)
		{
			const uint32_t sx0 = x * 2;
			const uint32_t sx1 = sx0 + xStep;
			const uint32_t p00 = row0[sx0];
			const uint32_t p10 = row0[sx1];
			const uint32_t p01 = row1[sx0];
			const uint32_t p11 = row1[sx1];
			uint32_t result = 0;
			for (int c = 0; c < ChannelCount; c++)
			{
				const uint32_t shift = channels[c].shift;
				const uint32_t mask = (1u << channels[c].bits) - 1;
				const uint32_t sum = ((p00 >> shift) & mask) + ((p10 >> shift) & mask) +
				                     ((p01 >> shift) & mask) + ((p11 >> shift) & mask);
				const uint32_t value = channels[c].bits == 1 ? (sum >= 2 ? 1u : 0u) : ((sum + 2) >> 2);
				result |= value << shift;
			}
			out[x] = (Pixel)result;
		}
	}
}

// False when the layout is unknown or dst is not the next mip level of src (max(1, n / 2) on each axis).
inline bool reduceLevel(Layout layout, const void *src, uint32_t srcPitch, uint32_t srcWidth, uint32_t srcHeight,
	void *dst, uint32_t dstPitch, uint32_t dstWidth, uint32_t dstHeight)
{
	const uint32_t halfWidth = srcWidth > 1 ? srcWidth / 2 : 1;
	const uint32_t halfHeight = srcHeight > 1 ? srcHeight / 2 : 1;
	if (srcWidth == 0 || srcHeight == 0 || dstWidth != halfWidth || dstHeight != halfHeight)
	{
		return false;
	}

	const uint8_t *s = (const uint8_t *)src;
	uint8_t *d = (uint8_t *)dst;
	switch (layout)
	{
		case LAYOUT_8888:
		{
			static const Channel c[4] = { {24, 8}, {16, 8}, {8, 8}, {0, 8} };
			reduce<uint32_t>(s, srcPitch, srcWidth, srcHeight, d, dstPitch, dstWidth, dstHeight, c);
			return true;
		}
		case LAYOUT_1555:
		{
			static const Channel c[4] = { {15, 1}, {10, 5}, {5, 5}, {0, 5} };
			reduce<uint16_t>(s, srcPitch, srcWidth, srcHeight, d, dstPitch, dstWidth, dstHeight, c);
			return true;
		}
		case LAYOUT_565:
		{
			static const Channel c[3] = { {11, 5}, {5, 6}, {0, 5} };
			reduce<uint16_t>(s, srcPitch, srcWidth, srcHeight, d, dstPitch, dstWidth, dstHeight, c);
			return true;
		}
		case LAYOUT_4444:
		{
			static const Channel c[4] = { {12, 4}, {8, 4}, {4, 4}, {0, 4} };
			reduce<uint16_t>(s, srcPitch, srcWidth, srcHeight, d, dstPitch, dstWidth, dstHeight, c);
			return true;
		}
		default:
			return false;
	}
}

} // namespace d3dx8box
