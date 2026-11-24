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

/*
** SDL3GameEngine.cpp
**
** Linux implementation of GameEngine using SDL3 for windowing/input.
**
** TheSuperHackers @feature CnC_Generals_Linux 07/02/2026
** Provides SDL3-based input and window management for Linux builds.
** Based on fighter19 reference implementation.
*/

#ifndef _WIN32

#include "SDL3GameEngine.h"
#include "NullAudioDevice/NullAudioManager.h"
// GeneralsX @build Mr. Meeseeks 16/06/2026 Make audio headers mutually exclusive to avoid redefinition conflicts
#ifdef SAGE_USE_MINIAUDIO
#include "MiniAudioDevice/MiniAudioManager.h"
#elif defined(SAGE_USE_OPENAL)
#include "OpenALAudioManager.h"
#endif
#include "SDL3Device/GameClient/SDL3Mouse.h"
#include "SDL3Device/GameClient/SDL3Keyboard.h"
#include "GameClient/Mouse.h"
#include "GameClient/Keyboard.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/Common/W3DThingFactory.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "StdDevice/Common/StdLocalFileSystem.h"
#include "Common/BIGFileSystem.h"
#include "Common/FramePacer.h"
#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "Common/AudioAffect.h"
#include "Common/GameAudio.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/ArchiveFile.h"
#include "Common/LocalFileSystem.h"
#include "GameLogic/VictoryConditions.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/NetworkInterface.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include "WebGPUDevice/WebGPUD3D8.h"
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(__EMSCRIPTEN__)
extern int __argc;
extern char **__argv;
extern void StartPressed();

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXOpenLanMenu()
{
	if (TheShell == nullptr)
		return FALSE;
	TheShell->push("Menus/LanLobbyMenu.wnd", TRUE);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLogicFrame()
{
	return TheGameLogic != nullptr ? TheGameLogic->getFrame() : 0;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanSetIdentity(Int client)
{
	if (TheLAN == nullptr || client < 1 || client > 254)
		return FALSE;
	UnicodeString name;
	name.format(L"Browser%d", client);
	TheLAN->RequestSetName(name);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanSetName(const char *label)
{
	if (TheLAN == nullptr || label == nullptr || label[0] == '\0')
		return FALSE;
	AsciiString ascii(label);
	UnicodeString name;
	name.translate(ascii);
	TheLAN->RequestSetName(name);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanHost()
{
	if (TheLAN == nullptr || TheLAN->GetMyGame() != nullptr)
		return FALSE;
	TheLAN->RequestGameCreate(L"GeneralsX", FALSE);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanJoinFirst()
{
	if (TheLAN == nullptr || TheLAN->GetMyGame() != nullptr)
		return FALSE;
	LANGameInfo *game = TheLAN->LookupGameByListOffset(0);
	if (game == nullptr)
		return FALSE;
	TheLAN->RequestGameJoin(game);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanAccept()
{
	if (TheLAN == nullptr || TheLAN->GetMyGame() == nullptr)
		return FALSE;
	TheLAN->RequestAccept();
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXMouseX()
{
	return TheMouse != nullptr ? TheMouse->getMouseStatus()->pos.x : -1;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXMouseY()
{
	return TheMouse != nullptr ? TheMouse->getMouseStatus()->pos.y : -1;
}

EM_JS(int, GeneralsXEnsureResident, (const char *big, double offset, double size), {
	const ensure = globalThis.__gxEnsure;
	return ensure ? ensure(UTF8ToString(big), offset, size) : 1;
});

extern "C" int GeneralsXFileResident(const char *name)
{
	if (TheGameLogic == nullptr || !TheGameLogic->isInGame() || TheGameLogic->isLoadingMap())
		return 1;
	if (name == nullptr || *name == '\0' || TheArchiveFileSystem == nullptr)
		return 1;
	if (TheLocalFileSystem != nullptr && TheLocalFileSystem->doesFileExist(name))
		return 1;
	AsciiString path(name);
	ArchiveFile *archive = TheArchiveFileSystem->friend_getArchiveFile(path);
	if (archive == nullptr)
		return 1;
	const ArchivedFileInfo *info = archive->friend_getArchivedFileInfo(path);
	if (info == nullptr)
		return 1;
	if (info->m_size <= 512 * 1024)
		return 1;
	// Streamed music (.mp3) is read in small chunks via MA_SOUND_FLAG_STREAM; do not drop it on first trigger.
	if (path.endsWithNoCase(".mp3"))
		return 1;
	return GeneralsXEnsureResident(archive->getName().str(), (double)info->m_offset, (double)info->m_size);
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXSetAudioMuted(Int muted)
{
	if (TheAudio == nullptr)
		return FALSE;
	TheAudio->setVolume(muted ? 0.0f : 1.0f, AudioAffect_All);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXSetPaused(Int paused)
{
	if (TheGameLogic == nullptr || !TheGameLogic->isInGame() || TheGameLogic->isLoadingMap())
		return FALSE;
	if (TheNetwork != nullptr)
		return FALSE;
	const Bool want = paused ? TRUE : FALSE;
	if ((TheGameLogic->isGamePaused() ? TRUE : FALSE) == want)
		return TRUE;
	TheGameLogic->setGamePaused(want);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanSetMapMinPlayers(Int minPlayers)
{
	if (TheLAN == nullptr || TheLAN->GetMyGame() == nullptr || !TheLAN->AmIHost() || TheMapCache == nullptr)
		return FALSE;
	TheMapCache->updateCache();
	const MapMetaData *best = nullptr;
	AsciiString bestName;
	for (std::map<AsciiString, MapMetaData>::iterator it = TheMapCache->begin(); it != TheMapCache->end(); ++it)
	{
		const MapMetaData &meta = it->second;
		if (!meta.m_isMultiplayer || !meta.m_isOfficial || meta.m_numPlayers < minPlayers)
			continue;
		if (best == nullptr || meta.m_numPlayers < best->m_numPlayers)
		{
			best = &it->second;
			bestName = it->first;
		}
	}
	if (best == nullptr)
		return FALSE;
	LANGameInfo *game = TheLAN->GetMyGame();
	game->setMap(bestName);
	game->getSlot(0)->setMapAvailability(true);
	game->setMapCRC(best->m_CRC);
	game->setMapSize(best->m_filesize);
	game->resetStartSpots();
	game->adjustSlotsForMap();
	game->resetAccepted();
	TheLAN->RequestGameOptions(GenerateGameOptionsString(), true);
	printf("GeneralsXLanSetMapMinPlayers: %s (%d players)\n", bestName.str(), best->m_numPlayers);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanSetSlotAI(Int slotIndex, Int difficulty)
{
	if (TheLAN == nullptr || TheLAN->GetMyGame() == nullptr || !TheLAN->AmIHost())
		return FALSE;
	if (slotIndex < 1 || slotIndex >= MAX_SLOTS || difficulty < 0 || difficulty > 2)
		return FALSE;
	LANGameInfo *game = TheLAN->GetMyGame();
	GameSlot *slot = game->getLANSlot(slotIndex);
	if (slot == nullptr || slot->getState() == SLOT_PLAYER)
		return FALSE;
	static const SlotState aiStates[3] = { SLOT_EASY_AI, SLOT_MED_AI, SLOT_BRUTAL_AI };
	slot->setState(aiStates[difficulty]);
	game->resetAccepted();
	TheLAN->RequestGameOptions(GenerateGameOptionsString(), true);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanStart()
{
	if (TheLAN == nullptr || TheLAN->GetMyGame() == nullptr || !TheLAN->AmIHost())
		return FALSE;
	StartPressed();
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanSurrender()
{
	if (TheGameLogic == nullptr || TheGameLogic->getGameMode() != GAME_LAN)
		return FALSE;
	GameMessage *msg = TheMessageStream->appendMessage(GameMessage::MSG_SELF_DESTRUCT);
	msg->appendBooleanArgument(TRUE);
	return TRUE;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanState()
{
	Int state = 0;
	if (TheLAN != nullptr)
		state |= 1;
	if (TheLAN != nullptr && TheLAN->GetMyGame() != nullptr)
		state |= 2;
	if (TheLAN != nullptr && TheLAN->AmIHost())
		state |= 4;
	if (TheLAN != nullptr && TheLAN->GetMyGame() != nullptr && TheLAN->GetMyGame()->isGameInProgress())
		state |= 8;
	if (TheNetwork != nullptr)
		state |= 16;
	if (TheGameLogic != nullptr && TheGameLogic->getGameMode() == GAME_LAN)
		state |= 32;
	if (TheVictoryConditions != nullptr && TheVictoryConditions->getEndFrame() != 0)
		state |= 64;
	if (TheNetwork != nullptr && TheNetwork->sawCRCMismatch())
		state |= 128;
	return state;
}

extern "C" EMSCRIPTEN_KEEPALIVE Int GeneralsXLanEndFrame()
{
	return TheVictoryConditions != nullptr ? TheVictoryConditions->getEndFrame() : 0;
}
#endif

// Extern globals for input devices (set by GameClient)
extern Mouse *TheMouse;
extern Keyboard *TheKeyboard;
extern GameWindowManager *TheWindowManager;

namespace {

Bool DecodeNextUtf8Codepoint(const char* text, size_t length, size_t& offset, UnsignedInt& outCodepoint)
{
	outCodepoint = 0;
	if (!text || offset >= length) {
		return false;
	}

	const unsigned char first = static_cast<unsigned char>(text[offset]);
	if (first == 0) {
		return false;
	}

	if (first < 0x80) {
		outCodepoint = first;
		offset += 1;
		return true;
	}

	if ((first & 0xE0) == 0xC0 && offset + 1 < length) {
		const unsigned char second = static_cast<unsigned char>(text[offset + 1]);
		if ((second & 0xC0) == 0x80) {
			outCodepoint = ((first & 0x1F) << 6) | (second & 0x3F);
			offset += 2;
			return true;
		}
	}

	if ((first & 0xF0) == 0xE0 && offset + 2 < length) {
		const unsigned char second = static_cast<unsigned char>(text[offset + 1]);
		const unsigned char third = static_cast<unsigned char>(text[offset + 2]);
		if ((second & 0xC0) == 0x80 && (third & 0xC0) == 0x80) {
			outCodepoint = ((first & 0x0F) << 12) | ((second & 0x3F) << 6) | (third & 0x3F);
			offset += 3;
			return true;
		}
	}

	if ((first & 0xF8) == 0xF0 && offset + 3 < length) {
		const unsigned char second = static_cast<unsigned char>(text[offset + 1]);
		const unsigned char third = static_cast<unsigned char>(text[offset + 2]);
		const unsigned char fourth = static_cast<unsigned char>(text[offset + 3]);
		if ((second & 0xC0) == 0x80 && (third & 0xC0) == 0x80 && (fourth & 0xC0) == 0x80) {
			outCodepoint = ((first & 0x07) << 18) | ((second & 0x3F) << 12) | ((third & 0x3F) << 6) | (fourth & 0x3F);
			offset += 4;
			return true;
		}
	}

	// Invalid UTF-8 sequence: skip one byte and keep processing.
	offset += 1;
	return false;
}

}

/**
 * Constructor: Initialize SDL3 game engine state
 */
SDL3GameEngine::SDL3GameEngine()
	: GameEngine(),
	  m_SDLWindow(nullptr),
	  m_IsInitialized(false),
	  m_IsActive(false),
	  m_IsTextInputActive(false),
	  m_TextInputFocusWindow(nullptr)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::SDL3GameEngine() created\n");
}

/**
 * Destructor: Cleanup SDL3 resources
 */
SDL3GameEngine::~SDL3GameEngine()
{
	if (m_SDLWindow && m_IsTextInputActive) {
		SDL_StopTextInput(m_SDLWindow);
		m_IsTextInputActive = false;
		m_TextInputFocusWindow = nullptr;
	}

	if (m_IsInitialized) {
		// Window cleanup is done in reset/shutdown
	}
	fprintf(stderr, "DEBUG: SDL3GameEngine::~SDL3GameEngine() destroyed\n");
}

/**
 * From GameEngine: init() - initialize subsystems
 * 
 * GeneralsX @bugfix felipebraz 16/02/2026
 * Simplified to follow fighter19 pattern - SDL3/Vulkan initialized in SDL3Main.cpp
 * before GameEngine is created. This init() only delegates to parent GameEngine::init().
 * ApplicationHWnd and TheSDL3Window are already set by main() before this is called.
 */
void SDL3GameEngine::init(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::init() starting\n");

	if (TheGlobalData && TheGlobalData->m_headless) {
		// GeneralsX @bugfix Copilot 17/05/2026 Allow headless replay path to initialize engine subsystems without an SDL window.
		fprintf(stderr, "INFO: SDL3GameEngine::init() headless mode - skipping SDL window binding\n");
		m_SDLWindow = nullptr;
		m_IsInitialized = true;
		m_IsActive = true;
		GameEngine::init();
		return;
	}

	// Verify window was created by SDL3Main.cpp
	extern SDL_Window* TheSDL3Window;
	extern HWND ApplicationHWnd;
	
	if (!TheSDL3Window || !ApplicationHWnd) {
		fprintf(stderr, "FATAL: SDL3 window not initialized before GameEngine::init()\n");
		fprintf(stderr, "FATAL: TheSDL3Window=%p, ApplicationHWnd=%p\n", TheSDL3Window, ApplicationHWnd);
		return;
	}

	// Store window reference locally
	m_SDLWindow = TheSDL3Window;
	m_IsInitialized = true;
	m_IsActive = true;

	fprintf(stderr, "INFO: SDL3GameEngine using pre-initialized window\n");

	// Call parent init to initialize game subsystems
	GameEngine::init();
}

/**
 * From GameEngine: reset() - reset system to starting state
 */
void SDL3GameEngine::reset(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::reset()\n");
	if (m_SDLWindow && m_IsTextInputActive) {
		SDL_StopTextInput(m_SDLWindow);
		m_IsTextInputActive = false;
		m_TextInputFocusWindow = nullptr;
	}
	GameEngine::reset();
}

/**
 * From GameEngine: update() - per-frame update
 */
void SDL3GameEngine::update(void)
{
	pollSDL3Events();
	GameEngine::update();
}

/**
 * From GameEngine: execute() - main game loop
 */
void SDL3GameEngine::execute(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::execute() - entering main loop\n");
#if defined(__EMSCRIPTEN__)
	if (TheGlobalData && TheGlobalData->m_headless) {
		GameEngine::execute();
		fprintf(stderr, "INFO: SDL3GameEngine::execute() - exited headless main loop\n");
		return;
	}
	TheFramePacer->reset();
	emscripten_set_main_loop_arg(browserFrame, this, 0, true);
	emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 16);
#else
	GameEngine::execute();
	fprintf(stderr, "INFO: SDL3GameEngine::execute() - exited main loop\n");
#endif
}

#if defined(__EMSCRIPTEN__)
static void* s_gxPumpEngine = nullptr;
static Bool s_gxInFrame = FALSE;

extern "C" EMSCRIPTEN_KEEPALIVE void GeneralsXPump(void)
{
	if (s_gxPumpEngine != nullptr)
		SDL3GameEngine::browserFrame(s_gxPumpEngine);
}

void SDL3GameEngine::browserFrame(void* enginePointer)
{
	if (s_gxInFrame)
		return;
	s_gxInFrame = TRUE;
	s_gxPumpEngine = enginePointer;
	SDL3GameEngine* engine = static_cast<SDL3GameEngine*>(enginePointer);
	if (engine->getQuitting())
	{
		s_gxInFrame = FALSE;
		emscripten_cancel_main_loop();
		return;
	}

	const Bool canDraw = WebGPUDeviceCanSubmitFrame();
	TheGameClient->setDrawingEnabled(canDraw);
	engine->executeFrame();
	TheGameClient->setDrawingEnabled(TRUE);
	s_gxInFrame = FALSE;
}
#endif

/**
 * From GameEngine: serviceWindowsOS() - native OS service
 * On Linux, process SDL3 events
 */
void SDL3GameEngine::serviceWindowsOS(void)
{
	pollSDL3Events();
}

/**
 * Check if game has OS focus
 */
Bool SDL3GameEngine::isActive(void)
{
	return m_IsActive;
}

/**
 * Set OS focus status
 */
void SDL3GameEngine::setIsActive(Bool isActive)
{
	m_IsActive = isActive;
}

/**
 * Poll and process SDL3 events
 * Handles keyboard, mouse, window, and quit events
 */
void SDL3GameEngine::pollSDL3Events(void)
{
	if (!m_SDLWindow) {
		return;
	}

	updateTextInputState();

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				m_quitting = true;
				break;

			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				m_quitting = true;
				break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				m_IsActive = true;
				if (TheMouse) {
					TheMouse->regainFocus();
					TheMouse->refreshCursorCapture();
				}
				break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				m_IsActive = false;
				if (m_IsTextInputActive) {
					SDL_StopTextInput(m_SDLWindow);
					m_IsTextInputActive = false;
					m_TextInputFocusWindow = nullptr;
				}
				if (TheMouse) {
					TheMouse->loseFocus();
				}
				break;

			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				if (TheMouse) {
					TheMouse->onCursorMovedInside();
				}
				break;

			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
				if (TheMouse) {
					TheMouse->onCursorMovedOutside();
				}
				break;

			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				// Fighter19 pattern: direct addSDLEvent() call
				// GeneralsX @refactor felipebraz 16/02/2026 Simplified event routing
				if (TheKeyboard) {
					SDL3Keyboard* keyboard = dynamic_cast<SDL3Keyboard*>(TheKeyboard);
					if (keyboard) {
						keyboard->addSDLEvent(&event);
					}
				}
				break;

			case SDL_EVENT_TEXT_INPUT:
				forwardTextInputEvent(event.text.text);
				break;

			case SDL_EVENT_MOUSE_MOTION:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_WHEEL:
				// Fighter19 pattern: direct addSDLEvent() call with raw SDL_Event
				// GeneralsX @refactor felipebraz 16/02/2026 Simplified event routing
				if (TheMouse) {
					SDL3Mouse* mouse = dynamic_cast<SDL3Mouse*>(TheMouse);
					if (mouse) {
						mouse->addSDLEvent(&event);
					}
				}
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				handleWindowEvent(event.window);
				break;

			default:
				// Ignore other events for now
				break;
		}

		updateTextInputState();
	}
}

// GeneralsX @bugfix felipebraz 01/04/2026 Enable SDL text input only while an entry gadget owns focus.
void SDL3GameEngine::updateTextInputState(void)
{
	if (!m_SDLWindow || !TheWindowManager) {
		return;
	}

	GameWindow* focusedWindow = TheWindowManager->winGetFocus();
	const Bool wantsTextInput =
		focusedWindow != nullptr && BitIsSet(focusedWindow->winGetStyle(), GWS_ENTRY_FIELD);

	if (wantsTextInput) {
		if (!m_IsTextInputActive) {
			if (SDL_StartTextInput(m_SDLWindow)) {
				m_IsTextInputActive = true;
			}
		}
		m_TextInputFocusWindow = focusedWindow;
	} else {
		if (m_IsTextInputActive) {
			SDL_StopTextInput(m_SDLWindow);
			m_IsTextInputActive = false;
		}
		m_TextInputFocusWindow = nullptr;
	}
}

// GeneralsX @bugfix felipebraz 01/04/2026 Forward SDL UTF-8 text input through existing GWM_IME_CHAR path.
void SDL3GameEngine::forwardTextInputEvent(const char* utf8Text)
{
	if (!utf8Text || !TheWindowManager) {
		return;
	}

	// GeneralsX @bugfix felipebraz 01/04/2026 Use tracked text-input focus window to keep SDL text delivery stable.
	GameWindow* targetWindow = m_TextInputFocusWindow;
	if (!targetWindow || !BitIsSet(targetWindow->winGetStyle(), GWS_ENTRY_FIELD)) {
		return;
	}

	const size_t textLength = strlen(utf8Text);
	size_t offset = 0;
	while (offset < textLength) {
		UnsignedInt codepoint = 0;
		if (!DecodeNextUtf8Codepoint(utf8Text, textLength, offset, codepoint)) {
			continue;
		}

		// GeneralsX @bugfix felipebraz 01/04/2026 Clamp IME char forwarding to BMP and reject UTF-16 surrogate range.
		if (codepoint == 0 || codepoint > 0x10FFFFU) {
			continue;
		}

		if (codepoint >= 0xD800U && codepoint <= 0xDFFFU) {
			continue;
		}

		if (codepoint > 0xFFFFU) {
			continue;
		}

		const WideChar wideCharacter = static_cast<WideChar>(codepoint);
		TheWindowManager->winSendInputMsg(targetWindow, GWM_IME_CHAR, static_cast<WindowMsgData>(wideCharacter), 0);
	}
}

/**
 * Handle keyboard event -dispatch to Keyboard manager
 * TheSuperHackers @build 10/02/2026 BenderAI - Phase 1.5 event wiring
 */
void SDL3GameEngine::handleKeyboardEvent(const SDL_KeyboardEvent& event)
{
	// Dispatch to SDL3Keyboard if available
	if (TheKeyboard) {
		SDL3Keyboard* sdlKeyboard = dynamic_cast<SDL3Keyboard*>(TheKeyboard);
		if (sdlKeyboard) {
			sdlKeyboard->addSDL3KeyEvent(event);
		}
	}
}

/**
 * Handle mouse motion event - dispatch to Mouse manager
 * TheSuperHackers @build 10/02/2026 BenderAI - Phase 1.5 event wiring
 */
void SDL3GameEngine::handleMouseMotionEvent(const SDL_MouseMotionEvent& event)
{
	// Dispatch to SDL3Mouse if available
	if (TheMouse) {
		SDL3Mouse* sdlMouse = dynamic_cast<SDL3Mouse*>(TheMouse);
		if (sdlMouse) {
			sdlMouse->addSDL3MouseMotionEvent(event);
		}
	}
}

/**
 * Handle mouse button event - dispatch to Mouse manager
 * TheSuperHackers @build 10/02/2026 BenderAI - Phase 1.5 event wiring
 */
void SDL3GameEngine::handleMouseButtonEvent(const SDL_MouseButtonEvent& event)
{
	// Dispatch to SDL3Mouse if available
	if (TheMouse) {
		SDL3Mouse* sdlMouse = dynamic_cast<SDL3Mouse*>(TheMouse);
		if (sdlMouse) {
			sdlMouse->addSDL3MouseButtonEvent(event);
		}
	}
}

/**
 * Handle mouse wheel event - dispatch to Mouse manager
 * TheSuperHackers @build 10/02/2026 BenderAI - Phase 1.5 event wiring
 */
void SDL3GameEngine::handleMouseWheelEvent(const SDL_MouseWheelEvent& event)
{
	// Dispatch to SDL3Mouse if available
	if (TheMouse) {
		SDL3Mouse* sdlMouse = dynamic_cast<SDL3Mouse*>(TheMouse);
		if (sdlMouse) {
			sdlMouse->addSDL3MouseWheelEvent(event);
		}
	}
}

/**
 * Handle window event (resize, etc.)
 */
void SDL3GameEngine::handleWindowEvent(const SDL_WindowEvent& event)
{
	// TODO: Phase 2 - Handle window resize, notify graphics subsystem
	// fprintf(stderr, "DEBUG: Window event (type=%d)\n", event.type);
}

/**
 * Factory Methods for GameEngine subsystems
 * TheSuperHackers @build felipebraz 13/02/2026
 * Implementations in .cpp to provide complete type definitions and avoid circular includes
 */

LocalFileSystem *SDL3GameEngine::createLocalFileSystem(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createLocalFileSystem() -> StdLocalFileSystem\n");
	return NEW StdLocalFileSystem;
}

ArchiveFileSystem *SDL3GameEngine::createArchiveFileSystem(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createArchiveFileSystem() -> BIGFileSystem\n");
	return NEW BIGFileSystem;
}

GameLogic *SDL3GameEngine::createGameLogic(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createGameLogic() -> W3DGameLogic\n");
	return NEW W3DGameLogic;
}

GameClient *SDL3GameEngine::createGameClient(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createGameClient() -> W3DGameClient\n");
	return NEW W3DGameClient;
}

ModuleFactory *SDL3GameEngine::createModuleFactory(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createModuleFactory() -> W3DModuleFactory\n");
	return NEW W3DModuleFactory;
}

ThingFactory *SDL3GameEngine::createThingFactory(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createThingFactory() -> W3DThingFactory\n");
	return NEW W3DThingFactory;
}

FunctionLexicon *SDL3GameEngine::createFunctionLexicon(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createFunctionLexicon() -> W3DFunctionLexicon\n");
	return NEW W3DFunctionLexicon;
}

// GeneralsX @bugfix Copilot 15/04/2026 Match upstream GameEngine pure-virtual signature after sync.
Radar *SDL3GameEngine::createRadar(Bool dummy)
{
	// GeneralsX @bugfix fbraz 04/05/2026 Respect headless mode and create dummy radar.
	// Upstream reference: Win32GameEngine headless factory behavior, TheSuperHackers/GeneralsGameCode
	// https://github.com/TheSuperHackers/GeneralsGameCode
	if (dummy) {
		fprintf(stderr, "INFO: SDL3GameEngine::createRadar() -> RadarDummy (headless)\n");
		return NEW RadarDummy;
	}
	fprintf(stderr, "INFO: SDL3GameEngine::createRadar() -> W3DRadar\n");
	return NEW W3DRadar;
}

// GeneralsX @bugfix Copilot 24/03/2026 Match upstream GameEngine pure-virtual signature after sync.
ParticleSystemManager* SDL3GameEngine::createParticleSystemManager(Bool dummy)
{
	// GeneralsX @bugfix fbraz 04/05/2026 Respect headless mode and create dummy particle manager.
	if (dummy) {
		fprintf(stderr, "INFO: SDL3GameEngine::createParticleSystemManager() -> ParticleSystemManagerDummy (headless)\n");
		return NEW ParticleSystemManagerDummy;
	}
	fprintf(stderr, "INFO: SDL3GameEngine::createParticleSystemManager() -> W3DParticleSystemManager\n");
	return NEW W3DParticleSystemManager;
}

/**
 * Factory method: AudioManager
 * Select audio backend based on compile flags
 * GeneralsX @bugfix Copilot 15/04/2026 Match upstream GameEngine pure-virtual signature after sync.
 */
AudioManager *SDL3GameEngine::createAudioManager(Bool dummy)
{
	(void)dummy;
	fprintf(stderr, "INFO: SDL3GameEngine::createAudioManager()\n");
	if (TheGlobalData && !TheGlobalData->m_audioOn) {
		fprintf(stderr, "INFO: Creating NullAudio audio backend\n");
		return new NullAudioManager();
	}

#ifdef SAGE_USE_MINIAUDIO
	fprintf(stderr, "INFO: Creating MiniAudio audio backend\n");
	return new MiniAudioManager();
#elif defined(SAGE_USE_OPENAL)
	fprintf(stderr, "INFO: Creating OpenAL audio backend\n");
	return new OpenALAudioManager();
#else
	fprintf(stderr, "INFO: Audio backend not available (SAGE_USE_OPENAL/SAGE_USE_MINIAUDIO not defined)\n");
	return new NullAudioManager();
#endif
}

#endif // !_WIN32

