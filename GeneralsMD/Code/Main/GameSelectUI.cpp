/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

// GeneralsX @feature cheviiot 24/10/2026 PLAN-023: monolithic pre-init GameSelect (SDL), then lock GameProfile.
// Window is kept for seamless handoff; SDL3Main recreates it with SDL_WINDOW_VULKAN for DXVK.

#ifndef _WIN32

#include "GameSelectUI.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cstdio>
#include <cstring>
#if defined(__linux__)
#include <unistd.h>
#endif

namespace GameSelectUI
{
namespace
{

struct Card
{
	SDL_FRect rect;
	const char *title;
	const char *subtitle;
	const char *hint;
	GameProfileKind kind;
	SDL_Texture *icon;
};

static void fillRect(SDL_Renderer *r, const SDL_FRect &rc, Uint8 R, Uint8 G, Uint8 B, Uint8 A = 255)
{
	SDL_SetRenderDrawColor(r, R, G, B, A);
	SDL_RenderFillRect(r, &rc);
}

static void drawFrame(SDL_Renderer *r, const SDL_FRect &rc, Uint8 R, Uint8 G, Uint8 B, int thickness)
{
	SDL_FRect edge = { rc.x, rc.y, rc.w, (float)thickness };
	fillRect(r, edge, R, G, B);
	edge = { rc.x, rc.y + rc.h - (float)thickness, rc.w, (float)thickness };
	fillRect(r, edge, R, G, B);
	edge = { rc.x, rc.y, (float)thickness, rc.h };
	fillRect(r, edge, R, G, B);
	edge = { rc.x + rc.w - (float)thickness, rc.y, (float)thickness, rc.h };
	fillRect(r, edge, R, G, B);
}

static Bool pointIn(const SDL_FRect &rc, float x, float y)
{
	return x >= rc.x && y >= rc.y && x < rc.x + rc.w && y < rc.y + rc.h;
}

static void drawText(SDL_Renderer *r, float x, float y, const char *text, Uint8 R, Uint8 G, Uint8 B)
{
	SDL_SetRenderDrawColor(r, R, G, B, 255);
	SDL_RenderDebugText(r, x, y, text);
}

static void drawTextBold(SDL_Renderer *r, float x, float y, const char *text, Uint8 R, Uint8 G, Uint8 B)
{
	drawText(r, x, y, text, R, G, B);
	drawText(r, x + 1.0f, y, text, R, G, B);
	drawText(r, x, y + 1.0f, text, R, G, B);
}

static void findAssetPath(char *out, size_t outSize, const char *relative)
{
	out[0] = '\0';
	static char buf[1024];
	static char exeDir[512];
	exeDir[0] = '\0';
#if defined(__linux__)
	{
		char link[512];
		const ssize_t n = readlink("/proc/self/exe", link, sizeof(link) - 1);
		if (n > 0) {
			link[n] = '\0';
			char *slash = strrchr(link, '/');
			if (slash != nullptr) {
				*slash = '\0';
				std::snprintf(exeDir, sizeof(exeDir), "%s", link);
			}
		}
	}
#endif
	const char *prefixes[] = {
		"assets/",
		"../assets/",
		"../../assets/",
		"../../../assets/",
		"../../../../assets/",
	};
	if (exeDir[0] != '\0') {
		for (int up = 0; up < 6; ++up) {
			std::snprintf(buf, sizeof(buf), "%s/", exeDir);
			for (int i = 0; i < up; ++i) {
				std::strncat(buf, "../", sizeof(buf) - std::strlen(buf) - 1);
			}
			std::strncat(buf, "assets/", sizeof(buf) - std::strlen(buf) - 1);
			std::strncat(buf, relative, sizeof(buf) - std::strlen(buf) - 1);
			SDL_IOStream *io = SDL_IOFromFile(buf, "rb");
			if (io != nullptr) {
				SDL_CloseIO(io);
				std::snprintf(out, outSize, "%s", buf);
				return;
			}
		}
	}
	for (size_t p = 0; p < sizeof(prefixes) / sizeof(prefixes[0]); ++p) {
		std::snprintf(buf, sizeof(buf), "%s%s", prefixes[p], relative);
		SDL_IOStream *io = SDL_IOFromFile(buf, "rb");
		if (io != nullptr) {
			SDL_CloseIO(io);
			std::snprintf(out, outSize, "%s", buf);
			return;
		}
	}
}

static SDL_Texture *loadTexture(SDL_Renderer *r, const char *relative)
{
	char path[1024];
	findAssetPath(path, sizeof(path), relative);
	if (path[0] == '\0') {
		return nullptr;
	}
	SDL_Surface *surf = IMG_Load(path);
	if (surf == nullptr) {
		fprintf(stderr, "[GAME_SELECT] icon load failed '%s': %s\n", path, SDL_GetError());
		fflush(stderr);
		return nullptr;
	}
	SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
	SDL_DestroySurface(surf);
	return tex;
}

static void drawCard(SDL_Renderer *r, const Card &card, Bool selected, Bool hovered)
{
	const Uint8 baseR = selected ? 48 : (hovered ? 38 : 26);
	const Uint8 baseG = selected ? 38 : (hovered ? 30 : 20);
	const Uint8 baseB = selected ? 18 : (hovered ? 14 : 10);
	fillRect(r, card.rect, baseR, baseG, baseB);

	// Inner panel (retail menu panel feel).
	SDL_FRect inner = {
		card.rect.x + 10.0f,
		card.rect.y + 10.0f,
		card.rect.w - 20.0f,
		card.rect.h - 20.0f
	};
	fillRect(r, inner, (Uint8)(baseR + 8), (Uint8)(baseG + 6), (Uint8)(baseB + 4));

	if (selected) {
		drawFrame(r, card.rect, 201, 162, 39, 4);
		drawFrame(r, inner, 160, 130, 40, 1);
	} else if (hovered) {
		drawFrame(r, card.rect, 170, 145, 70, 2);
	} else {
		drawFrame(r, card.rect, 100, 80, 40, 1);
	}

	if (card.icon != nullptr) {
		const float iconSize = 96.0f;
		SDL_FRect dst = {
			card.rect.x + (card.rect.w - iconSize) * 0.5f,
			card.rect.y + 36.0f,
			iconSize,
			iconSize
		};
		SDL_RenderTexture(r, card.icon, nullptr, &dst);
	}

	drawTextBold(r, card.rect.x + 24.0f, card.rect.y + 150.0f, card.title, 235, 215, 160);
	drawText(r, card.rect.x + 24.0f, card.rect.y + 178.0f, card.subtitle, 180, 170, 140);
	drawText(r, card.rect.x + 24.0f, card.rect.y + 200.0f, card.hint, 140, 130, 110);
	if (selected) {
		drawTextBold(r, card.rect.x + 24.0f, card.rect.y + card.rect.h - 40.0f, "PRESS ENTER", 201, 162, 39);
	}
}

} // namespace

Bool run(GameProfileKind preselect, GameProfileKind *outChosen, SDL_Window **outWindow)
{
	if (outChosen == nullptr || outWindow == nullptr) {
		return FALSE;
	}
	*outWindow = nullptr;

	fprintf(stderr, "[GAME_SELECT] opening native profile chooser (GUI)\n");
	fflush(stderr);

	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
			fprintf(stderr, "[GAME_SELECT] FATAL: SDL_InitSubSystem VIDEO failed: %s\n", SDL_GetError());
			fflush(stderr);
			return FALSE;
		}
	}

	// Match default game resolution for seamless handoff into DXVK window.
	SDL_Window *window = SDL_CreateWindow(
		"Echelon",
		1024,
		768,
		SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		fprintf(stderr, "[GAME_SELECT] FATAL: SDL_CreateWindow failed: %s\n", SDL_GetError());
		fflush(stderr);
		return FALSE;
	}
	{
		const SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
		if (primaryDisplay != 0) {
			const int centered = SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay);
			SDL_SetWindowPosition(window, centered, centered);
		}
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
	if (renderer == nullptr) {
		fprintf(stderr, "[GAME_SELECT] FATAL: SDL_CreateRenderer failed: %s\n", SDL_GetError());
		fflush(stderr);
		SDL_DestroyWindow(window);
		return FALSE;
	}
	SDL_SetRenderVSync(renderer, 1);

	Card cards[2];
	cards[0] = {
		{ 0, 0, 0, 0 },
		"GENERALS",
		"Command & Conquer Generals",
		"Base game campaigns, skirmish, LAN",
		GAME_PROFILE_GENERALS,
		loadTexture(renderer, "generalsx_icon.png")
	};
	cards[1] = {
		{ 0, 0, 0, 0 },
		"ZERO HOUR",
		"Generals Zero Hour",
		"Expansion campaigns, generals, NGMP",
		GAME_PROFILE_ZERO_HOUR,
		loadTexture(renderer, "generalsx-zh_icon.png")
	};

	GameProfileKind focus = preselect;
	Bool running = TRUE;
	Bool confirmed = FALSE;
	int hover = -1;

	while (running) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_EVENT_QUIT) {
				running = FALSE;
				confirmed = FALSE;
			} else if (ev.type == SDL_EVENT_KEY_DOWN) {
				if (ev.key.key == SDLK_ESCAPE) {
					running = FALSE;
					confirmed = FALSE;
				} else if (ev.key.key == SDLK_LEFT || ev.key.key == SDLK_A || ev.key.key == SDLK_1) {
					focus = GAME_PROFILE_GENERALS;
				} else if (ev.key.key == SDLK_RIGHT || ev.key.key == SDLK_D || ev.key.key == SDLK_2) {
					focus = GAME_PROFILE_ZERO_HOUR;
				} else if (ev.key.key == SDLK_RETURN || ev.key.key == SDLK_KP_ENTER || ev.key.key == SDLK_SPACE) {
					confirmed = TRUE;
					running = FALSE;
				}
			} else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
				hover = -1;
				for (int i = 0; i < 2; ++i) {
					if (pointIn(cards[i].rect, (float)ev.motion.x, (float)ev.motion.y)) {
						hover = i;
						focus = cards[i].kind;
					}
				}
			} else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
				for (int i = 0; i < 2; ++i) {
					if (pointIn(cards[i].rect, (float)ev.button.x, (float)ev.button.y)) {
						focus = cards[i].kind;
						confirmed = TRUE;
						running = FALSE;
					}
				}
			}
		}

		int w = 1024;
		int h = 768;
		SDL_GetWindowSize(window, &w, &h);
		const float cardW = (float)w * 0.40f;
		const float cardH = (float)h * 0.55f;
		const float gap = (float)w * 0.04f;
		const float total = cardW * 2.0f + gap;
		const float x0 = ((float)w - total) * 0.5f;
		const float y0 = (float)h * 0.26f;
		cards[0].rect = { x0, y0, cardW, cardH };
		cards[1].rect = { x0 + cardW + gap, y0, cardW, cardH };

		// Background + top/bottom bars (MainMenu chrome language).
		fillRect(renderer, { 0, 0, (float)w, (float)h }, 10, 8, 6);
		fillRect(renderer, { 0, 0, (float)w, 64.0f }, 18, 14, 8);
		fillRect(renderer, { 0, (float)h - 48.0f, (float)w, 48.0f }, 18, 14, 8);
		drawFrame(renderer, { 0, 62.0f, (float)w, 2.0f }, 201, 162, 39, 2);
		drawFrame(renderer, { 0, (float)h - 48.0f, (float)w, 2.0f }, 201, 162, 39, 2);

		drawTextBold(renderer, 28.0f, 24.0f, "ECHELON", 201, 162, 39);
		drawText(renderer, 140.0f, 28.0f, "SELECT GAME", 210, 200, 180);
		drawText(renderer, 28.0f, (float)h - 30.0f, "1/2 OR ARROWS   ENTER   ESC=QUIT", 130, 120, 100);

		for (int i = 0; i < 2; ++i) {
			drawCard(renderer, cards[i], cards[i].kind == focus, hover == i);
		}

		SDL_RenderPresent(renderer);
	}

	for (int i = 0; i < 2; ++i) {
		if (cards[i].icon != nullptr) {
			SDL_DestroyTexture(cards[i].icon);
		}
	}
	SDL_DestroyRenderer(renderer);

	if (!confirmed) {
		SDL_DestroyWindow(window);
		fprintf(stderr, "[GAME_SELECT] cancelled\n");
		fflush(stderr);
		return FALSE;
	}

	*outChosen = focus;
	*outWindow = window;
	SDL_SetWindowTitle(window, focus == GAME_PROFILE_GENERALS
		? "Command & Conquer Generals"
		: "Command & Conquer Generals: Zero Hour");
	fprintf(stderr, "[GAME_SELECT] chose '%s' (window kept for Vulkan handoff)\n",
		focus == GAME_PROFILE_GENERALS ? "generals" : "zh");
	fflush(stderr);
	return TRUE;
}

} // namespace GameSelectUI

#endif // !_WIN32
