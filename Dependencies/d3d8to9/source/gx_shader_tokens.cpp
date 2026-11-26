// GeneralsX: D3D8 -> D3D9 shader bytecode without D3DX (patch 0003); see gx_shader_tokens.hpp.
//
// gx_vs8to9 is adapted from DXVK's TranslateVertexShader8 (src/d3d8/d3d8_shader.cpp, DXVK 2.6 as vendored in
// references/fbraz3-dxvk), which carries this notice:
//
//   Copyright (c) 2017 Philip Rebohle
//   Copyright (c) 2019 Joshua Ashton
//   Copyright (c) 2019 Robin Kertels
//   Copyright (c) 2023 Jeffrey Ellison
//
//   zlib/libpng license
//
//   This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held
//   liable for any damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any purpose, including commercial applications, and to
//   alter it and redistribute it freely, subject to the following restrictions:
//
//   - The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
//     If you use this software in a product, an acknowledgment in the product documentation would be appreciated but
//     is not required.
//   - Altered source versions must be plainly marked as such, and must not be misrepresented as being the original
//     software.
//   - This notice may not be removed or altered from any source distribution.
//
// Altered for d3d8to9 by GeneralsX: plain uint32_t tokens instead of DXVK's d3d9 types and logger; a declared constant
// becomes a def of c<n> (D3DSPR_CONST) where DXVK wrote D3DSPR_CONST2, the 2048+ bank its own d3d9 ignores and
// Microsoft's rejects; comment tokens are copied whole instead of being scanned as instructions; vs_1_0 becomes vs_1_1;
// every read is bounded; DXVK's forced-declaration option and debug log are gone.
#include "gx_shader_tokens.hpp"

namespace
{
	const std::uint32_t VS_END_TOKEN = 0x0000FFFF;	// D3DVS_END() / D3DPS_END()
	const std::uint32_t DECL_END_TOKEN = 0xFFFFFFFF;	// D3DVSD_END()
	const std::uint32_t OPCODE_MASK = 0x0000FFFF;
	const std::uint32_t OP_COMMENT = 0xFFFE;
	const std::uint32_t OP_RCP = 6, OP_RSQ = 7, OP_EXP = 14, OP_LOG = 15, OP_DCL = 31, OP_EXPP = 78, OP_LOGP = 79, OP_DEF = 81;
	const std::uint32_t PARAM_BIT = 0x80000000;	// set on every parameter token, clear on an instruction token
	const std::uint32_t SWIZZLE_MASK = 0x00FF0000, NO_SWIZZLE = 0x00E40000, REPLICATE_W = 0x00FF0000;
	const std::uint32_t REG_INPUT = 1, REG_CONST = 2;
	const int NUM_INPUT_REGISTERS = 17;

	// v0-v16 -> D3D9 usage and usage index (D3DVSDE_* order; DXVK's D3D8_VERTEX_INPUT_REGISTERS).
	enum { USAGE_POSITION = 0, USAGE_BLENDWEIGHT = 1, USAGE_BLENDINDICES = 2, USAGE_NORMAL = 3, USAGE_PSIZE = 4, USAGE_TEXCOORD = 5, USAGE_COLOR = 10 };
	const std::uint8_t INPUT_USAGE[NUM_INPUT_REGISTERS][2] = {
		{ USAGE_POSITION, 0 }, { USAGE_BLENDWEIGHT, 0 }, { USAGE_BLENDINDICES, 0 }, { USAGE_NORMAL, 0 }, { USAGE_PSIZE, 0 },
		{ USAGE_COLOR, 0 }, { USAGE_COLOR, 1 },
		{ USAGE_TEXCOORD, 0 }, { USAGE_TEXCOORD, 1 }, { USAGE_TEXCOORD, 2 }, { USAGE_TEXCOORD, 3 },
		{ USAGE_TEXCOORD, 4 }, { USAGE_TEXCOORD, 5 }, { USAGE_TEXCOORD, 6 }, { USAGE_TEXCOORD, 7 },
		{ USAGE_POSITION, 1 }, { USAGE_NORMAL, 1 },
	};

	std::uint32_t destRegister(std::uint32_t type, std::uint32_t reg)
	{
		return (reg & 0x7FF) | ((type & 0x07) << 28) | (((type & 0x18) >> 3) << 11) | (0xFu << 16) | PARAM_BIT;
	}

	bool fail(std::string *why, const char *text)
	{
		if (why != nullptr)
			*why = text;
		return false;
	}
}

bool gx_ps8to9(const std::uint32_t *function, std::vector<std::uint32_t> &out, std::string *why, std::size_t maxTokens)
{
	out.clear();
	if (function == nullptr)
		return fail(why, "no function");
	if (function[0] < 0xFFFF0100 || function[0] > 0xFFFF0104)
		return fail(why, "not a ps_1_x program");
	out.push_back(function[0] == 0xFFFF0100 ? 0xFFFF0101 : function[0]);
	// END counts only where an instruction could start: a comment's payload and a def's four literals are data and may
	// hold any bit pattern, 0x0000FFFF included
	for (std::size_t i = 1; i < maxTokens;)
	{
		const std::uint32_t token = function[i++];
		out.push_back(token);
		if (token == VS_END_TOKEN)
			return true;
		if ((token & PARAM_BIT) != 0)
			continue;
		std::size_t data = 0;
		if ((token & OPCODE_MASK) == OP_COMMENT)
			data = (token >> 16) & 0x7FFF;
		else if ((token & OPCODE_MASK) == OP_DEF)
			data = 5;	// dest + 4 raw floats
		if (i + data > maxTokens)
			break;
		out.insert(out.end(), function + i, function + i + data);
		i += data;
	}
	out.clear();
	return fail(why, "no end token");
}

