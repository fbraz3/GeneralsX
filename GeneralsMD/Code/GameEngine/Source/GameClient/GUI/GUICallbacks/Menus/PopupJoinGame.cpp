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

// FILE: PopupJoinGame.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Jul 2002
//
//	Filename: 	PopupJoinGame.cpp
//
//	author:		Matthew D. Campbell
//
//	purpose:	Contains the Callbacks for the Join Game Popup
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpyOverlay.h"


#if defined(SAGE_USE_NGMP)
#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_LobbyInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMP_types.h"
#endif

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

static NameKeyType parentPopupID = NAMEKEY_INVALID;
static NameKeyType textEntryGamePasswordID = NAMEKEY_INVALID;
static NameKeyType buttonCancelID = NAMEKEY_INVALID;
static NameKeyType buttonOkID = NAMEKEY_INVALID;

static GameWindow *parentPopup = nullptr;
static GameWindow *textEntryGamePassword = nullptr;
static GameWindow *buttonOk = nullptr;

static void joinGame( AsciiString password );

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/** Initialize the PopupHostGameInit menu */
//-------------------------------------------------------------------------------------------------
void PopupJoinGameInit( WindowLayout *layout, void *userData )
{
	parentPopupID = TheNameKeyGenerator->nameToKey("PopupJoinGame.wnd:ParentJoinPopUp");
	parentPopup = TheWindowManager->winGetWindowFromId(nullptr, parentPopupID);

	textEntryGamePasswordID = TheNameKeyGenerator->nameToKey("PopupJoinGame.wnd:TextEntryGamePassword");
	textEntryGamePassword = TheWindowManager->winGetWindowFromId(parentPopup, textEntryGamePasswordID);
	GadgetTextEntrySetText(textEntryGamePassword, UnicodeString::TheEmptyString);

	NameKeyType staticTextGameNameID = TheNameKeyGenerator->nameToKey("PopupJoinGame.wnd:StaticTextGameName");
	GameWindow *staticTextGameName = TheWindowManager->winGetWindowFromId(parentPopup, staticTextGameNameID);
	GadgetStaticTextSetText(staticTextGameName, UnicodeString::TheEmptyString);

	buttonCancelID = NAMEKEY("PopupJoinGame.wnd:ButtonCancel");

	buttonOkID = TheNameKeyGenerator->nameToKey("PopupJoinGame.wnd:ButtonOK");
	buttonOk = TheWindowManager->winGetWindowFromId(parentPopup, buttonOkID);
	if (!buttonOk && parentPopup)
	{
		WinInstanceData instData;
		instData.init();
		BitSet(instData.m_style, GWS_PUSH_BUTTON | GWS_MOUSE_TRACK);
		instData.m_textLabelString = "GUI:OK";

		buttonOk = TheWindowManager->gogoGadgetPushButton(parentPopup,
			WIN_STATUS_ENABLED | WIN_STATUS_IMAGE,
			355, 314, 90, 26,
			&instData, nullptr, TRUE);
		if (buttonOk)
		{
			buttonOk->winSetWindowId(buttonOkID);
			if (TheGameText)
			{
				GadgetButtonSetText(buttonOk, TheGameText->fetch("GUI:OK"));
			}
		}
	}

#if defined(SAGE_USE_NGMP)
	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface == nullptr)
	{
		DEBUG_LOG(("NGMP_OnlineServices_LobbyInterface is not initialized!"));
		return;
	}

	LobbyEntry lobbyTryingToJoin = pLobbyInterface->GetLobbyTryingToJoin();
	UnicodeString lobbyName;
	lobbyName.translate(AsciiString(lobbyTryingToJoin.name.c_str()));
	GadgetStaticTextSetText(staticTextGameName, lobbyName);
#else
	GameSpyStagingRoom *ourRoom = TheGameSpyInfo->findStagingRoomByID(TheGameSpyInfo->getCurrentStagingRoomID());
	if (ourRoom)
		GadgetStaticTextSetText(staticTextGameName, ourRoom->getGameName());
#endif

	TheWindowManager->winSetFocus( textEntryGamePassword );
	TheWindowManager->winSetModal( parentPopup );

}

