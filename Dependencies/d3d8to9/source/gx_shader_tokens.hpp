// GeneralsX: D3D8 -> D3D9 shader bytecode without D3DX (patch 0003). Upstream d3d8to9 disassembles every shader
// with d3dx9_43.dll and reassembles it, so on a PC without the 2010 DirectX runtime no shader could be created at all.
// Like DXVK's own D3D8 front end, a pixel shader passes through (D3D9 reads ps_1_x tokens as D3D8 wrote them) and a
// vertex shader gets the dcl_ inputs its D3D8 declaration implied. Plain token work, no Windows headers:
// scripts/test/d3d8to9-shader-test.sh runs both over the game's whole shader catalogue.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

// ps_1_0 becomes ps_1_1 (D3D9 has no 1.0); everything else is copied through D3DPS_END. false + why: not a ps_1_x
// program, or no end token within maxTokens.
bool gx_ps8to9(const std::uint32_t *function, std::vector<std::uint32_t> &out, std::string *why, std::size_t maxTokens = 65536);

// A D3D8 vs_1_x function + its D3DVSD declaration -> a D3D9 vs_1_1 function: version, one dcl per declared input
// register, one def per declared constant, then the program. false + why on a malformed declaration or function.
bool gx_vs8to9(const std::uint32_t *declaration, const std::uint32_t *function, std::vector<std::uint32_t> &out, std::string *why,
	std::size_t maxTokens = 65536);
