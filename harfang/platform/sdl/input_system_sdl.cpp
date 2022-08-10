// HARFANG(R) Copyright (C) 2019 Emmanuel Julien, Movida Production. Released under GPL/LGPL/Commercial Licence, see licence.txt for details.
#include "platform/sdl/input_system_sdl.h"
#include "platform/input_system.h"
#include "foundation/cext.h"
#include "foundation/log.h"
#include "foundation/format.h"
#include "foundation/utf8.h"
#include "platform/window_system.h"
#include <SDL.h>
#include <array>
#include <memory>
#include <map>

namespace hg {

static double wheel = 0, hwheel = 0;
static int inhibit_click = 0;

MouseState previous_state;

static MouseState ReadMouse() {
	MouseState state = previous_state;

	state.wheel = 0;
	int w, h;

	auto win = GetWindowInFocus();
	if (!GetWindowClientSize(win, w, h)) {
		inhibit_click = 3;
		return {};
	}

	// ...and update it
	SDL_Event event;
	while (SDL_PollEvent(&event))
		switch (event.type) {
			case SDL_FINGERMOTION: {
				// use only if there is one finger
				if (SDL_GetTouchFinger(event.tfinger.touchId, 1))
					break;
				state.x = float(event.tfinger.x) * w;
				state.y = float(1.f - event.tfinger.y) * h;
			} break;

			case SDL_FINGERDOWN: {
				// use only if there is one finger
				if (SDL_GetTouchFinger(event.tfinger.touchId, 1))
					break;
				state.button[0] = true;
				state.x = float(event.tfinger.x) * w;
				state.y = float(1.f - event.tfinger.y) * h;
			} break;

			case SDL_MULTIGESTURE: // use this for pinch
			{
				if (fabs(event.mgesture.dDist) > 0.002f) {
					//Pinch open
					if (event.mgesture.dDist > 0) {
						state.wheel = 1;
					}
					//Pinch close
					else {
						state.wheel = -1;
					}
				}
			} break;

			case SDL_FINGERUP:
				// use only if there is one finger
				if (SDL_GetTouchFinger(event.tfinger.touchId, 1))
					break;
				state.button[0] = false;
				break;
				
			case SDL_MOUSEMOTION: {
				state.x = float(event.motion.x);
				state.y = float(h - event.motion.y);
			} break;

			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
					case SDL_BUTTON_LEFT:
						state.button[0] = (event.button.state == SDL_PRESSED);
						break;
					case SDL_BUTTON_MIDDLE:
						state.button[2] = (event.button.state == SDL_PRESSED);
						break;
					case SDL_BUTTON_RIGHT:
						state.button[1] = (event.button.state == SDL_PRESSED);
						break;
				}
				break;

			case SDL_MOUSEWHEEL:
				if (event.wheel.y >= 1) // scroll up
					state.wheel = 1;
				else if (event.wheel.y <= -1) // scroll down
					state.wheel = -1;
				break;
		}

	previous_state = state;
	return state;
}

// Keyboard

