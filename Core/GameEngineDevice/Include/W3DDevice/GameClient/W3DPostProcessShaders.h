/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 Electronic Arts Inc.
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

// FILE: W3DPostProcessShaders.h ///////////////////////////////////////////////
// GeneralsX @feature fbraz3 08/10/2026 Precompiled DirectX pixel shader bytecode for post-processing:
//              Bloom bright-pass extraction, separable Gaussian blur,
//              bloom composite with HDR tone-mapping, and FXAA anti-aliasing.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "WW3D2/dx8wrapper.h"

namespace W3DPostProcessShaders
{
	//-------------------------------------------------------------------------
	// Copy Shader (Pass-through blit)
	// ps.1.1
	// tex t0
	// mov r0, t0
	//-------------------------------------------------------------------------
	static const DWORD Copy[] = {
		0xFFFF0101,
		0x00000042, 0xB00F0000,                         // tex t0
		0x00000001, 0x800F0000, 0xB0E40000,             // mov r0, t0
		0x0000FFFF                                      // end
	};

	//-------------------------------------------------------------------------
	// Bloom Extract Shader
	// Isolates bright regions with luminance > threshold (0.70) and boosts
	// overbright excess for downscaled bloom generation.
	// ps.1.1
	// def c0, 0.299, 0.587, 0.114, 0.0             ; Luminance weights
	// def c1, 0.70, 0.70, 0.70, 0.0                ; Brightness threshold
	// def c2, 2.0, 2.0, 2.0, 1.0                   ; Bloom boost factor
	// tex t0
	// dp3 r1.rgb, t0, c0                           ; Compute luminance
	// sub_sat r1.rgb, r1, c1                       ; Threshold clamp
	// mul r0.rgb, t0, r1                           ; Modulate color by excess
	// mul r0.rgb, r0, c2                           ; Boost energy
	// +mov r0.a, c2.a
	//-------------------------------------------------------------------------
	static const DWORD BloomExtract[] = {
		0xFFFF0101,
		0x00000051, 0xA00F0000, 0x3E991687, 0x3F1645A2, 0x3DE978D5, 0x00000000, // def c0
		0x00000051, 0xA00F0001, 0x3F333333, 0x3F333333, 0x3F333333, 0x00000000, // def c1 (0.70)
		0x00000051, 0xA00F0002, 0x40000000, 0x40000000, 0x40000000, 0x3F800000, // def c2 (2.0)
		0x00000042, 0xB00F0000,                                                 // tex t0
		0x00000008, 0x80070001, 0xB0E40000, 0xA0E40000,                         // dp3 r1.rgb, t0, c0
		0x00000003, 0x80170001, 0x80E40001, 0xA0E40001,                         // sub_sat r1.rgb, r1, c1
		0x00000005, 0x80070000, 0xB0E40000, 0x80E40001,                         // mul r0.rgb, t0, r1
		0x00000005, 0x80070000, 0x80E40000, 0xA0E40002,                         // mul r0.rgb, r0, c2
		0x40000001, 0x80080000, 0xA0E40002,                                     // +mov r0.a, c2.a
		0x0000FFFF                                                               // end
	};

