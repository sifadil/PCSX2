/*  OnePAD - author: arcum42(@gmail.com)
 *  Copyright (C) 2009
 *
 *  Based on ZeroPAD, author zerofrog@gmail.com
 *  Copyright (C) 2006-2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "onepad.h"
#include "controller.h"

HatPins hat_position = {false, false, false, false};

__forceinline void set_keyboad_key(int pad, int keysym, int index)
{
	conf.keysym_map[pad][keysym] = index;
}

__forceinline int get_keyboard_key(int pad, int keysym)
{
	// You must use find instead of []
	// [] will create an element if the key does not exist and return 0
	map<u32,u32>::iterator it = conf.keysym_map[pad].find(keysym);
	if (it != conf.keysym_map[pad].end())
		return it->second;
	else
		return -1;
}

__forceinline void set_key(int pad, int index, int value)
{
	conf.keys[pad][index] = value;
}

__forceinline int get_key(int pad, int index)
{
	return conf.keys[pad][index];
}

__forceinline KeyType type_of_key(int pad, int index)
 {
	 int key = get_key(pad, index);

	if (key < 0x10000) return PAD_KEYBOARD;
	else if (key >= 0x10000 && key < 0x20000) return PAD_JOYBUTTONS;
	else if (key >= 0x20000 && key < 0x30000) return PAD_JOYSTICK;
	else if (key >= 0x30000 && key < 0x40000) return PAD_POV;
	else if (key >= 0x40000 && key < 0x50000) return PAD_HAT;
	else if (key >= 0x50000 && key < 0x60000) return PAD_MOUSE;
	else return PAD_NULL;
 }

//*******************************************************
//			onepad key -> input
//*******************************************************
// keyboard

// joystick
__forceinline int key_to_button(int pad, int index)
{
	return (get_key(pad, index) & 0xff);
}

__forceinline int key_to_axis(int pad, int index)
{
	return (get_key(pad, index) & 0xff);
}

__forceinline int key_to_pov_sign(int pad, int index)
{
	return ((get_key(pad, index) & 0x100) >> 8);
}

__forceinline int key_to_hat_dir(int pad, int index)
{
	return ((get_key(pad, index) & 0xF00) >> 8);
}

// mouse
__forceinline int key_to_mouse(int pad, int index)
{
	return (get_key(pad, index) & 0xff);
}

//*******************************************************
//			input -> onepad key
//*******************************************************
// keyboard ???
#if 0
__forceinline int pad_to_key(int pad, int index)
{
	return (get_key(pad, index) & 0xffff);
}
#endif

// joystick
__forceinline int button_to_key(int button_id)
{
	return (0x10000 | button_id);
}

__forceinline int joystick_to_key(int axis_id)
{
	return (0x20000 | axis_id);
}

__forceinline int pov_to_key(int sign, int axis_id)
{
	return (0x30000 | ((sign) << 8) | (axis_id));
}

__forceinline int hat_to_key(int dir, int axis_id)
{
	return (0x40000 | ((dir) << 8) | (axis_id));
}

#if 0
// mouse
__forceinline int mouse_to_key(int button_id)
{
	return (0x50000 | button_id);
}
#endif
