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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: FunctionLexicon.cpp //////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Collection of function pointers to help us in managing
//						 and assign callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FunctionLexicon.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/Gadget.h"

// Popup Ladder Select --------------------------------------------------------------------------
extern void PopupLadderSelectInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupLadderSelectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupLadderSelectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

extern WindowMsgHandledType PopupBuddyNotificationSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Buddy Overlay Right Click menu callbacks --------------------------------------------------------------
extern void RCGameDetailsMenuInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType RCGameDetailsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Beacon control bar callback --------------------------------------------------------------
extern WindowMsgHandledType BeaconWindowInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup Replay Save Menu ----------------------------------------------------------------------------------
extern void PopupReplayInit( WindowLayout *layout, void *userData );
extern void PopupReplayUpdate( WindowLayout *layout, void *userData );
extern void PopupReplayShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupReplaySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupReplayInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Extended MessageBox ----------------------------------------------------------------------------------
extern WindowMsgHandledType ExtendedMessageBoxSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// game window draw table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinDrawTable[] =
{
	{ NAMEKEY_INVALID, "IMECandidateMainDraw",						reinterpret_cast<void*>(IMECandidateMainDraw) },
	{ NAMEKEY_INVALID, "IMECandidateTextAreaDraw",				reinterpret_cast<void*>(IMECandidateTextAreaDraw) },
	{ NAMEKEY_INVALID, NULL,																NULL }
};