	//-------------------------------------------------------------------------
	// 4-Tap Separable Gaussian Blur Shader
	// Takes 4 offset samples per pass; run horizontally then vertically
	// over quarter-resolution buffer to achieve wide 16-pixel screen coverage.
	// ps.1.1
	// def c0, 0.25, 0.25, 0.25, 0.25
	// tex t0
	// tex t1
	// tex t2
	// tex t3
	// add r0, t0, t1
	// add r1, t2, t3
	// add r0, r0, r1
	// mul r0, r0, c0
	//-------------------------------------------------------------------------
	static const DWORD BloomBlur4Tap[] = {
		0xFFFF0101,
		0x00000051, 0xA00F0000, 0x3E800000, 0x3E800000, 0x3E800000, 0x3E800000, // def c0 (0.25)
		0x00000042, 0xB00F0000,                                                 // tex t0
		0x00000042, 0xB00F0001,                                                 // tex t1
		0x00000042, 0xB00F0002,                                                 // tex t2
		0x00000042, 0xB00F0003,                                                 // tex t3
		0x00000002, 0x800F0000, 0xB0E40000, 0xB0E40001,                         // add r0, t0, t1
		0x00000002, 0x800F0001, 0xB0E40002, 0xB0E40003,                         // add r1, t2, t3
		0x00000002, 0x800F0000, 0x80E40000, 0x80E40001,                         // add r0, r0, r1
		0x00000005, 0x800F0000, 0x80E40000, 0xA0E40000,                         // mul r0, r0, c0
		0x0000FFFF                                                               // end
	};

	//-------------------------------------------------------------------------
	// Bloom Composite Shader (Additive blend with clamp, without HDR)
	// ps.1.1
	// def c0, 0.65, 0.65, 0.65, 1.0                 ; Bloom intensity
	// tex t0                                        ; Scene color
	// tex t1                                        ; Bloom halo
	// mad_sat r0.rgb, t1, c0, t0                    ; clamp(scene + bloom * 0.65)
	// +mov r0.a, t0.a
	//-------------------------------------------------------------------------
	static const DWORD BloomComposite[] = {
		0xFFFF0101,
		0x00000051, 0xA00F0000, 0x3F266666, 0x3F266666, 0x3F266666, 0x3F800000, // def c0 (0.65)
		0x00000042, 0xB00F0000,                                                 // tex t0
		0x00000042, 0xB00F0001,                                                 // tex t1
		0x00000004, 0x80170000, 0xB0E40001, 0xA0E40000, 0xB0E40000,             // mad_sat r0.rgb, t1, c0, t0
		0x40000001, 0x80080000, 0xB0E40000,                                     // +mov r0.a, t0.a
		0x0000FFFF                                                               // end
	};

	//-------------------------------------------------------------------------
	// Bloom Composite & HDR Tone-Mapping Shader
	// Blends scene color with bloom halo, applying highlight soft-shoulder
	// compression to avoid clipping artifacts on intense detonations.
	// ps.1.1
	// def c0, 0.65, 0.65, 0.65, 1.0                 ; Bloom intensity
	// def c1, 0.20, 0.20, 0.20, 0.0                 ; Tone-curve shoulder
	// tex t0                                        ; Scene color
	// tex t1                                        ; Bloom halo
	// mad r0.rgb, t1, c0, t0                        ; scene + bloom * intensity
	// mul r1.rgb, r0, c1
	// mov r1.rgb, 1-r1                              ; 1.0 - r0 * shoulder
	// mul r0.rgb, r0, r1                            ; Soft compressed output
	// +mov r0.a, t0.a
	//-------------------------------------------------------------------------
	static const DWORD BloomCompositeHDR[] = {
		0xFFFF0101,
		0x00000051, 0xA00F0000, 0x3F266666, 0x3F266666, 0x3F266666, 0x3F800000, // def c0 (0.65)
		0x00000051, 0xA00F0001, 0x3E4CCCCD, 0x3E4CCCCD, 0x3E4CCCCD, 0x00000000, // def c1 (0.20)
		0x00000042, 0xB00F0000,                                                 // tex t0
		0x00000042, 0xB00F0001,                                                 // tex t1
		0x00000004, 0x80070000, 0xB0E40001, 0xA0E40000, 0xB0E40000,             // mad r0.rgb, t1, c0, t0
		0x00000005, 0x80070001, 0x80E40000, 0xA0E40001,                         // mul r1.rgb, r0, c1
		0x00000001, 0x80070001, 0x86E40001,                                     // mov r1.rgb, 1-r1
		0x00000005, 0x80070000, 0x80E40000, 0x80E40001,                         // mul r0.rgb, r0, r1
		0x40000001, 0x80080000, 0xB0E40000,                                     // +mov r0.a, t0.a
		0x0000FFFF                                                               // end
	};

