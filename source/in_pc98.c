/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// in_null.c -- for systems without a mouse


#include "quakedef.h"
#include "d_local.h"





void IN_Init (void)
{
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
	/*keygroup_sense(0);
	input_update_bool(inputs[ESC_KEY], (regs.h.ah & K0_ESC));
	input_update_bool(inputs[KEY1_GAME], (regs.h.ah & K0_1));
	input_update_bool(inputs[KEY2_GAME], (regs.h.ah & K0_2));
	input_update_bool(inputs[KEY3_GAME], (regs.h.ah & K0_3));
	
	keygroup_sense(1);
	input_update_bool(inputs[KEY0_GAME], (regs.h.ah & K1_0));
	
	keygroup_sense(2);
	input_update_bool(inputs[KEY_W], (regs.h.ah & K2_W));
	
	keygroup_sense(3);
	input_update_bool(inputs[CONFIRM_KEY], (regs.h.ah & K3_RETURN));
	input_update_bool(inputs[KEY_S], (regs.h.ah & K3_S));
	input_update_bool(inputs[KEY_A], (regs.h.ah & K3_A));
	input_update_bool(inputs[KEY_D], (regs.h.ah & K3_D));

	keygroup_sense(6);
	input_update_bool(inputs[SPACE_KEY], (regs.h.ah & K6_SPACE));
	
	keygroup_sense(7);
	input_update_bool(inputs[DOWN_KEY], (regs.h.ah & K7_ARROW_DOWN));
	input_update_bool(inputs[UP_KEY], (regs.h.ah & K7_ARROW_UP));
	input_update_bool(inputs[LEFT_KEY], (regs.h.ah & K7_ARROW_LEFT));
	input_update_bool(inputs[RIGHT_KEY], (regs.h.ah & K7_ARROW_RIGT));
	
	keygroup_sense(9);
	input_update_bool(inputs[NUMPAD0_KEY], (regs.h.ah & K9_NUM_0));
	input_update_bool(inputs[NUMPAD1_KEY], (regs.h.ah & K9_NUM_1));
	input_update_bool(inputs[NUMPAD2_KEY], (regs.h.ah & K9_NUM_2));*/
}

void IN_Move (usercmd_t *cmd)
{
}

