// GeneralsX: x64 handles for d3d8to9 (patch 0002). A D3D8 shader handle or state-block token is a 32-bit DWORD; upstream
// stored the D3D9 interface pointer in it, which only works in a 32-bit process. Here a handle is a slot index + 1, so
// 0 is never a valid handle and a freed slot is reused. No Windows headers: scripts/test/d3d8to9-handle-test.cpp.
#pragma once

#include <cstdint>
#include <vector>

template <typename T> struct GxHandleTable
{
	std::vector<T *> Slots;

	std::uint32_t Add(T *p)
	{
		for (size_t i = 0; i < Slots.size(); ++i)
			if (Slots[i] == nullptr) { Slots[i] = p; return static_cast<std::uint32_t>(i + 1); }
		Slots.push_back(p);
		return static_cast<std::uint32_t>(Slots.size());
	}
	T *Get(std::uint32_t h) const { return h != 0 && h <= Slots.size() ? Slots[h - 1] : nullptr; }
	void Remove(std::uint32_t h) { if (Get(h) != nullptr) Slots[h - 1] = nullptr; }
};
