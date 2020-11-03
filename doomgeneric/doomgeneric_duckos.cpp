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
#include <libpond/pond.h>

PWindow* window;
PContext* pond;
struct keyboard_event key_queue[16];
int key_queue_insert_index = 0;
int key_queue_dequeue_index = 0;

extern "C" void DG_Init() {
	pond = PContext::init();
	if(!pond)
		exit(-1);

	window = pond->create_window(nullptr, 10, 10, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
	if(!window)
		exit(-1);

	free(DG_ScreenBuffer);
	DG_ScreenBuffer = window->framebuffer.data;
}

extern "C" void DG_DrawFrame() {
	window->invalidate();
	while(pond->has_event()) {
		PEvent evt = pond->next_event();
		switch(evt.type) {
			case PEVENT_WINDOW_DESTROY:
				exit(0);
			case PEVENT_KEY: {
				if(key_queue_insert_index >= 16)
					break;
				struct keyboard_event* kevt = &key_queue[key_queue_insert_index++];
				kevt->key = evt.key.key;
				kevt->character = evt.key.character;
				kevt->scancode = evt.key.scancode;
				kevt->modifiers = evt.key.modifiers;
				break;
			}
			default:
				break;
		}
	}
}

extern "C" void DG_SleepMs(uint32_t ms) {
	//TODO
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
