/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "PreRTS.h"

#include <cstdint>

#include "Common/PlayerTemplate.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Mouse.h"
#include "GameNetwork/LobbyTooltips.h"

// GeneralsX @refactor Codex 04/08/2026 Share lobby tooltips across local and online game modes.
void playerTemplateComboBoxTooltip(GameWindow *window, WinInstanceData *, UnsignedInt)
{
	Int index = 0;
	GadgetComboBoxGetSelectedPos(window, &index);
	Int templateNumber = static_cast<Int>(reinterpret_cast<intptr_t>(GadgetComboBoxGetItemData(window, index)));
	UnicodeString tooltip;
	if (templateNumber == -1)
	{
		tooltip = TheGameText->fetch("TOOLTIP:BioStrategyLong_Random");
	}
	else
	{
		const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNumber);
		if (playerTemplate)
		{
			tooltip = TheGameText->fetch(playerTemplate->getTooltip());
		}
	}
	TheMouse->setCursorTooltip(tooltip);
}

void playerTemplateListBoxTooltip(GameWindow *window, WinInstanceData *, UnsignedInt mouse)
{
	Int row;
	Int column;
	GadgetListBoxGetEntryBasedOnXY(window, LOLONGTOSHORT(mouse), HILONGTOSHORT(mouse), row, column);
	if (row == -1 || column == -1)
	{
		return;
	}

	Int templateNumber = static_cast<Int>(reinterpret_cast<intptr_t>(GadgetListBoxGetItemData(window, row, column)));
	UnicodeString tooltip;
	if (templateNumber == -1)
	{
		tooltip = TheGameText->fetch("TOOLTIP:BioStrategyLong_Random");
	}
	else
	{
		const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNumber);
		if (playerTemplate)
		{
			tooltip = TheGameText->fetch(playerTemplate->getTooltip());
		}
	}
	TheMouse->setCursorTooltip(tooltip, 0);
}

void gameAcceptTooltip(GameWindow *window, WinInstanceData *, UnsignedInt mouse)
{
	Int windowX;
	Int windowY;
	Int windowWidth;
	Int windowHeight;
	window->winGetScreenPosition(&windowX, &windowY);
	window->winGetSize(&windowWidth, &windowHeight);

	const Int mouseX = LOLONGTOSHORT(mouse);
	const Int mouseY = HILONGTOSHORT(mouse);
	if (mouseX > windowX && mouseX < windowX + windowWidth && mouseY > windowY && mouseY < windowY + windowHeight)
	{
		TheMouse->setCursorTooltip(TheGameText->fetch("TOOLTIP:GameAcceptance"), -1, nullptr);
	}
}