// game window system table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinSystemTable[] =
{


	{ NAMEKEY_INVALID, "PassSelectedButtonsToParentSystem",	reinterpret_cast<void*>(PassSelectedButtonsToParentSystem) },
	{ NAMEKEY_INVALID, "PassMessagesToParentSystem",				reinterpret_cast<void*>(PassMessagesToParentSystem) },

	{ NAMEKEY_INVALID, "GameWinDefaultSystem",							reinterpret_cast<void*>(GameWinDefaultSystem) },
	{ NAMEKEY_INVALID, "GadgetPushButtonSystem",						reinterpret_cast<void*>(GadgetPushButtonSystem) },
	{ NAMEKEY_INVALID, "GadgetCheckBoxSystem",							reinterpret_cast<void*>(GadgetCheckBoxSystem) },
	{ NAMEKEY_INVALID, "GadgetRadioButtonSystem",						reinterpret_cast<void*>(GadgetRadioButtonSystem) },
	{ NAMEKEY_INVALID, "GadgetTabControlSystem",						reinterpret_cast<void*>(GadgetTabControlSystem) },
	{ NAMEKEY_INVALID, "GadgetListBoxSystem",								reinterpret_cast<void*>(GadgetListBoxSystem) },
	{ NAMEKEY_INVALID, "GadgetComboBoxSystem",							reinterpret_cast<void*>(GadgetComboBoxSystem) },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderSystem",			reinterpret_cast<void*>(GadgetHorizontalSliderSystem) },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderSystem",				reinterpret_cast<void*>(GadgetVerticalSliderSystem) },
	{ NAMEKEY_INVALID, "GadgetProgressBarSystem",						reinterpret_cast<void*>(GadgetProgressBarSystem) },
	{ NAMEKEY_INVALID, "GadgetStaticTextSystem",						reinterpret_cast<void*>(GadgetStaticTextSystem) },
	{ NAMEKEY_INVALID, "GadgetTextEntrySystem",							reinterpret_cast<void*>(GadgetTextEntrySystem) },
	{ NAMEKEY_INVALID, "MessageBoxSystem",									reinterpret_cast<void*>(MessageBoxSystem) },
	{ NAMEKEY_INVALID, "QuitMessageBoxSystem",							reinterpret_cast<void*>(QuitMessageBoxSystem) },

	{ NAMEKEY_INVALID, "ExtendedMessageBoxSystem",					reinterpret_cast<void*>(ExtendedMessageBoxSystem) },

	{ NAMEKEY_INVALID, "MOTDSystem",										reinterpret_cast<void*>(MOTDSystem) },
	{ NAMEKEY_INVALID, "MainMenuSystem",								reinterpret_cast<void*>(MainMenuSystem) },
	{ NAMEKEY_INVALID, "OptionsMenuSystem",							reinterpret_cast<void*>(OptionsMenuSystem) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuSystem",				reinterpret_cast<void*>(SinglePlayerMenuSystem) },
	{ NAMEKEY_INVALID, "QuitMenuSystem",								reinterpret_cast<void*>(QuitMenuSystem) },
	{ NAMEKEY_INVALID, "MapSelectMenuSystem",						reinterpret_cast<void*>(MapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "ReplayMenuSystem",							reinterpret_cast<void*>(ReplayMenuSystem) },
	{ NAMEKEY_INVALID, "CreditsMenuSystem",							reinterpret_cast<void*>(CreditsMenuSystem) },
	{ NAMEKEY_INVALID, "LanLobbyMenuSystem",						reinterpret_cast<void*>(LanLobbyMenuSystem) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuSystem",			reinterpret_cast<void*>(LanGameOptionsMenuSystem) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuSystem",				reinterpret_cast<void*>(LanMapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuSystem", reinterpret_cast<void*>(SkirmishGameOptionsMenuSystem) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuSystem",   reinterpret_cast<void*>(SkirmishMapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "ChallengeMenuSystem",			reinterpret_cast<void*>(ChallengeMenuSystem) },
	{ NAMEKEY_INVALID, "SaveLoadMenuSystem",            reinterpret_cast<void*>(SaveLoadMenuSystem) },
	{ NAMEKEY_INVALID, "PopupCommunicatorSystem",       reinterpret_cast<void*>(PopupCommunicatorSystem) },
	{ NAMEKEY_INVALID, "PopupBuddyNotificationSystem",  reinterpret_cast<void*>(PopupBuddyNotificationSystem) },
	{ NAMEKEY_INVALID, "PopupReplaySystem",							reinterpret_cast<void*>(PopupReplaySystem) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuSystem",     reinterpret_cast<void*>(KeyboardOptionsMenuSystem) },
	{ NAMEKEY_INVALID, "WOLLadderScreenSystem",			    reinterpret_cast<void*>(WOLLadderScreenSystem) },
	{ NAMEKEY_INVALID, "WOLLoginMenuSystem",						reinterpret_cast<void*>(WOLLoginMenuSystem) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectSystem",					reinterpret_cast<void*>(WOLLocaleSelectSystem) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuSystem",						reinterpret_cast<void*>(WOLLobbyMenuSystem) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuSystem",				reinterpret_cast<void*>(WOLGameSetupMenuSystem) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuSystem",				reinterpret_cast<void*>(WOLMapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlaySystem",					reinterpret_cast<void*>(WOLBuddyOverlaySystem) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuSystem",		reinterpret_cast<void*>(WOLBuddyOverlayRCMenuSystem) },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuSystem",				reinterpret_cast<void*>(RCGameDetailsMenuSystem) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlaySystem",reinterpret_cast<void*>(GameSpyPlayerInfoOverlaySystem) },
	{ NAMEKEY_INVALID, "WOLMessageWindowSystem",				reinterpret_cast<void*>(WOLMessageWindowSystem) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuSystem",				reinterpret_cast<void*>(WOLQuickMatchMenuSystem) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuSystem",					reinterpret_cast<void*>(WOLWelcomeMenuSystem) },
	{ NAMEKEY_INVALID, "WOLStatusMenuSystem",						reinterpret_cast<void*>(WOLStatusMenuSystem) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenSystem",				reinterpret_cast<void*>(WOLQMScoreScreenSystem) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenSystem",		reinterpret_cast<void*>(WOLCustomScoreScreenSystem) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectSystem",		reinterpret_cast<void*>(NetworkDirectConnectSystem) },
	{ NAMEKEY_INVALID, "PopupHostGameSystem",						reinterpret_cast<void*>(PopupHostGameSystem) },
	{ NAMEKEY_INVALID, "PopupJoinGameSystem",						reinterpret_cast<void*>(PopupJoinGameSystem) },
	{ NAMEKEY_INVALID, "PopupLadderSelectSystem",				reinterpret_cast<void*>(PopupLadderSelectSystem) },
	{ NAMEKEY_INVALID, "InGamePopupMessageSystem",			reinterpret_cast<void*>(InGamePopupMessageSystem) },
	{ NAMEKEY_INVALID, "ControlBarSystem",							reinterpret_cast<void*>(ControlBarSystem) },
	{ NAMEKEY_INVALID, "ControlBarObserverSystem",			reinterpret_cast<void*>(ControlBarObserverSystem) },
	{ NAMEKEY_INVALID, "IMECandidateWindowSystem",			reinterpret_cast<void*>(IMECandidateWindowSystem) },
	{ NAMEKEY_INVALID, "ReplayControlSystem",						reinterpret_cast<void*>(ReplayControlSystem) },
	{ NAMEKEY_INVALID, "InGameChatSystem",							reinterpret_cast<void*>(InGameChatSystem) },
	{ NAMEKEY_INVALID, "DisconnectControlSystem",				reinterpret_cast<void*>(DisconnectControlSystem) },
	{ NAMEKEY_INVALID, "DiplomacySystem",								reinterpret_cast<void*>(DiplomacySystem) },
	{ NAMEKEY_INVALID, "GeneralsExpPointsSystem",				reinterpret_cast<void*>(GeneralsExpPointsSystem) },
	{ NAMEKEY_INVALID, "DifficultySelectSystem",				reinterpret_cast<void*>(DifficultySelectSystem) },

	{ NAMEKEY_INVALID, "IdleWorkerSystem",							reinterpret_cast<void*>(IdleWorkerSystem) },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlSystem", reinterpret_cast<void*>(EstablishConnectionsControlSystem) },
	{ NAMEKEY_INVALID, "GameInfoWindowSystem",					reinterpret_cast<void*>(GameInfoWindowSystem) },
	{ NAMEKEY_INVALID, "ScoreScreenSystem",							reinterpret_cast<void*>(ScoreScreenSystem) },
	{ NAMEKEY_INVALID, "DownloadMenuSystem",            reinterpret_cast<void*>(DownloadMenuSystem) },

	{ NAMEKEY_INVALID, NULL,																NULL }

};

// game window input table ------------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinInputTable[] =
{

	{ NAMEKEY_INVALID, "GameWinDefaultInput",						reinterpret_cast<void*>(GameWinDefaultInput) },
	{ NAMEKEY_INVALID, "GameWinBlockInput",							reinterpret_cast<void*>(GameWinBlockInput) },
	{ NAMEKEY_INVALID, "GadgetPushButtonInput",					reinterpret_cast<void*>(GadgetPushButtonInput) },
	{ NAMEKEY_INVALID, "GadgetCheckBoxInput",						reinterpret_cast<void*>(GadgetCheckBoxInput) },
	{ NAMEKEY_INVALID, "GadgetRadioButtonInput",				reinterpret_cast<void*>(GadgetRadioButtonInput) },
	{ NAMEKEY_INVALID, "GadgetTabControlInput",					reinterpret_cast<void*>(GadgetTabControlInput) },
	{ NAMEKEY_INVALID, "GadgetListBoxInput",						reinterpret_cast<void*>(GadgetListBoxInput) },
	{ NAMEKEY_INVALID, "GadgetListBoxMultiInput",				reinterpret_cast<void*>(GadgetListBoxMultiInput) },
	{ NAMEKEY_INVALID, "GadgetComboBoxInput",						reinterpret_cast<void*>(GadgetComboBoxInput) },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderInput",		reinterpret_cast<void*>(GadgetHorizontalSliderInput) },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderInput",			reinterpret_cast<void*>(GadgetVerticalSliderInput) },
	{ NAMEKEY_INVALID, "GadgetStaticTextInput",					reinterpret_cast<void*>(GadgetStaticTextInput) },
	{ NAMEKEY_INVALID, "GadgetTextEntryInput",					reinterpret_cast<void*>(GadgetTextEntryInput) },

	{ NAMEKEY_INVALID, "MainMenuInput",									reinterpret_cast<void*>(MainMenuInput) },
	{ NAMEKEY_INVALID, "MapSelectMenuInput",						reinterpret_cast<void*>(MapSelectMenuInput) },
	{ NAMEKEY_INVALID, "OptionsMenuInput",							reinterpret_cast<void*>(OptionsMenuInput) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInput",					reinterpret_cast<void*>(SinglePlayerMenuInput) },
	{ NAMEKEY_INVALID, "LanLobbyMenuInput",							reinterpret_cast<void*>(LanLobbyMenuInput) },
	{ NAMEKEY_INVALID, "ReplayMenuInput",								reinterpret_cast<void*>(ReplayMenuInput) },
	{ NAMEKEY_INVALID, "CreditsMenuInput",								reinterpret_cast<void*>(CreditsMenuInput) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInput",      reinterpret_cast<void*>(KeyboardOptionsMenuInput) },
	{ NAMEKEY_INVALID, "PopupCommunicatorInput",        reinterpret_cast<void*>(PopupCommunicatorInput) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInput",				reinterpret_cast<void*>(LanGameOptionsMenuInput) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInput",					reinterpret_cast<void*>(LanMapSelectMenuInput) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInput",  reinterpret_cast<void*>(SkirmishGameOptionsMenuInput) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInput",    reinterpret_cast<void*>(SkirmishMapSelectMenuInput) },
	{ NAMEKEY_INVALID, "ChallengeMenuInput",			reinterpret_cast<void*>(ChallengeMenuInput) },
	{ NAMEKEY_INVALID, "WOLLadderScreenInput",					reinterpret_cast<void*>(WOLLadderScreenInput) },
	{ NAMEKEY_INVALID, "WOLLoginMenuInput",							reinterpret_cast<void*>(WOLLoginMenuInput) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInput",					reinterpret_cast<void*>(WOLLocaleSelectInput) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInput",							reinterpret_cast<void*>(WOLLobbyMenuInput) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInput",					reinterpret_cast<void*>(WOLGameSetupMenuInput) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInput",					reinterpret_cast<void*>(WOLMapSelectMenuInput) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInput",					reinterpret_cast<void*>(WOLBuddyOverlayInput) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInput",	reinterpret_cast<void*>(GameSpyPlayerInfoOverlayInput) },
	{ NAMEKEY_INVALID, "WOLMessageWindowInput",					reinterpret_cast<void*>(WOLMessageWindowInput) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInput",				reinterpret_cast<void*>(WOLQuickMatchMenuInput) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInput",						reinterpret_cast<void*>(WOLWelcomeMenuInput) },
	{ NAMEKEY_INVALID, "WOLStatusMenuInput",						reinterpret_cast<void*>(WOLStatusMenuInput) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInput",					reinterpret_cast<void*>(WOLQMScoreScreenInput) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInput",			reinterpret_cast<void*>(WOLCustomScoreScreenInput) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInput",			reinterpret_cast<void*>(NetworkDirectConnectInput) },
	{ NAMEKEY_INVALID, "PopupHostGameInput",						reinterpret_cast<void*>(PopupHostGameInput) },
	{ NAMEKEY_INVALID, "PopupJoinGameInput",						reinterpret_cast<void*>(PopupJoinGameInput) },
	{ NAMEKEY_INVALID, "PopupLadderSelectInput",				reinterpret_cast<void*>(PopupLadderSelectInput) },
	{ NAMEKEY_INVALID, "InGamePopupMessageInput",				reinterpret_cast<void*>(InGamePopupMessageInput) },
	{ NAMEKEY_INVALID, "ControlBarInput",								reinterpret_cast<void*>(ControlBarInput) },
	{ NAMEKEY_INVALID, "ReplayControlInput",						reinterpret_cast<void*>(ReplayControlInput) },
	{ NAMEKEY_INVALID, "InGameChatInput",								reinterpret_cast<void*>(InGameChatInput) },
	{ NAMEKEY_INVALID, "DisconnectControlInput",				reinterpret_cast<void*>(DisconnectControlInput) },
	{ NAMEKEY_INVALID, "DiplomacyInput",								reinterpret_cast<void*>(DiplomacyInput) },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlInput", reinterpret_cast<void*>(EstablishConnectionsControlInput) },
	{ NAMEKEY_INVALID, "LeftHUDInput",									reinterpret_cast<void*>(LeftHUDInput) },
	{ NAMEKEY_INVALID, "ScoreScreenInput",							reinterpret_cast<void*>(ScoreScreenInput) },
	{ NAMEKEY_INVALID, "SaveLoadMenuInput",							reinterpret_cast<void*>(SaveLoadMenuInput) },
	{ NAMEKEY_INVALID, "BeaconWindowInput",							reinterpret_cast<void*>(BeaconWindowInput) },
	{ NAMEKEY_INVALID, "DifficultySelectInput",					reinterpret_cast<void*>(DifficultySelectInput) },
	{ NAMEKEY_INVALID, "PopupReplayInput",							reinterpret_cast<void*>(PopupReplayInput) },
	{ NAMEKEY_INVALID, "GeneralsExpPointsInput",				reinterpret_cast<void*>(GeneralsExpPointsInput)},

	{ NAMEKEY_INVALID, "DownloadMenuInput",							reinterpret_cast<void*>(DownloadMenuInput) },

	{ NAMEKEY_INVALID, "IMECandidateWindowInput",				reinterpret_cast<void*>(IMECandidateWindowInput) },
	{ NAMEKEY_INVALID, NULL,														NULL }

};

// game window tooltip table ----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinTooltipTable[] =
{


	{ NAMEKEY_INVALID, "GameWinDefaultTooltip",		reinterpret_cast<void*>(GameWinDefaultTooltip) },

	{ NAMEKEY_INVALID, NULL,											NULL }

};

// window layout init table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutInitTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuInit",									reinterpret_cast<void*>(MainMenuInit) },
	{ NAMEKEY_INVALID, "OptionsMenuInit",								reinterpret_cast<void*>(OptionsMenuInit) },
	{ NAMEKEY_INVALID, "SaveLoadMenuInit",              reinterpret_cast<void*>(SaveLoadMenuInit) },
	{ NAMEKEY_INVALID, "SaveLoadMenuFullScreenInit",    reinterpret_cast<void*>(SaveLoadMenuFullScreenInit) },

	{ NAMEKEY_INVALID, "PopupCommunicatorInit",         reinterpret_cast<void*>(PopupCommunicatorInit) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInit",       reinterpret_cast<void*>(KeyboardOptionsMenuInit) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInit",					reinterpret_cast<void*>(SinglePlayerMenuInit) },
	{ NAMEKEY_INVALID, "MapSelectMenuInit",							reinterpret_cast<void*>(MapSelectMenuInit) },
	{ NAMEKEY_INVALID, "LanLobbyMenuInit",							reinterpret_cast<void*>(LanLobbyMenuInit) },
	{ NAMEKEY_INVALID, "ReplayMenuInit",								reinterpret_cast<void*>(ReplayMenuInit) },
	{ NAMEKEY_INVALID, "CreditsMenuInit",								reinterpret_cast<void*>(CreditsMenuInit) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInit",				reinterpret_cast<void*>(LanGameOptionsMenuInit) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInit",					reinterpret_cast<void*>(LanMapSelectMenuInit) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInit",   reinterpret_cast<void*>(SkirmishGameOptionsMenuInit) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInit",     reinterpret_cast<void*>(SkirmishMapSelectMenuInit) },
	{ NAMEKEY_INVALID, "ChallengeMenuInit",				reinterpret_cast<void*>(ChallengeMenuInit) },
	{ NAMEKEY_INVALID, "WOLLadderScreenInit",						reinterpret_cast<void*>(WOLLadderScreenInit) },
	{ NAMEKEY_INVALID, "WOLLoginMenuInit",							reinterpret_cast<void*>(WOLLoginMenuInit) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInit",						reinterpret_cast<void*>(WOLLocaleSelectInit) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInit",							reinterpret_cast<void*>(WOLLobbyMenuInit) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInit",					reinterpret_cast<void*>(WOLGameSetupMenuInit) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInit",					reinterpret_cast<void*>(WOLMapSelectMenuInit) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInit",						reinterpret_cast<void*>(WOLBuddyOverlayInit) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuInit",			reinterpret_cast<void*>(WOLBuddyOverlayRCMenuInit) },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuInit",					reinterpret_cast<void*>(RCGameDetailsMenuInit) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInit",	reinterpret_cast<void*>(GameSpyPlayerInfoOverlayInit) },
	{ NAMEKEY_INVALID, "WOLMessageWindowInit",					reinterpret_cast<void*>(WOLMessageWindowInit) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInit",					reinterpret_cast<void*>(WOLQuickMatchMenuInit) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInit",						reinterpret_cast<void*>(WOLWelcomeMenuInit) },
	{ NAMEKEY_INVALID, "WOLStatusMenuInit",							reinterpret_cast<void*>(WOLStatusMenuInit) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInit",					reinterpret_cast<void*>(WOLQMScoreScreenInit) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInit",			reinterpret_cast<void*>(WOLCustomScoreScreenInit) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInit",			reinterpret_cast<void*>(NetworkDirectConnectInit) },
	{ NAMEKEY_INVALID, "PopupHostGameInit",							reinterpret_cast<void*>(PopupHostGameInit) },
	{ NAMEKEY_INVALID, "PopupJoinGameInit",							reinterpret_cast<void*>(PopupJoinGameInit) },
	{ NAMEKEY_INVALID, "PopupLadderSelectInit",					reinterpret_cast<void*>(PopupLadderSelectInit) },
	{ NAMEKEY_INVALID, "InGamePopupMessageInit",				reinterpret_cast<void*>(InGamePopupMessageInit) },
	{ NAMEKEY_INVALID, "GameInfoWindowInit",						reinterpret_cast<void*>(GameInfoWindowInit) },
	{ NAMEKEY_INVALID, "ScoreScreenInit",								reinterpret_cast<void*>(ScoreScreenInit) },
	{ NAMEKEY_INVALID, "DownloadMenuInit",              reinterpret_cast<void*>(DownloadMenuInit) },
	{ NAMEKEY_INVALID, "DifficultySelectInit",          reinterpret_cast<void*>(DifficultySelectInit) },
	{ NAMEKEY_INVALID, "PopupReplayInit",							  reinterpret_cast<void*>(PopupReplayInit) },

	{ NAMEKEY_INVALID, NULL,														NULL }

};

// window layout update table ---------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutUpdateTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuUpdate",								reinterpret_cast<void*>(MainMenuUpdate) },
	{ NAMEKEY_INVALID, "OptionsMenuUpdate",							reinterpret_cast<void*>(OptionsMenuUpdate) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuUpdate",				reinterpret_cast<void*>(SinglePlayerMenuUpdate) },
	{ NAMEKEY_INVALID, "MapSelectMenuUpdate",						reinterpret_cast<void*>(MapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "LanLobbyMenuUpdate",						reinterpret_cast<void*>(LanLobbyMenuUpdate) },
	{ NAMEKEY_INVALID, "ReplayMenuUpdate",							reinterpret_cast<void*>(ReplayMenuUpdate) },
	{ NAMEKEY_INVALID, "SaveLoadMenuUpdate",							reinterpret_cast<void*>(SaveLoadMenuUpdate) },

	{ NAMEKEY_INVALID, "CreditsMenuUpdate",							reinterpret_cast<void*>(CreditsMenuUpdate) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuUpdate",			reinterpret_cast<void*>(LanGameOptionsMenuUpdate) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuUpdate",				reinterpret_cast<void*>(LanMapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuUpdate", reinterpret_cast<void*>(SkirmishGameOptionsMenuUpdate) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuUpdate",   reinterpret_cast<void*>(SkirmishMapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "ChallengeMenuUpdate",			reinterpret_cast<void*>(ChallengeMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLLadderScreenUpdate",					reinterpret_cast<void*>(WOLLadderScreenUpdate) },
	{ NAMEKEY_INVALID, "WOLLoginMenuUpdate",						reinterpret_cast<void*>(WOLLoginMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectUpdate",					reinterpret_cast<void*>(WOLLocaleSelectUpdate) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuUpdate",						reinterpret_cast<void*>(WOLLobbyMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuUpdate",				reinterpret_cast<void*>(WOLGameSetupMenuUpdate) },
	{ NAMEKEY_INVALID, "PopupHostGameUpdate",						reinterpret_cast<void*>(PopupHostGameUpdate) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuUpdate",				reinterpret_cast<void*>(WOLMapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayUpdate",					reinterpret_cast<void*>(WOLBuddyOverlayUpdate) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayUpdate",reinterpret_cast<void*>(GameSpyPlayerInfoOverlayUpdate) },
	{ NAMEKEY_INVALID, "WOLMessageWindowUpdate",				reinterpret_cast<void*>(WOLMessageWindowUpdate) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuUpdate",				reinterpret_cast<void*>(WOLQuickMatchMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuUpdate",					reinterpret_cast<void*>(WOLWelcomeMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLStatusMenuUpdate",						reinterpret_cast<void*>(WOLStatusMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenUpdate",				reinterpret_cast<void*>(WOLQMScoreScreenUpdate) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenUpdate",		reinterpret_cast<void*>(WOLCustomScoreScreenUpdate) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectUpdate",		reinterpret_cast<void*>(NetworkDirectConnectUpdate) },
	{ NAMEKEY_INVALID, "ScoreScreenUpdate",							reinterpret_cast<void*>(ScoreScreenUpdate) },
	{ NAMEKEY_INVALID, "DownloadMenuUpdate",						reinterpret_cast<void*>(DownloadMenuUpdate) },
	{ NAMEKEY_INVALID, "PopupReplayUpdate",							reinterpret_cast<void*>(PopupReplayUpdate) },
	{ NAMEKEY_INVALID, NULL,														NULL }

};

// window layout shutdown table -------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutShutdownTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuShutdown",							reinterpret_cast<void*>(MainMenuShutdown) },
	{ NAMEKEY_INVALID, "OptionsMenuShutdown",						reinterpret_cast<void*>(OptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "SaveLoadMenuShutdown",          reinterpret_cast<void*>(SaveLoadMenuShutdown) },
	{ NAMEKEY_INVALID, "PopupCommunicatorShutdown",     reinterpret_cast<void*>(PopupCommunicatorShutdown) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuShutdown",   reinterpret_cast<void*>(KeyboardOptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuShutdown",			reinterpret_cast<void*>(SinglePlayerMenuShutdown) },
	{ NAMEKEY_INVALID, "MapSelectMenuShutdown",					reinterpret_cast<void*>(MapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "LanLobbyMenuShutdown",					reinterpret_cast<void*>(LanLobbyMenuShutdown) },
	{ NAMEKEY_INVALID, "ReplayMenuShutdown",						reinterpret_cast<void*>(ReplayMenuShutdown) },
	{ NAMEKEY_INVALID, "CreditsMenuShutdown",						reinterpret_cast<void*>(CreditsMenuShutdown) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuShutdown",		reinterpret_cast<void*>(LanGameOptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuShutdown",			reinterpret_cast<void*>(LanMapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuShutdown",reinterpret_cast<void*>(SkirmishGameOptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuShutdown", reinterpret_cast<void*>(SkirmishMapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "ChallengeMenuShutdown",				reinterpret_cast<void*>(ChallengeMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLLadderScreenShutdown",				reinterpret_cast<void*>(WOLLadderScreenShutdown) },
	{ NAMEKEY_INVALID, "WOLLoginMenuShutdown",					reinterpret_cast<void*>(WOLLoginMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectShutdown",				reinterpret_cast<void*>(WOLLocaleSelectShutdown) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuShutdown",					reinterpret_cast<void*>(WOLLobbyMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuShutdown",			reinterpret_cast<void*>(WOLGameSetupMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuShutdown",			reinterpret_cast<void*>(WOLMapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayShutdown",				reinterpret_cast<void*>(WOLBuddyOverlayShutdown) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayShutdown",reinterpret_cast<void*>(GameSpyPlayerInfoOverlayShutdown) },
	{ NAMEKEY_INVALID, "WOLMessageWindowShutdown",			reinterpret_cast<void*>(WOLMessageWindowShutdown) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuShutdown",			reinterpret_cast<void*>(WOLQuickMatchMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuShutdown",				reinterpret_cast<void*>(WOLWelcomeMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLStatusMenuShutdown",					reinterpret_cast<void*>(WOLStatusMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenShutdown",			reinterpret_cast<void*>(WOLQMScoreScreenShutdown) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenShutdown",	reinterpret_cast<void*>(WOLCustomScoreScreenShutdown) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectShutdown",	reinterpret_cast<void*>(NetworkDirectConnectShutdown) },
	{ NAMEKEY_INVALID, "ScoreScreenShutdown",						reinterpret_cast<void*>(ScoreScreenShutdown) },
	{ NAMEKEY_INVALID, "DownloadMenuShutdown",          reinterpret_cast<void*>(DownloadMenuShutdown) },
	{ NAMEKEY_INVALID, "PopupReplayShutdown",	          reinterpret_cast<void*>(PopupReplayShutdown) },
	{ NAMEKEY_INVALID, NULL,														NULL }

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA
///////////////////////////////////////////////////////////////////////////////////////////////////
FunctionLexicon *TheFunctionLexicon = NULL;  ///< the function dictionary

//-------------------------------------------------------------------------------------------------
/** Since we have a convenient table to organize our callbacks anyway,
	* we'll just use this same storage space to load in any run time
	* components we might want to add to the table, such as generating
	* a key based off the name supplied in the table for faster access */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::loadTable( TableEntry *table,
																 TableIndex tableIndex )
{

	// sanity
	if( table == NULL )
		return;

	// loop through all entries
	TableEntry *entry = table;
	while( entry->name )
	{

		// assign key from name key based on name provided in table
		entry->key = TheNameKeyGenerator->nameToKey( entry->name );

		// next table entry please
		entry++;

	}

	// assign table to the index specified
	m_tables[ tableIndex ] = table;

}

//-------------------------------------------------------------------------------------------------
/** Search the provided table for a function matching the key */
//-------------------------------------------------------------------------------------------------
void *FunctionLexicon::keyToFunc( NameKeyType key, TableEntry *table )
{

	// sanity
	if( key == NAMEKEY_INVALID )
		return NULL;

	// search table for key
	TableEntry *entry = table;
	while( entry && entry->key != NAMEKEY_INVALID )
	{

		if( entry->key == key )
			return entry->func;
		entry++;

	}

	return NULL;  // not found

}

//-------------------------------------------------------------------------------------------------
/** Search tables for the function given this key, if the index parameter
	* is TABLE_ANY, then ALL tables will be searched.  Otherwise index refers
	* to only a single table index to be searched */
//-------------------------------------------------------------------------------------------------
void *FunctionLexicon::findFunction( NameKeyType key, TableIndex index )
{
	void *func = NULL;

	// sanity
	if( key == NAMEKEY_INVALID )
		return NULL;

	// search ALL tables for function if the index paramater allows if
	if( index == TABLE_ANY )
	{

		Int i;
		for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
		{

			func = keyToFunc( key, m_tables[ i ] );
			if( func )
				break;  // exit for i

		}

	}
	else
	{

		// do NOT search all tables, just the one specified by the parameter
		func = keyToFunc( key, m_tables[ index ] );

	}

	// return function, if found
	return func;

}

#ifdef NOT_IN_USE
//-------------------------------------------------------------------------------------------------
/** Search for the function in the specified table */
//-------------------------------------------------------------------------------------------------
const char *FunctionLexicon::funcToName( void *func, TableEntry *table )
{

	// sanity
	if( func == NULL )
		return NULL;

	// search the table
	TableEntry *entry = table;
	while( entry && entry->key != NAMEKEY_INVALID )
	{

		// is this it
		if( entry->func == func )
			return entry->name;

		// not it, check next
		entry++;

	}

	return NULL;  // not found

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::FunctionLexicon( void )
{
	Int i;

	// empty the tables
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
		m_tables[ i ] = NULL;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::~FunctionLexicon( void )
{

}

//-------------------------------------------------------------------------------------------------
/** Initialize our dictionary of funtion pointers and symbols */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::init( void )
{

	// if you change this method, check the reset() and make sure it's OK

	// initialize the name key identifiers for the lookup table
	loadTable( gameWinDrawTable,						TABLE_GAME_WIN_DRAW );
	loadTable( gameWinSystemTable,					TABLE_GAME_WIN_SYSTEM );
	loadTable( gameWinInputTable,						TABLE_GAME_WIN_INPUT );
	loadTable( gameWinTooltipTable,					TABLE_GAME_WIN_TOOLTIP );

	loadTable( winLayoutInitTable,					TABLE_WIN_LAYOUT_INIT );
	loadTable( winLayoutUpdateTable,				TABLE_WIN_LAYOUT_UPDATE );
	loadTable( winLayoutShutdownTable,			TABLE_WIN_LAYOUT_SHUTDOWN );

	validate();

}

//-------------------------------------------------------------------------------------------------
/** reset */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::reset( void )
{

	//
	// make sure the ordering of what happens here with respect to derived classes resets is
	// all OK since we're cheating and using the init() method
	//

	// nothing dynamically loaded, just reinit the tables
	init();

}

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::update( void )
{

}

/*
// !NOTE! We can not have this function, see the header for
// more information as to why
//
//-------------------------------------------------------------------------------------------------
// translate a function pointer to its symbolic name
//-------------------------------------------------------------------------------------------------
char *FunctionLexicon::functionToName( void *func )
{

	// sanity
	if( func == NULL )
		return NULL;

	// search ALL the tables
	Int i;
	char *name = NULL;
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		name = funcToName( func, m_tables[ i ] );
		if( name )
			return name;

	}

	return NULL;  // not found

}
*/

//-------------------------------------------------------------------------------------------------
/** Scan the tables and make sure that each function address is unique.
	* We want to do this to prevent accidental entries of two identical
	* functions and because the compiler will optimize identical functions
	* to the same address (typically in empty functions with no body and the
	* same parameters) which we MUST keep separate for when we add code to
	* them */
//-------------------------------------------------------------------------------------------------
Bool FunctionLexicon::validate( void )
{
	Bool valid = TRUE;
	Int i, j;
	TableEntry *sourceEntry, *lookAtEntry;

	// scan all talbes
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		// scan through this table
		sourceEntry = m_tables[ i ];
		while( sourceEntry && sourceEntry->key != NAMEKEY_INVALID )
		{

			//
			// scan all tables looking for the function in sourceEntry, do not bother
			// of source entry is NULL (a valid entry in the table, but not a function)
			//
			if( sourceEntry->func )
			{

				// scan all tables
				for( j = 0; j < MAX_FUNCTION_TABLES; j++ )
				{

					// scan all entries in this table
					lookAtEntry = m_tables[ j ];
					while( lookAtEntry && lookAtEntry->key != NAMEKEY_INVALID )
					{

						//
						// check for match, do not match the entry source with itself
						//
						if( sourceEntry != lookAtEntry )
							if( sourceEntry->func == lookAtEntry->func )
							{

								DEBUG_LOG(( "WARNING! Function lexicon entries match same address! '%s' and '%s'",
														sourceEntry->name, lookAtEntry->name ));
								valid = FALSE;

							}

						// next entry in this target table
						lookAtEntry++;

					}

				}

			}

			// next source entry
			sourceEntry++;

		}

	}

	// return the valid state of our tables
	return valid;

}

//============================================================================
// FunctionLexicon::gameWinDrawFunc
//============================================================================

GameWinDrawFunc FunctionLexicon::gameWinDrawFunc( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		GameWinDrawFunc func;

		func = (GameWinDrawFunc)findFunction( key, TABLE_GAME_WIN_DEVICEDRAW );
		if ( func == NULL )
		{
			func = (GameWinDrawFunc)findFunction( key, TABLE_GAME_WIN_DRAW );
		}
		return func;
	}
	// search the specified table
	return (GameWinDrawFunc)findFunction( key, index );
}

WindowLayoutInitFunc FunctionLexicon::winLayoutInitFunc( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		WindowLayoutInitFunc func;

		func = (WindowLayoutInitFunc)findFunction( key, TABLE_WIN_LAYOUT_DEVICEINIT );
		if ( func == NULL )
		{
			func = (WindowLayoutInitFunc)findFunction( key, TABLE_WIN_LAYOUT_INIT );
		}
		return func;
	}
	// search the specified table
	return (WindowLayoutInitFunc)findFunction( key, index );
}
