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

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include "doomgeneric.h"
#include "doomkeys.h"

//TODO: In m_misc, implement mkdir

int framebufferf, kbdf;

#define KBD_IS_PRESSED 0x80u

typedef struct __attribute__((packed)) KeyEvent {
		uint16_t scancode;
		uint8_t key;
		uint8_t character;
		uint8_t flags;
} KeyEvent;

KeyEvent eventbuf;

void DG_Init() {
	framebufferf = open("/dev/fb0", O_WRONLY);
	if(framebufferf == -1) {
		printf("Could not open framebuffer.\n");
		exit(errno);
	}

	kbdf = open("/dev/input/keyboard", O_RDONLY);
	if(kbdf == -1) {
		printf("Could not open keyboard.\n");
		exit(errno);
	}
	while(read(kbdf, &eventbuf, sizeof(KeyEvent))); //Get rid of events still in keyboarddevice buffer
}

void DG_DrawFrame() {
	write(framebufferf, DG_ScreenBuffer, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
	lseek(framebufferf, 0, SEEK_SET);
}

void DG_SleepMs(uint32_t ms) {
	//TODO
}

struct timeval timestorage;
struct timezone tzstorage;

uint32_t DG_GetTicksMs() {
	gettimeofday(&timestorage, &tzstorage);
	return timestorage.tv_usec / 1000 + timestorage.tv_sec * 1000;
}

int DG_GetKey(int* pressed, unsigned char* key) {
	if(!read(kbdf, &eventbuf, sizeof(KeyEvent))) return 0;
	*pressed = !!(eventbuf.flags & KBD_IS_PRESSED);
	switch (eventbuf.key) {
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
			*key = eventbuf.character;
			break;
	}
	return 1;
}

void DG_SetWindowTitle(const char * title) {
}