//-------------------------------------------------------------------------------------------------
/** PopupHostGameInput callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType PopupJoinGameInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;
//			if (buttonPushed)
//				break;

			switch( key )
			{

				case KEY_ENTER:
				case KEY_KPENTER:
				{
					if( BitIsSet( state, KEY_STATE_UP ) )
					{
						if (textEntryGamePassword)
						{
							UnicodeString txtInput;
							txtInput.set(GadgetTextEntryGetText( textEntryGamePassword ));
							GadgetTextEntrySetText(textEntryGamePassword, UnicodeString::TheEmptyString);
							txtInput.trim();
							if (!txtInput.isEmpty())
							{
								AsciiString munkee;
								munkee.translate(txtInput);
								joinGame(munkee);
							}
						}
					}
					return MSG_HANDLED;
				}

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{

					//
					// send a simulated selected event to the parent window of the
					// back/exit button
					//
					if( BitIsSet( state, KEY_STATE_UP ) )
					{
						GameSpyCloseOverlay(GSOVERLAY_GAMEPASSWORD);
						SetLobbyAttemptHostJoin( FALSE );
						parentPopup = nullptr;
						buttonOk = nullptr;
					}

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}

			}

		}

	}

	return MSG_IGNORED;

}

//-------------------------------------------------------------------------------------------------
/** PopupHostGameSystem callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType PopupJoinGameSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{
  switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			break;

		}
    //---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			buttonOk = nullptr;
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			if (controlID == buttonCancelID)
			{
				GameSpyCloseOverlay(GSOVERLAY_GAMEPASSWORD);
				SetLobbyAttemptHostJoin( FALSE );
				parentPopup = nullptr;
				buttonOk = nullptr;
			}
			else if (controlID == buttonOkID)
			{
				if (textEntryGamePassword)
				{
					UnicodeString txtInput;
					txtInput.set(GadgetTextEntryGetText( textEntryGamePassword ));
					GadgetTextEntrySetText(textEntryGamePassword, UnicodeString::TheEmptyString);
					txtInput.trim();
					if (!txtInput.isEmpty())
					{
						AsciiString munkee;
						munkee.translate(txtInput);
						joinGame(munkee);
					}
				}
			}
			break;
		}

    //----------------------------------------------------------------------------------------------
    case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}
    //---------------------------------------------------------------------------------------------
		case GEM_EDIT_DONE:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

      if( controlID == textEntryGamePasswordID )
			{
				// read the user's input and clear the entry box
				UnicodeString txtInput;
				txtInput.set(GadgetTextEntryGetText( textEntryGamePassword ));
				GadgetTextEntrySetText(textEntryGamePassword, UnicodeString::TheEmptyString);
				txtInput.trim();
				if (!txtInput.isEmpty())
				{
					AsciiString munkee;
					munkee.translate(txtInput);
					joinGame(munkee);
				}
			}
			break;
		}
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}


//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

static void joinGame( AsciiString password )
{
#if defined(SAGE_USE_NGMP)
	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface == nullptr)
	{
		DEBUG_LOG(("NGMP_OnlineServices_LobbyInterface is not initialized!"));
		GameSpyCloseOverlay(GSOVERLAY_GAMEPASSWORD);
		SetLobbyAttemptHostJoin(FALSE);
		parentPopup = nullptr;
		buttonOk = nullptr;
		return;
	}

	LobbyEntry lobbyTryingToJoin = pLobbyInterface->GetLobbyTryingToJoin();

	if (lobbyTryingToJoin.lobbyID == -1)
	{
		GameSpyCloseOverlay(GSOVERLAY_GAMEPASSWORD);
		SetLobbyAttemptHostJoin(FALSE);
		parentPopup = nullptr;
		buttonOk = nullptr;
		return;
	}

	pLobbyInterface->JoinLobby(lobbyTryingToJoin, password.str());
	DEBUG_LOG(("Attempting to join game %d(%s) with password [%s]\n", lobbyTryingToJoin.lobbyID, lobbyTryingToJoin.name.c_str(), password.str()));
#else
	GameSpyStagingRoom *ourRoom = TheGameSpyInfo->findStagingRoomByID(TheGameSpyInfo->getCurrentStagingRoomID());
	if (!ourRoom)
	{
		GameSpyCloseOverlay(GSOVERLAY_GAMEPASSWORD);
		SetLobbyAttemptHostJoin( FALSE );
		parentPopup = nullptr;
		buttonOk = nullptr;
		return;
	}
	PeerRequest req;
	req.peerRequestType = PeerRequest::PEERREQUEST_JOINSTAGINGROOM;
	req.text = ourRoom->getGameName().str();
	req.stagingRoom.id = ourRoom->getID();
	req.password = password.str();
	TheGameSpyPeerMessageQueue->addRequest(req);
	DEBUG_LOG(("Attempting to join game %d(%ls) with password [%s]", ourRoom->getID(), ourRoom->getGameName().str(), password.str()));
#endif

	GameSpyCloseOverlay(GSOVERLAY_GAMEPASSWORD);
	parentPopup = nullptr;
	buttonOk = nullptr;
}
