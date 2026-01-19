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

#pragma once

#include "Lib/BaseType.h"
#include "Common/AsciiString.h"

// ============================================================================
// Registry Compatibility Wrapper
//
// This module provides compatibility functions that mimic the old Windows
// Registry API, but use the new INI-based ConfigurationManager backend.
// This allows legacy code and mods (e.g., gentools) to work unchanged.
//
// All functions support both "Section\Key" and "Section/Key" formats.
// ============================================================================

/**
 * Get a string value from configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param defaultValue Value to return if not found
 * @return Configuration value or defaultValue if not found
 */
extern AsciiString GetRegistryString(const AsciiString& keyPath, const AsciiString& defaultValue = "");

/**
 * Get an integer value from configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param defaultValue Value to return if not found
 * @return Configuration value or defaultValue if not found
 */
extern Int GetRegistryInt(const AsciiString& keyPath, Int defaultValue = 0);

/**
 * Get a boolean value from configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param defaultValue Value to return if not found
 * @return Configuration value or defaultValue if not found
 */
extern Bool GetRegistryBool(const AsciiString& keyPath, Bool defaultValue = FALSE);

/**
 * Get a floating-point value from configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param defaultValue Value to return if not found
 * @return Configuration value or defaultValue if not found
 */
extern Real GetRegistryReal(const AsciiString& keyPath, Real defaultValue = 0.0f);

/**
 * Set a string value in configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param value Value to set
 */
extern void SetRegistryString(const AsciiString& keyPath, const AsciiString& value);

/**
 * Set an integer value in configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param value Value to set
 */
extern void SetRegistryInt(const AsciiString& keyPath, Int value);

/**
 * Set a boolean value in configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param value Value to set
 */
extern void SetRegistryBool(const AsciiString& keyPath, Bool value);

/**
 * Set a floating-point value in configuration
 * @param keyPath Path in format "Section\Key" or "Section/Key"
 * @param value Value to set
 */
extern void SetRegistryReal(const AsciiString& keyPath, Real value);

/**
 * Flush all pending configuration changes to disk
 */
extern void FlushRegistry();

// ============================================================================
// Legacy Generals registry API compatibility
//
// The original game used the Windows Registry. GeneralsX uses the INI-backed
// ConfigurationManager, but many legacy call sites (and mods) still reference
// these older functions.
// ============================================================================

// Original-style helpers (kept for source compatibility)
Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val);
Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val);
Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val);

AsciiString GetRegistryLanguage(void);
AsciiString GetRegistryGameName(void);
UnsignedInt GetRegistryVersion(void);
UnsignedInt GetRegistryMapPackVersion(void);
