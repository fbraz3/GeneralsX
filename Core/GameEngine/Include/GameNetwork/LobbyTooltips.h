/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#pragma once

#include "Lib/BaseType.h"

class GameWindow;
struct WinInstanceData;

void playerTemplateComboBoxTooltip(GameWindow *window, WinInstanceData *instanceData, UnsignedInt mouse);
void playerTemplateListBoxTooltip(GameWindow *window, WinInstanceData *instanceData, UnsignedInt mouse);
void gameAcceptTooltip(GameWindow *window, WinInstanceData *instanceData, UnsignedInt mouse);
