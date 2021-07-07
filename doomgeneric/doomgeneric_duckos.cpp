/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <cstdio>
#include <fcntl.h>
#include <sys/time.h>
#include "doomgeneric.h"
#include "doomkeys.h"
#include <sys/input.h>
#include <libui/libui.h>
#include <unistd.h>


int key_queue_insert_index = 0;
int key_queue_dequeue_index = 0;
struct keyboard_event key_queue[16];
bool key_states[256] = {false};

class DoomWidget: public UI::Widget {
public:

	DoomWidget() {
	}

	//Widget
	Dimensions preferred_size() override {
		return {DOOMGENERIC_RESX, DOOMGENERIC_RESY};
	}

	void do_repaint(const UI::DrawContext& ctx) override {
		Image fb(DG_ScreenBuffer, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
		ctx.framebuffer().copy(fb, {0, 0, DOOMGENERIC_RESX, DOOMGENERIC_RESY}, {0, 0});
	}

	bool on_keyboard(Pond::KeyEvent evt) override {
		if(key_queue_insert_index >= 16)
			return false;
		//Don't insert multiple events for a held-down key
		bool pressed = KBD_ISPRESSED(evt);
		if(key_states[evt.key] && pressed)
			return true;
		key_states[evt.key] = pressed;
		struct keyboard_event* kevt = &key_queue[key_queue_insert_index++];
		kevt->key = evt.key;
		kevt->character = evt.character;
		kevt->scancode = evt.scancode;
		kevt->modifiers = evt.modifiers;
		return true;
	}
};

DoomWidget* widget;
UI::Window* window;

extern "C" void DG_Init() {
	UI::init(NULL, NULL);

	window = UI::Window::create();
	widget = new DoomWidget;
	window->set_contents(widget);
	window->show();
}

extern "C" void DG_DrawFrame() {
	widget->repaint();
}

extern "C" void DG_SleepMs(uint32_t ms) {
	usleep(ms * 1000);
}

extern "C" void DG_Update() {
	UI::update(0);
}

struct timeval timestorage;
struct timezone tzstorage;

extern "C" uint32_t DG_GetTicksMs() {
	gettimeofday(&timestorage, &tzstorage);
	return timestorage.tv_usec / 1000 + timestorage.tv_sec * 1000;
}

extern "C" int DG_GetKey(int* pressed, unsigned char* key) {
	if(key_queue_insert_index == 0) {
		return 0;
	}

	struct keyboard_event evt = key_queue[key_queue_dequeue_index++];
	if(key_queue_dequeue_index == key_queue_insert_index) {
		key_queue_dequeue_index = 0;
		key_queue_insert_index = 0;
	}

	*pressed = KBD_ISPRESSED(evt);

	switch (evt.key) {
		case 0x1C:
			*key = KEY_ENTER;
			break;
		case 0x01:
			*key = KEY_ESCAPE;
			break;
		case 0x4B:
			*key = KEY_LEFTARROW;
			break;
		case 0x4D:
			*key = KEY_RIGHTARROW;
			break;
		case 0x48:
			*key = KEY_UPARROW;
			break;
		case 0x50:
			*key = KEY_DOWNARROW;
			break;
		case 0x1D:
			*key = KEY_FIRE;
			break;
		case 0x39:
			*key = KEY_USE;
			break;
		case 0x2a:
			*key = KEY_RSHIFT;
			break;
		default:
			*key = evt.character;
			break;
	}
	return 1;
}

extern "C" void DG_SetWindowTitle(const char * title) {
	window->set_title(title);
}