bool gx_vs8to9(const std::uint32_t *declaration, const std::uint32_t *function, std::vector<std::uint32_t> &out, std::string *why,
	std::size_t maxTokens)
{
	out.clear();
	if (declaration == nullptr || function == nullptr)
		return fail(why, "no declaration or no function");
	if (function[0] != 0xFFFE0100 && function[0] != 0xFFFE0101)
		return fail(why, "not a vs_1_x program");

	// The declaration: which input registers the program reads, and the constants it carries.
	std::uint32_t inputs = 0;
	std::vector<std::uint32_t> defs;
	std::size_t i = 0;
	for (;;)
	{
		if (i >= maxTokens)
			return fail(why, "declaration has no end token");
		const std::uint32_t token = declaration[i++];
		if (token == DECL_END_TOKEN)
			break;
		switch (token >> 29)
		{
		case 2:	// D3DVSD_TOKEN_STREAMDATA: a vertex register (D3DVSD_REG), or D3DVSD_SKIP
			if ((token & 0x10000000) == 0)
			{
				const std::uint32_t reg = token & 0x1F;
				if (reg >= NUM_INPUT_REGISTERS)
					return fail(why, "declaration names an input register past v16");
				inputs |= 1u << reg;
			}
			break;
		case 4:	// D3DVSD_TOKEN_CONSTMEM: count x 4 floats loaded into c<address>.. before the program runs
		{
			const std::uint32_t count = (token >> 25) & 0xF, address = token & 0x7F;
			if (i + count * 4 > maxTokens)
				return fail(why, "declaration constants run past the end");
			for (std::uint32_t c = 0; c < count; ++c, i += 4)
			{
				defs.push_back(OP_DEF);
				defs.push_back(destRegister(REG_CONST, address + c));
				defs.insert(defs.end(), declaration + i, declaration + i + 4);
			}
			break;
		}
		case 5:	// D3DVSD_TOKEN_EXT: info + count DWORDs we do not use
			i += (token >> 24) & 0x1F;
			break;
		case 0: case 1: case 3:	// NOP, STREAM (no effect on the program), TESSELLATOR (not used by the game)
			break;
		default:
			return fail(why, "declaration has an unknown token type");
		}
	}

	out.push_back(0xFFFE0101);	// vs_1_0 -> vs_1_1, as upstream d3d8to9 does: D3D9 has no 1.0
	for (int vn = 0; vn < NUM_INPUT_REGISTERS; ++vn)
	{
		if ((inputs & (1u << vn)) == 0)
			continue;
		out.push_back(OP_DCL);
		out.push_back(INPUT_USAGE[vn][0] | (std::uint32_t(INPUT_USAGE[vn][1]) << 16) | PARAM_BIT);
		out.push_back(destRegister(REG_INPUT, std::uint32_t(vn)));
	}
	out.insert(out.end(), defs.begin(), defs.end());

	// The program. D3D8 read a scalar op's missing swizzle as .w; D3D9 wants the replicate swizzle written out.
	for (i = 1;;)
	{
		if (i >= maxTokens)
		{
			out.clear();
			return fail(why, "function has no end token");
		}
		const std::uint32_t token = function[i++];
		out.push_back(token);
		if (token == VS_END_TOKEN)
			return true;
		if ((token & PARAM_BIT) != 0)
			continue;
		const std::uint32_t opcode = token & OPCODE_MASK;
		if (opcode == OP_COMMENT)
		{
			const std::uint32_t length = (token >> 16) & 0x7FFF;
			if (i + length > maxTokens)
			{
				out.clear();
				return fail(why, "comment runs past the end");
			}
			out.insert(out.end(), function + i, function + i + length);
			i += length;
		}
		else if (opcode == OP_DEF)
		{
			if (i + 5 > maxTokens)
			{
				out.clear();
				return fail(why, "def runs past the end");
			}
			out.insert(out.end(), function + i, function + i + 5);	// dest + 4 raw floats (a positive float has no param bit)
			i += 5;
		}
		else if (opcode == OP_RSQ || opcode == OP_RCP || opcode == OP_EXP || opcode == OP_LOG || opcode == OP_EXPP || opcode == OP_LOGP)
		{
			if (i + 2 > maxTokens)
			{
				out.clear();
				return fail(why, "scalar op runs past the end");
			}
			out.push_back(function[i++]);	// dest
			std::uint32_t src = function[i++];
			if ((src & SWIZZLE_MASK) == NO_SWIZZLE)
				src = (src & ~SWIZZLE_MASK) | REPLICATE_W;
			out.push_back(src);
		}
	}
}