static KeyboardState ReadKeyboard() {
	auto handle = GetWindowHandle(GetWindowInFocus());
	if (!handle)
		return {};

	KeyboardState state;

	// ...and update it.
	const Uint8 *keys = SDL_GetKeyboardState(nullptr);

	state.key[K_Up] = keys[SDL_SCANCODE_UP];
	state.key[K_Down] = keys[SDL_SCANCODE_DOWN];
	state.key[K_Left] = keys[SDL_SCANCODE_LEFT];
	state.key[K_Right] = keys[SDL_SCANCODE_RIGHT];

	state.key[K_Escape] = keys[SDL_SCANCODE_ESCAPE];

	state.key[K_Add] = keys[SDL_SCANCODE_KP_PLUS];
	state.key[K_Sub] = keys[SDL_SCANCODE_KP_MINUS];
	state.key[K_Mul] = keys[SDL_SCANCODE_KP_MULTIPLY];
	state.key[K_Div] = keys[SDL_SCANCODE_KP_DIVIDE];
	state.key[K_Enter] = keys[SDL_SCANCODE_KP_ENTER];

	state.key[K_PrintScreen] = keys[SDL_SCANCODE_PRINTSCREEN];
	state.key[K_ScrollLock] = keys[SDL_SCANCODE_SCROLLLOCK];
	state.key[K_Pause] = keys[SDL_SCANCODE_PAUSE];
	state.key[K_NumLock] = keys[SDL_SCANCODE_NUMLOCKCLEAR];
	state.key[K_Return] = keys[SDL_SCANCODE_RETURN];

	state.key[K_LShift] = keys[SDL_SCANCODE_LSHIFT];
	state.key[K_RShift] = keys[SDL_SCANCODE_RSHIFT];
	state.key[K_LCtrl] = keys[SDL_SCANCODE_LCTRL];
	state.key[K_RCtrl] = keys[SDL_SCANCODE_RCTRL];
	state.key[K_LAlt] = keys[SDL_SCANCODE_LALT];
	state.key[K_RAlt] = keys[SDL_SCANCODE_RALT];
	state.key[K_LWin] = keys[SDL_SCANCODE_LGUI];
	state.key[K_RWin] = keys[SDL_SCANCODE_RGUI];

	state.key[K_Tab] = keys[SDL_SCANCODE_TAB];
	state.key[K_CapsLock] = keys[SDL_SCANCODE_CAPSLOCK];
	state.key[K_Space] = keys[SDL_SCANCODE_SPACE];
	state.key[K_Backspace] = keys[SDL_SCANCODE_BACKSPACE];
	state.key[K_Insert] = keys[SDL_SCANCODE_INSERT];
	state.key[K_Suppr] = keys[SDL_SCANCODE_DELETE];
	state.key[K_Home] = keys[SDL_SCANCODE_HOME];
	state.key[K_End] = keys[SDL_SCANCODE_END];
	state.key[K_PageUp] = keys[SDL_SCANCODE_PAGEUP];
	state.key[K_PageDown] = keys[SDL_SCANCODE_PAGEDOWN];

	state.key[K_F1] = keys[SDL_SCANCODE_F1];
	state.key[K_F2] = keys[SDL_SCANCODE_F2];
	state.key[K_F3] = keys[SDL_SCANCODE_F3];
	state.key[K_F4] = keys[SDL_SCANCODE_F4];
	state.key[K_F5] = keys[SDL_SCANCODE_F5];
	state.key[K_F6] = keys[SDL_SCANCODE_F6];
	state.key[K_F7] = keys[SDL_SCANCODE_F7];
	state.key[K_F8] = keys[SDL_SCANCODE_F8];
	state.key[K_F9] = keys[SDL_SCANCODE_F9];
	state.key[K_F10] = keys[SDL_SCANCODE_F10];
	state.key[K_F11] = keys[SDL_SCANCODE_F11];
	state.key[K_F12] = keys[SDL_SCANCODE_F12];

	state.key[K_Numpad0] = keys[SDL_SCANCODE_KP_0];
	state.key[K_Numpad1] = keys[SDL_SCANCODE_KP_1];
	state.key[K_Numpad2] = keys[SDL_SCANCODE_KP_2];
	state.key[K_Numpad3] = keys[SDL_SCANCODE_KP_3];
	state.key[K_Numpad4] = keys[SDL_SCANCODE_KP_4];
	state.key[K_Numpad5] = keys[SDL_SCANCODE_KP_5];
	state.key[K_Numpad6] = keys[SDL_SCANCODE_KP_6];
	state.key[K_Numpad7] = keys[SDL_SCANCODE_KP_7];
	state.key[K_Numpad8] = keys[SDL_SCANCODE_KP_8];
	state.key[K_Numpad9] = keys[SDL_SCANCODE_KP_9];

	state.key[K_A] = keys[SDL_SCANCODE_A];
	state.key[K_B] = keys[SDL_SCANCODE_B];
	state.key[K_C] = keys[SDL_SCANCODE_C];
	state.key[K_D] = keys[SDL_SCANCODE_D];
	state.key[K_E] = keys[SDL_SCANCODE_E];
	state.key[K_F] = keys[SDL_SCANCODE_F];
	state.key[K_G] = keys[SDL_SCANCODE_G];
	state.key[K_H] = keys[SDL_SCANCODE_H];
	state.key[K_I] = keys[SDL_SCANCODE_I];
	state.key[K_J] = keys[SDL_SCANCODE_J];
	state.key[K_K] = keys[SDL_SCANCODE_K];
	state.key[K_L] = keys[SDL_SCANCODE_L];
	state.key[K_M] = keys[SDL_SCANCODE_M];
	state.key[K_N] = keys[SDL_SCANCODE_N];
	state.key[K_O] = keys[SDL_SCANCODE_O];
	state.key[K_P] = keys[SDL_SCANCODE_P];
	state.key[K_Q] = keys[SDL_SCANCODE_Q];
	state.key[K_R] = keys[SDL_SCANCODE_R];
	state.key[K_S] = keys[SDL_SCANCODE_S];
	state.key[K_T] = keys[SDL_SCANCODE_T];
	state.key[K_U] = keys[SDL_SCANCODE_U];
	state.key[K_V] = keys[SDL_SCANCODE_V];
	state.key[K_W] = keys[SDL_SCANCODE_W];
	state.key[K_X] = keys[SDL_SCANCODE_X];
	state.key[K_Y] = keys[SDL_SCANCODE_Y];
	state.key[K_Z] = keys[SDL_SCANCODE_Z];
	return state;
}


