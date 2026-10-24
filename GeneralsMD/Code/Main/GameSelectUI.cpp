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
// Runs before initMemoryManager / BIG mount so the process stays one session and one profile.
// Visual language: retail-inspired dark shell + gold accents; handoff continues into original MainMenu.

#ifndef _WIN32

#include "GameSelectUI.h"

#include <SDL3/SDL.h>

#include <cstdio>

namespace GameSelectUI
{
namespace
{

struct Card
{
	SDL_FRect rect;
	const char *title;
	const char *subtitle;
	GameProfileKind kind;
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
	// Fake weight: stamp debug text with 1px offsets (pre-FontLibrary).
	drawText(r, x, y, text, R, G, B);
	drawText(r, x + 1.0f, y, text, R, G, B);
	drawText(r, x, y + 1.0f, text, R, G, B);
}

static void drawCard(SDL_Renderer *r, const Card &card, Bool selected, Bool hovered)
{
	const Uint8 baseR = selected ? 42 : (hovered ? 36 : 28);
	const Uint8 baseG = selected ? 34 : (hovered ? 28 : 22);
	const Uint8 baseB = selected ? 18 : (hovered ? 16 : 12);
	fillRect(r, card.rect, baseR, baseG, baseB);

	if (selected) {
		drawFrame(r, card.rect, 201, 162, 39, 3);
	} else if (hovered) {
		drawFrame(r, card.rect, 160, 140, 90, 2);
	} else {
		drawFrame(r, card.rect, 90, 75, 45, 1);
	}

	drawTextBold(r, card.rect.x + 28.0f, card.rect.y + 56.0f, card.title, 230, 210, 150);
	drawText(r, card.rect.x + 28.0f, card.rect.y + 88.0f, card.subtitle, 170, 160, 130);
	if (selected) {
		drawText(r, card.rect.x + 28.0f, card.rect.y + card.rect.h - 36.0f, "ENTER / CLICK TO START", 201, 162, 39);
	}
}

} // namespace

Bool run(GameProfileKind preselect, GameProfileKind *outChosen)
{
	if (outChosen == nullptr) {
		return FALSE;
	}

	fprintf(stderr, "[GAME_SELECT] opening native profile chooser (GUI)\n");
	fflush(stderr);

	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
			fprintf(stderr, "[GAME_SELECT] FATAL: SDL_InitSubSystem VIDEO failed: %s\n", SDL_GetError());
			fflush(stderr);
			return FALSE;
		}
	}

	SDL_Window *window = SDL_CreateWindow("Echelon — Select Game", 1280, 720, SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		fprintf(stderr, "[GAME_SELECT] FATAL: SDL_CreateWindow failed: %s\n", SDL_GetError());
		fflush(stderr);
		return FALSE;
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
	cards[0] = { { 0, 0, 0, 0 }, "GENERALS", "Command & Conquer Generals", GAME_PROFILE_GENERALS };
	cards[1] = { { 0, 0, 0, 0 }, "ZERO HOUR", "Generals Zero Hour expansion", GAME_PROFILE_ZERO_HOUR };

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

		int w = 1280;
		int h = 720;
		SDL_GetWindowSize(window, &w, &h);
		const float cardW = (float)w * 0.38f;
		const float cardH = (float)h * 0.42f;
		const float gap = (float)w * 0.04f;
		const float total = cardW * 2.0f + gap;
		const float x0 = ((float)w - total) * 0.5f;
		const float y0 = (float)h * 0.32f;
		cards[0].rect = { x0, y0, cardW, cardH };
		cards[1].rect = { x0 + cardW + gap, y0, cardW, cardH };

		fillRect(renderer, { 0, 0, (float)w, (float)h }, 12, 10, 8);
		fillRect(renderer, { 0, 0, (float)w, (float)h * 0.12f }, 20, 16, 10);
		drawFrame(renderer, { 0, (float)h * 0.12f - 2.0f, (float)w, 2.0f }, 201, 162, 39, 2);
		drawTextBold(renderer, 48.0f, 28.0f, "ECHELON", 201, 162, 39);
		drawText(renderer, 48.0f, (float)h * 0.18f, "SELECT GAME", 210, 200, 180);
		drawText(renderer, 48.0f, (float)h * 0.88f, "ARROWS / 1-2  ENTER  ESC", 120, 110, 90);

		for (int i = 0; i < 2; ++i) {
			drawCard(renderer, cards[i], cards[i].kind == focus, hover == i);
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	if (!confirmed) {
		fprintf(stderr, "[GAME_SELECT] cancelled\n");
		fflush(stderr);
		return FALSE;
	}

	*outChosen = focus;
	fprintf(stderr, "[GAME_SELECT] chose '%s'\n", focus == GAME_PROFILE_GENERALS ? "generals" : "zh");
	fflush(stderr);
	return TRUE;
}

} // namespace GameSelectUI

#endif // !_WIN32
