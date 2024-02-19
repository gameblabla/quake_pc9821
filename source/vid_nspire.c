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
// vid_null.c -- null video driver to aid porting efforts
#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include "quakedef.h"
#include "d_local.h"

extern viddef_t	vid;				// global video state

#define	BASEWIDTH	640
#define	BASEHEIGHT	400

short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[SURFCACHE_SIZE_AT_320X200];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];


SDL_Surface* screen;

void	VID_SetPalette (unsigned char *palette)
{
    int32_t		i;
    SDL_Color colors[256];

    for ( i=0; i<256; ++i )
    {
        colors[i].r = *palette++;
        colors[i].g = *palette++;
        colors[i].b = *palette++;
    }
    SDL_SetColors(screen, colors, 0, 256);
}

void	VID_ShiftPalette (unsigned char *palette)
{
    VID_SetPalette(palette);
}

void	VID_Init (unsigned char *palette)
{

	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(BASEWIDTH, BASEHEIGHT, 8, SDL_HWSURFACE|SDL_HWPALETTE);
	
	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;

    vid.colormap = host_colormap;
    vid.fullbright = 256 - LittleLong (*((int32_t		*)vid.colormap + 2048));
	vid.buffer = screen->pixels;
	vid.rowbytes = screen->pitch;
    vid.conbuffer = vid.buffer;
    vid.conrowbytes = vid.rowbytes;
    vid.direct = 1;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));

	
	//initBuffering();
}

void	VID_Shutdown (void)
{

}

void	VID_Update (vrect_t *rects)
{
	SDL_Flip(screen);
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}


