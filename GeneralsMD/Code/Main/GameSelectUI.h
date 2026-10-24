/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

// GeneralsX @feature cheviiot 24/10/2026 PLAN-023: pre-init native game profile chooser (GUI launches).
#pragma once

#include "Common/GameProfile.h"

namespace GameSelectUI
{

// Blocking SDL chooser before memory pools / asset mount.
// Returns FALSE if the user closed the window (abort launch).
Bool run(GameProfileKind preselect, GameProfileKind *outChosen);

} // namespace GameSelectUI
