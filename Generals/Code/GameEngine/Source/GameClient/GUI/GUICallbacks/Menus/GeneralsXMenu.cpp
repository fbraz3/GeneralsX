/*
**	Command & Conquer Generals(tm)
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

// FILE: GeneralsXMenu.cpp /////////////////////////////////////////////////////////////////////////
// GeneralsX @feature fbraz3 14/10/2026 GeneralsX configuration options menu
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/OptionPreferences.h"
#include "GameClient/GameClient.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GameText.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/Shell.h"
#include "GameClient/WindowLayout.h"

static NameKeyType parentGeneralsXMenuID = NAMEKEY_INVALID;
static GameWindow *parentGeneralsXMenu = nullptr;

static NameKeyType sliderDrawDistanceID = NAMEKEY_INVALID;
static NameKeyType sliderMaxCameraHeightID = NAMEKEY_INVALID;
static NameKeyType checkAdvancedShadowsID = NAMEKEY_INVALID;
static NameKeyType checkAdvancedLightingID = NAMEKEY_INVALID;
static NameKeyType checkEnablePostProcessingID = NAMEKEY_INVALID;

static NameKeyType buttonDefaultsID = NAMEKEY_INVALID;
static NameKeyType buttonAcceptID = NAMEKEY_INVALID;
static NameKeyType buttonCloseID = NAMEKEY_INVALID;

static GameWindow *sliderDrawDistance = nullptr;
static GameWindow *sliderMaxCameraHeight = nullptr;
static GameWindow *checkAdvancedShadows = nullptr;
static GameWindow *checkAdvancedLighting = nullptr;
static GameWindow *checkEnablePostProcessing = nullptr;

static GameWindow *buttonDefaults = nullptr;
static GameWindow *buttonAccept = nullptr;
static GameWindow *buttonClose = nullptr;

static OptionPreferences *pref = nullptr;

//-------------------------------------------------------------------------------------------------
static void setStaticTextSafe(const char *nameKeyStr, const char *labelKey)
{
	GameWindow *w = TheWindowManager->winGetWindowFromId(nullptr, TheNameKeyGenerator->nameToKey(nameKeyStr));
	if (w)
	{
		UnicodeString ustr = TheGameText->fetch(labelKey);
		GadgetStaticTextSetText(w, ustr);
	}
}

//-------------------------------------------------------------------------------------------------
static void setButtonTextSafe(const char *nameKeyStr, const char *labelKey)
{
	GameWindow *w = TheWindowManager->winGetWindowFromId(nullptr, TheNameKeyGenerator->nameToKey(nameKeyStr));
	if (w)
	{
		UnicodeString ustr = TheGameText->fetch(labelKey);
		GadgetButtonSetText(w, ustr);
	}
}

//-------------------------------------------------------------------------------------------------
static void setCheckBoxTextSafe(const char *nameKeyStr, const char *labelKey)
{
	GameWindow *w = TheWindowManager->winGetWindowFromId(nullptr, TheNameKeyGenerator->nameToKey(nameKeyStr));
	if (w)
	{
		UnicodeString ustr = TheGameText->fetch(labelKey);
		GadgetCheckBoxSetText(w, ustr);
	}
}

//-------------------------------------------------------------------------------------------------
static void saveOptions()
{
	if (!pref)
		return;

	if (sliderDrawDistance)
	{
		Int val = GadgetSliderGetPosition(sliderDrawDistance);
		if (val >= 100 && val <= 200)
		{
			TheWritableGlobalData->m_terrainDrawDistanceScale = val / 100.0f;
			pref->setTerrainDrawDistanceScale(val / 100.0f);
		}
	}

	if (sliderMaxCameraHeight)
	{
		Int val = GadgetSliderGetPosition(sliderMaxCameraHeight);
		if (val >= 300 && val <= 1000)
		{
			TheWritableGlobalData->m_maxCameraHeight = (Real)val;
			TheWritableGlobalData->m_enforceMaxCameraHeight = FALSE;
			pref->setMaxCameraHeight((Real)val);
		}
	}

	if (checkAdvancedShadows)
	{
		Bool val = GadgetCheckBoxIsChecked(checkAdvancedShadows);
		TheWritableGlobalData->m_advancedShadows = val;
		pref->setAdvancedShadows(val);
	}

	if (checkAdvancedLighting)
	{
		Bool val = GadgetCheckBoxIsChecked(checkAdvancedLighting);
		TheWritableGlobalData->m_advancedLighting = val;
		pref->setAdvancedLighting(val);
	}

	if (checkEnablePostProcessing)
	{
		Bool val = GadgetCheckBoxIsChecked(checkEnablePostProcessing);
		TheWritableGlobalData->m_enablePostProcessing = val;
		pref->setEnablePostProcessing(val);
	}

	pref->write();
}

//-------------------------------------------------------------------------------------------------
static void setDefaults()
{
	if (sliderDrawDistance)
		GadgetSliderSetPosition(sliderDrawDistance, 105);

	if (sliderMaxCameraHeight)
		GadgetSliderSetPosition(sliderMaxCameraHeight, 500);

	if (checkAdvancedShadows)
		GadgetCheckBoxSetChecked(checkAdvancedShadows, TRUE);

	if (checkAdvancedLighting)
		GadgetCheckBoxSetChecked(checkAdvancedLighting, TRUE);

	if (checkEnablePostProcessing)
		GadgetCheckBoxSetChecked(checkEnablePostProcessing, TRUE);
}

//-------------------------------------------------------------------------------------------------
void GeneralsXMenuInit(WindowLayout *layout, void *userData)
{
	parentGeneralsXMenuID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:GeneralsXMenuParent");
	parentGeneralsXMenu = TheWindowManager->winGetWindowFromId(nullptr, parentGeneralsXMenuID);

	sliderDrawDistanceID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:SliderDrawDistance");
	sliderMaxCameraHeightID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:SliderMaxCameraHeight");
	checkAdvancedShadowsID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:CheckAdvancedShadows");
	checkAdvancedLightingID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:CheckAdvancedLighting");
	checkEnablePostProcessingID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:CheckEnablePostProcessing");

	buttonDefaultsID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:ButtonDefaults");
	buttonAcceptID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:ButtonAccept");
	buttonCloseID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:ButtonClose");

	sliderDrawDistance = TheWindowManager->winGetWindowFromId(nullptr, sliderDrawDistanceID);
	sliderMaxCameraHeight = TheWindowManager->winGetWindowFromId(nullptr, sliderMaxCameraHeightID);
	checkAdvancedShadows = TheWindowManager->winGetWindowFromId(nullptr, checkAdvancedShadowsID);
	checkAdvancedLighting = TheWindowManager->winGetWindowFromId(nullptr, checkAdvancedLightingID);
	checkEnablePostProcessing = TheWindowManager->winGetWindowFromId(nullptr, checkEnablePostProcessingID);

	buttonDefaults = TheWindowManager->winGetWindowFromId(nullptr, buttonDefaultsID);
	buttonAccept = TheWindowManager->winGetWindowFromId(nullptr, buttonAcceptID);
	buttonClose = TheWindowManager->winGetWindowFromId(nullptr, buttonCloseID);

	if (!pref)
		pref = NEW OptionPreferences;

	// Populate controls from current engine / preferences values
	if (sliderDrawDistance)
	{
		Int drawDist = (Int)(TheGlobalData->m_terrainDrawDistanceScale * 100.0f + 0.5f);
		if (drawDist < 100) drawDist = 100;
		if (drawDist > 200) drawDist = 200;
		GadgetSliderSetPosition(sliderDrawDistance, drawDist);
	}

	if (sliderMaxCameraHeight)
	{
		Int maxCam = (Int)(TheGlobalData->m_maxCameraHeight + 0.5f);
		if (maxCam < 300) maxCam = 300;
		if (maxCam > 1000) maxCam = 1000;
		GadgetSliderSetPosition(sliderMaxCameraHeight, maxCam);
	}

	if (checkAdvancedShadows)
		GadgetCheckBoxSetChecked(checkAdvancedShadows, TheGlobalData->m_advancedShadows);

	if (checkAdvancedLighting)
		GadgetCheckBoxSetChecked(checkAdvancedLighting, TheGlobalData->m_advancedLighting);

	if (checkEnablePostProcessing)
		GadgetCheckBoxSetChecked(checkEnablePostProcessing, TheGlobalData->m_enablePostProcessing);

	// Localized texts
	setStaticTextSafe("GeneralsXMenu.wnd:LabelTitle", "GUI:GeneralsXTitle");
	setStaticTextSafe("GeneralsXMenu.wnd:LabelCameraTitle", "GUI:SagePatchOptions");
	setStaticTextSafe("GeneralsXMenu.wnd:LabelDrawDistance", "GUI:DrawDistance");
	setStaticTextSafe("GeneralsXMenu.wnd:LabelMaxCameraHeight", "GUI:MaxCameraHeight");
	setStaticTextSafe("GeneralsXMenu.wnd:LabelGraphicsTitle", "GUI:EnhancedGraphics");

	setCheckBoxTextSafe("GeneralsXMenu.wnd:CheckAdvancedShadows", "GUI:AdvancedShadows");
	setCheckBoxTextSafe("GeneralsXMenu.wnd:CheckAdvancedLighting", "GUI:AdvancedLighting");
	setCheckBoxTextSafe("GeneralsXMenu.wnd:CheckEnablePostProcessing", "GUI:EnablePostProcessing");

	setButtonTextSafe("GeneralsXMenu.wnd:ButtonDefaults", "GUI:Defaults");
	setButtonTextSafe("GeneralsXMenu.wnd:ButtonAccept", "GUI:Accept");
	setButtonTextSafe("GeneralsXMenu.wnd:ButtonClose", "GUI:Cancel");

	layout->hide(FALSE);

	if (parentGeneralsXMenu)
		TheWindowManager->winSetFocus(parentGeneralsXMenu);
}

//-------------------------------------------------------------------------------------------------
void GeneralsXMenuUpdate(WindowLayout *layout, void *userData)
{
}

//-------------------------------------------------------------------------------------------------
void GeneralsXMenuShutdown(WindowLayout *layout, void *userData)
{
	layout->hide(TRUE);
	TheShell->shutdownComplete(layout);

	sliderDrawDistance = nullptr;
	sliderMaxCameraHeight = nullptr;
	checkAdvancedShadows = nullptr;
	checkAdvancedLighting = nullptr;
	checkEnablePostProcessing = nullptr;

	buttonDefaults = nullptr;
	buttonAccept = nullptr;
	buttonClose = nullptr;
	parentGeneralsXMenu = nullptr;

	if (pref)
	{
		delete pref;
		pref = nullptr;
	}
}

//-------------------------------------------------------------------------------------------------
WindowMsgHandledType GeneralsXMenuSystem(GameWindow *window, UnsignedInt msg,
																				WindowMsgData mData1, WindowMsgData mData2)
{
	switch (msg) {
		case GWM_CREATE:
			buttonDefaultsID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:ButtonDefaults");
			buttonAcceptID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:ButtonAccept");
			buttonCloseID = TheNameKeyGenerator->nameToKey("GeneralsXMenu.wnd:ButtonClose");
			break;

		case GWM_DESTROY:
			break;

		case GWM_INPUT_FOCUS:
			if (mData1 == TRUE)
				*(Bool *)mData2 = TRUE;
			return MSG_HANDLED;

		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if (controlID == buttonCloseID)
			{
				TheShell->pop();
				return MSG_HANDLED;
			}
			else if (controlID == buttonAcceptID)
			{
				saveOptions();
				TheShell->pop();
				return MSG_HANDLED;
			}
			else if (controlID == buttonDefaultsID)
			{
				setDefaults();
				return MSG_HANDLED;
			}
			break;
		}

		default:
			break;
	}

	return MSG_IGNORED;
}

//-------------------------------------------------------------------------------------------------
WindowMsgHandledType GeneralsXMenuInput(GameWindow *window, UnsignedInt msg,
																			WindowMsgData mData1, WindowMsgData mData2)
{
	switch (msg) {
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;

			if (key == KEY_ESC && BitIsSet(state, KEY_STATE_UP)) {
				TheShell->pop();
				return MSG_HANDLED;
			}
			break;
		}

		default:
			break;
	}

	return MSG_IGNORED;
}
