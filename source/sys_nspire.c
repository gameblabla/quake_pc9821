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
// sys_null.h -- null system driver to aid porting efforts
#include <stdint.h>
#include <stdio.h>

#ifdef DJGPP
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dir.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <conio.h>
#endif

#include "quakedef.h"
#include "errno.h"

#ifdef PC98
#include "inputpc98.h"
#include "keyspc98.h"

uint8_t inputs[19] = {0};

#define input_func_bool(var) { var = 1; } else { var = 0; }

#define input_update_bool(var, cur_sensed) \
	if(cur_sensed) input_func_bool(var)
	
static __dpmi_regs regs;

static void keygroup_sense(char group_id)
{
    regs.h.ah = 0x04;
    regs.h.al = group_id;
    __dpmi_int(0x18, &regs);
}
#endif

static quakeparms_t    parms;
qboolean			isDedicated = false;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
	mkdir (path, 0777);
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/


void Sys_Init( void )
{

}


void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (char *error, ...)
{
	va_list         argptr;
	char rgc_errorstr[ 256 ];

	va_start (argptr,error);
	vsprintf (rgc_errorstr, error,argptr);
	va_end (argptr);

	printf("Sys Error:\n");
	printf( rgc_errorstr );
	printf("\n");

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr); 
}

void Sys_Quit (void)
{
	int i;

	Host_Shutdown();

	for( i = 0; i < MAX_HANDLES; i++ )
	{
		if( sys_handles[ i ] )
		{
			fclose( sys_handles[ i ] );
			sys_handles[ i ] = NULL;
		}
	}
	if( parms.membase )
	{
		free( parms.membase );
	}

	exit (0);
}

double Sys_FloatTime (void)
{
	static double t;

	t += 0.05;
	
	return t;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}

void Sys_SendKeyEvents (void)
{

}
#if !defined(id386)
void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}
#endif

void Sys_NSpireInput( void )
{
#ifdef PC98
	int i_idx;
	keygroup_sense(0);
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
	input_update_bool(inputs[NUMPAD2_KEY], (regs.h.ah & K9_NUM_2));
	
	Key_Event( K_ESCAPE, false );
	Key_Event( K_ENTER, false );
	Key_Event( K_SPACE, false );
	Key_Event( K_LEFTARROW, false );
	Key_Event( K_RIGHTARROW, false );
	Key_Event( K_UPARROW, false );
	Key_Event( K_DOWNARROW, false );
	Key_Event( K_CTRL, false );
	Key_Event( K_SHIFT, false );
	
	if(inputs[ESC_KEY])
	{
		Key_Event( K_ESCAPE, true );
	}
	
	if(inputs[CONFIRM_KEY])
	{
		Key_Event( K_ENTER, true );
	}
	
	if(inputs[SPACE_KEY])
	{
		Key_Event( K_SPACE, true );
	}
	
	if(inputs[NUMPAD0_KEY])
	{
		Key_Event( K_CTRL, true );
	}
	
	if(inputs[NUMPAD1_KEY])
	{
		Key_Event( K_SHIFT, true );
	}

	if(inputs[LEFT_KEY] || inputs[KEY_A])
	{
		Key_Event( K_LEFTARROW, true );
	}
	if(inputs[RIGHT_KEY] || inputs[KEY_D])
	{
		Key_Event( K_RIGHTARROW, true );
	}
	if(inputs[UP_KEY] || inputs[KEY_W])
	{
		Key_Event( K_UPARROW, true );
	}
	if(inputs[DOWN_KEY] || inputs[KEY_S])
	{
		Key_Event( K_DOWNARROW, true );
	}
#else

#endif
}

//=============================================================================

int main (int argc, char **argv)
{
	char *pc_basedir_term;
	unsigned int i = 0;
	double f64_start, f64_end, f64_delta;
	static  char    cwd[1024];


#if id386
	printf("Quake rwill TINSPIRE fork v2, 386 optimized\n" );
#else
	printf("Quake rwill TINSPIRE fork v2\n" );
#endif
	Sys_Init();
	parms.memsize = 6*1024*1024;
	
	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";

	if( !parms.membase )
	{
		Sys_Error( "could not alloc %.2f kb of memory\n", ( ( double )parms.memsize ) / 1024.0 );
	}

#ifdef DJGPP
	getwd (cwd);
	if (cwd[Q_strlen(cwd)-1] == '/') cwd[Q_strlen(cwd)-1] = 0;
	parms.basedir = cwd; //"f:/quake";
#else
	pc_basedir_term = argv[ 0 ] + Q_strlen( argv[ 0 ] );

	while( pc_basedir_term >= argv[ 0 ] )
	{
		if( *pc_basedir_term == '/' || *pc_basedir_term == '\\' )
		{
			*pc_basedir_term = 0;
			break;
		}
		pc_basedir_term--;
	}
	parms.basedir = argv[ 0 ];
#endif

	memset(parms.membase, 0, parms.memsize);

	printf("membase: %p\n", parms.membase );
	printf("argc: %d\n", argc );

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	printf ("Chained..\n");
#ifndef WIN32
	/*bkpt();*/
#endif
	Host_Init (&parms);
	f64_start = f64_end = Sys_FloatTime();
	while (1)
	{
		f64_delta = f64_end - f64_start;
		Sys_NSpireInput();
		Host_Frame ( f64_delta );
		f64_start = f64_end;
		f64_end = Sys_FloatTime();
	}
	
	return 0;
}