	//-------------------------------------------------------------------------
	// HDR Tone-Mapping Only Shader (When Bloom is disabled)
	// ps.1.1
	// def c1, 0.20, 0.20, 0.20, 0.0
	// tex t0
	// mul r1.rgb, t0, c1
	// mov r1.rgb, 1-r1
	// mul r0.rgb, t0, r1
	// +mov r0.a, t0.a
	//-------------------------------------------------------------------------
	static const DWORD ToneMapOnly[] = {
		0xFFFF0101,
		0x00000051, 0xA00F0001, 0x3E4CCCCD, 0x3E4CCCCD, 0x3E4CCCCD, 0x00000000, // def c1 (0.20)
		0x00000042, 0xB00F0000,                                                 // tex t0
		0x00000005, 0x80070001, 0xB0E40000, 0xA0E40001,                         // mul r1.rgb, t0, c1
		0x00000001, 0x80070001, 0x86E40001,                                     // mov r1.rgb, 1-r1
		0x00000005, 0x80070000, 0xB0E40000, 0x80E40001,                         // mul r0.rgb, t0, r1
		0x40000001, 0x80080000, 0xB0E40000,                                     // +mov r0.a, t0.a
		0x0000FFFF                                                               // end
	};

	//-------------------------------------------------------------------------
	// FXAA / Fast Screen-Space Anti-Aliasing Shader
	// Tests diagonal luminance contrast and blends neighbour average across
	// high-contrast geometry edges, preserving flat regions untouched.
	// ps.1.1
	// def c0, 0.299, 0.587, 0.114, 0.0
	// def c1, 0.333333, 0.333333, 0.333333, 1.0
	// tex t0                                        ; Center sample
	// tex t1                                        ; NW sample
	// tex t2                                        ; NE sample
	// tex t3                                        ; SW sample
	// add r0, t1, t2
	// add r0, r0, t3
	// mul r0, r0, c1                                ; Neighbour average
	// sub r1, t1, t3                                ; Diagonal gradient
	// dp3 r1.rgb, r1, c0                            ; Scalar luminance difference
	// lrp r0.rgb, r1, r0, t0                        ; Blend if edge detected
	// +mov r0.a, t0.a
	//-------------------------------------------------------------------------
	static const DWORD FXAA[] = {
		0xFFFF0101,
		0x00000051, 0xA00F0000, 0x3E991687, 0x3F1645A2, 0x3DE978D5, 0x00000000, // def c0
		0x00000051, 0xA00F0001, 0x3EAAAAAF, 0x3EAAAAAF, 0x3EAAAAAF, 0x3F800000, // def c1 (0.333)
		0x00000042, 0xB00F0000,                                                 // tex t0
		0x00000042, 0xB00F0001,                                                 // tex t1
		0x00000042, 0xB00F0002,                                                 // tex t2
		0x00000042, 0xB00F0003,                                                 // tex t3
		0x00000002, 0x800F0000, 0xB0E40001, 0xB0E40002,                         // add r0, t1, t2
		0x00000002, 0x800F0000, 0x80E40000, 0xB0E40003,                         // add r0, r0, t3
		0x00000005, 0x800F0000, 0x80E40000, 0xA0E40001,                         // mul r0, r0, c1
		0x00000003, 0x800F0001, 0xB0E40001, 0xB0E40003,                         // sub r1, t1, t3
		0x00000008, 0x80070001, 0x80E40001, 0xA0E40000,                         // dp3 r1.rgb, r1, c0
		0x00000012, 0x80070000, 0x80E40001, 0x80E40000, 0xB0E40000,             // lrp r0.rgb, r1, r0, t0
		0x40000001, 0x80080000, 0xB0E40000,                                     // +mov r0.a, t0.a
		0x0000FFFF                                                               // end
	};
}