static const char *GetKeyName(Key key) {
	static std::map<Key, int> key_to_sdl = {
		{K_LShift, SDL_SCANCODE_LSHIFT},
		{K_RShift, SDL_SCANCODE_RSHIFT}/*,
		{K_LCtrl, GLFW_KEY_LEFT_CONTROL},
		{K_RCtrl, GLFW_KEY_RIGHT_CONTROL},
		{K_LAlt, GLFW_KEY_LEFT_ALT},
		{K_RAlt, GLFW_KEY_RIGHT_ALT},
		{K_LWin, GLFW_KEY_LEFT_SUPER},
		{K_RWin, GLFW_KEY_RIGHT_SUPER},
		{K_Tab, GLFW_KEY_TAB},
		{K_CapsLock, GLFW_KEY_CAPS_LOCK},
		{K_Space, GLFW_KEY_SPACE},
		{K_Backspace, GLFW_KEY_BACKSPACE},
		{K_Insert, GLFW_KEY_INSERT},
		{K_Suppr, GLFW_KEY_DELETE},
		{K_Home, GLFW_KEY_HOME},
		{K_End, GLFW_KEY_END},
		{K_PageUp, GLFW_KEY_PAGE_UP},
		{K_PageDown, GLFW_KEY_PAGE_DOWN},
		{K_Up, GLFW_KEY_UP},
		{K_Down, GLFW_KEY_DOWN},
		{K_Left, GLFW_KEY_LEFT},
		{K_Right, GLFW_KEY_RIGHT},
		{K_Escape, GLFW_KEY_ESCAPE},
		{K_F1, GLFW_KEY_F1},
		{K_F2, GLFW_KEY_F2},
		{K_F3, GLFW_KEY_F3},
		{K_F4, GLFW_KEY_F4},
		{K_F5, GLFW_KEY_F5},
		{K_F6, GLFW_KEY_F6},
		{K_F7, GLFW_KEY_F7},
		{K_F8, GLFW_KEY_F8},
		{K_F9, GLFW_KEY_F9},
		{K_F10, GLFW_KEY_F10},
		{K_F11, GLFW_KEY_F11},
		{K_F12, GLFW_KEY_F12},
		{K_PrintScreen, GLFW_KEY_PRINT_SCREEN},
		{K_ScrollLock, GLFW_KEY_SCROLL_LOCK},
		{K_Pause, GLFW_KEY_PAUSE},
		{K_NumLock, GLFW_KEY_NUM_LOCK},
		{K_Return, GLFW_KEY_ENTER},
		{K_0, GLFW_KEY_0},
		{K_1, GLFW_KEY_1},
		{K_2, GLFW_KEY_2},
		{K_3, GLFW_KEY_3},
		{K_4, GLFW_KEY_4},
		{K_5, GLFW_KEY_5},
		{K_6, GLFW_KEY_6},
		{K_7, GLFW_KEY_7},
		{K_8, GLFW_KEY_8},
		{K_9, GLFW_KEY_9},
		{K_Numpad0, GLFW_KEY_KP_0},
		{K_Numpad1, GLFW_KEY_KP_1},
		{K_Numpad2, GLFW_KEY_KP_2},
		{K_Numpad3, GLFW_KEY_KP_3},
		{K_Numpad4, GLFW_KEY_KP_4},
		{K_Numpad5, GLFW_KEY_KP_5},
		{K_Numpad6, GLFW_KEY_KP_6},
		{K_Numpad7, GLFW_KEY_KP_7},
		{K_Numpad8, GLFW_KEY_KP_8},
		{K_Numpad9, GLFW_KEY_KP_9},
		{K_Add, GLFW_KEY_KP_ADD},
		{K_Sub, GLFW_KEY_KP_SUBTRACT},
		{K_Mul, GLFW_KEY_KP_MULTIPLY},
		{K_Div, GLFW_KEY_KP_DIVIDE},
		{K_Enter, GLFW_KEY_KP_ENTER},
		{K_A, GLFW_KEY_A},
		{K_B, GLFW_KEY_B},
		{K_C, GLFW_KEY_C},
		{K_D, GLFW_KEY_D},
		{K_E, GLFW_KEY_E},
		{K_F, GLFW_KEY_F},
		{K_G, GLFW_KEY_G},
		{K_H, GLFW_KEY_H},
		{K_I, GLFW_KEY_I},
		{K_J, GLFW_KEY_J},
		{K_K, GLFW_KEY_K},
		{K_L, GLFW_KEY_L},
		{K_M, GLFW_KEY_M},
		{K_N, GLFW_KEY_N},
		{K_O, GLFW_KEY_O},
		{K_P, GLFW_KEY_P},
		{K_Q, GLFW_KEY_Q},
		{K_R, GLFW_KEY_R},
		{K_S, GLFW_KEY_S},
		{K_T, GLFW_KEY_T},
		{K_U, GLFW_KEY_U},
		{K_V, GLFW_KEY_V},
		{K_W, GLFW_KEY_W},
		{K_X, GLFW_KEY_X},
		{K_Y, GLFW_KEY_Y},
		{K_Z, GLFW_KEY_Z},
		{K_Plus, GLFW_KEY_EQUAL},
		{K_Comma, GLFW_KEY_COMMA},
		{K_Minus, GLFW_KEY_MINUS},
		{K_Period, GLFW_KEY_PERIOD},*/
	};

	const auto i = key_to_sdl.find(key);
	if (i != std::end(key_to_sdl))
		return "yo";// glfwGetKeyName(i->second, -1);

	return nullptr;
}


static Signal<void(const Window *)>::Connection on_new_window_connection;

void InputInit() {
	AddMouseReader("default", ReadMouse);
	AddKeyboardReader("default", ReadKeyboard, GetKeyName);
}

void InputShutdown() { new_window_signal.Disconnect(on_new_window_connection); }

} // namespace hg
