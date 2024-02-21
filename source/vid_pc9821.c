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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#define DJGPP 1
#ifdef DJGPP
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#endif
#include "pc98.h"
#include "gdc.h"
#include <stdint.h>
#include <stdio.h>
#include "quakedef.h"
#include "d_local.h"

extern viddef_t	vid;				// global video state

#define	BASEWIDTH	640
#define	BASEHEIGHT	400

static uint16_t screen_width, screen_height;

#ifdef DJGPP
// Doing so greatly reduces size
#include <crt0.h>
char **__crt0_glob_function (char *arg) { return 0; }
void   __crt0_load_environment_file (char *progname) { }
void   __crt0_setup_arguments (void) { }
#endif

static int pegc_fb_location = PEGC_FB_LOCATION_LOW;
#ifdef DJGPP
static __dpmi_meminfo	vram_dpmi;			// DPMI descriptor for far-memory location of framebuffer
#endif
static int vram_dpmi_selector;	// Memory region selector handle

#ifdef DJGPP
#define OUTPORT_DOS outportb
#define INPORT_DOS inportb
#else
#define OUTPORT_DOS _outp
#define INPORT_DOS _inp
#endif

#define ENABLE_NEAR() __djgpp_nearptr_enable();
#define DISABLE_NEAR() __djgpp_nearptr_disable();

/* These are used for hardware double buffering using the second plane */
#define PAGE_OFFSET_1 0
#define PAGE_OFFSET_2 PEGC_FB_SIZE/2
#define PAGE_TODRAW PAGE_OFFSET_2
#define PAGE_BUFFER_ID_0 0
#define PAGE_BUFFER_ID_1 1
const unsigned page_to_draw_to[2] = {PAGE_OFFSET_1, PAGE_OFFSET_2};
static uint8_t front_buffer_page = PAGE_BUFFER_ID_0;
static uint8_t back_buffer_page = PAGE_BUFFER_ID_1;

uint8_t*	basebuffer;
unsigned char* lowmem;
unsigned char* ScreenMem;

short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[SURFCACHE_SIZE_AT_320X200];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];


static inline int_fast8_t gfx_DPMI()
{
	// Enable DPMI mapper to framebuffer far location
	if (INPORT_DOS(MEMORY_HOLE_CHECK_ADDR) & 0x04){
		/* Memory hole not available, relocate VRAM address */
		pegc_fb_location = PEGC_FB_LOCATION_HIGH;
	} else {
		/* Memory hole available */
		pegc_fb_location = PEGC_FB_LOCATION_LOW;
	}

	#ifdef DJGPP
	if (!__djgpp_nearptr_enable()){
		return -1;
	}
	
	vram_dpmi.address = pegc_fb_location;
	vram_dpmi.size    = PEGC_FB_SIZE;
	
	if (__dpmi_physical_address_mapping(&vram_dpmi) != 0){
		__djgpp_nearptr_disable();
		return -1;
	}
	vram_dpmi_selector = __dpmi_allocate_ldt_descriptors(1);
	if (vram_dpmi_selector < 0){
		__dpmi_free_physical_address_mapping(&vram_dpmi);
		__djgpp_nearptr_disable();
		return -1;
	}
	__dpmi_set_segment_base_address(vram_dpmi_selector, vram_dpmi.address);
	__dpmi_set_segment_limit(vram_dpmi_selector, vram_dpmi.size - 1);
	#endif
	
	lowmem = (unsigned char*)__djgpp_conventional_base;
	ScreenMem = lowmem+pegc_fb_location;
	
	#ifdef DJGPP
	//__djgpp_nearptr_disable();
	#endif
	
	return 0;
}

static void Print_text(const char *str)
{
	union REGS regs;
	regs.h.cl = 0x10;
	regs.h.ah = 0;
	while(*str) {
		regs.h.dl = *(str++);
		int86(0xDC, &regs, &regs);
	}
	outp(0x64, 0); // VSync interrupt trigger	
}


void	VID_SetPalette (unsigned char *palette)
{
	unsigned int i;
	for(i=0;i<256;i++)
	{
		OUTPORT_DOS(PEGC_PALLETE_SEL_ADDR, i);
		OUTPORT_DOS(PEGC_RED_ADDR, *palette++);
		OUTPORT_DOS(PEGC_GREEN_ADDR, *palette++);
		OUTPORT_DOS(PEGC_BLUE_ADDR, *palette++);
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
    VID_SetPalette(palette);
}

#define ClearTextVRAM()  Print_text("\x1B[2J");

void	VID_Init (unsigned char *palette)
{
	ClearTextVRAM();
	
	// HSYNC 24 KHz (640x400)
	OUTPORT_DOS(PEGC_SCANFREQ_ADDR, PEGC_SCANFREQ_24);

	// 256 color mode
	OUTPORT_DOS(PEGC_MODE_ADDR, 0x07);
	OUTPORT_DOS(PEGC_MODE_ADDR, PEGC_BPPMODE_256c);
	OUTPORT_DOS(PEGC_MODE_ADDR, PEGC_BPPMODE_DUAL_PAGE);
	OUTPORT_DOS(PEGC_MODE_ADDR, 0x06);
	
	// Enable Packed Pixel mode
	#ifdef DJGPP
	_farpokeb(_dos_ds, PEGC_PIXELMODE_ADDR, PEGC_PIXELMODE_PACKED);
	#endif

	// Set up DPMI mapper for vram framebuffer regionbui
	if (gfx_DPMI() < 0){
		return 1;	
	}
	
	// Enable linear framebuffer at 16MB and 4095MB
	#ifdef DJGPP
	_farpokeb(_dos_ds, PEGC_FB_CONTROL_ADDR, PEGC_FB_ON);
	#endif

	// Set screen 0 to be active for drawing and active for display
	OUTPORT_DOS(PEGC_DRAW_SCREEN_SEL_ADDR, 0x00);
	OUTPORT_DOS(PEGC_DISP_SCREEN_SEL_ADDR, 0x00);
	
	// Graphics mode on
	OUTPORT_DOS(PEGC_GDC_COMMAND_ADDR, GDC_COMMAND_START);
	
	// Text mode off
	OUTPORT_DOS(0x62, GDC_COMMAND_STOP1);

	basebuffer = malloc (BASEHEIGHT*BASEWIDTH);
	
	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;

    vid.colormap = host_colormap;
    vid.fullbright = 256 - LittleLong (*((int32_t		*)vid.colormap + 2048));
	//vid.buffer = lowmem + pegc_fb_location;
	vid.buffer = basebuffer;
	vid.rowbytes = BASEWIDTH;
    vid.conbuffer = vid.buffer;
    vid.conrowbytes = vid.rowbytes;
    vid.direct = 1;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));

	
	//initBuffering();
}

void	VID_Shutdown (void)
{
	free(basebuffer);
	
	#ifdef DJGPP
	_farpokeb(_dos_ds, PEGC_FB_CONTROL_ADDR, PEGC_FB_OFF);
	#endif

	// 16 Color mode
	OUTPORT_DOS(PEGC_MODE_ADDR, 0x07);
	OUTPORT_DOS(PEGC_MODE_ADDR, PEGC_BPPMODE_16c);
	OUTPORT_DOS(PEGC_MODE_ADDR, 0x06);

	// Graphics mode off
	OUTPORT_DOS(PEGC_GDC_COMMAND_ADDR, GDC_COMMAND_STOP1);
	
	#ifdef DJGPP
	// Free DPMI mapping
	__dpmi_free_physical_address_mapping(&vram_dpmi);
	#endif
	
	//  Clear anything we did to the screen
	ClearTextVRAM();
	
	// Text mode on
	OUTPORT_DOS(0x62, GDC_COMMAND_START);

	#ifdef DJGPP
	__djgpp_nearptr_disable();
	#endif
	
	// Print command to enable text cursor
	Print_text("\x1B[>5l");
	// Print command to enable system line
	Print_text("\x1B[>1l");
}

static inline void memcpy64(void *dest, void *src, size_t count)
{
	uint64_t *tmp = dest;
	uint64_t *s = src;
	while (count--)
		*tmp++ = *s++;
}

static inline void memcpy_asm(void *pDest, void *pData, unsigned int dwByteCount) {
    __asm__ __volatile__(
        "cld\n\t"
        "mov %1, %%esi\n\t"     // Load pData into esi
        "mov %2, %%ecx\n\t"     // Load dwByteCount into ecx
        "mov %0, %%edi\n\t"     // Load pDest into edi
        "sub $256, %%ecx\n\t"   // Subtract 256 from dwByteCount
        "jl 1f\n\t"             // Jump if less than to DonePreWarm\n\t
        
        ".align 16\n"           // Align loop to 16-byte boundary
        "2:\n\t"
        "movb (%%esi), %%al\n\t"       // Read byte from esi to al
        "movb 32(%%esi), %%bl\n\t"     // Read byte from esi + 32 to bl
        "movb 64(%%esi), %%al\n\t"     // Read byte from esi + 64 to al
        "movb 96(%%esi), %%bl\n\t"     // Read byte from esi + 96 to bl
        "movb 128(%%esi), %%al\n\t"    // Read byte from esi + 128 to al
        "movb 160(%%esi), %%bl\n\t"    // Read byte from esi + 160 to bl
        "movb 192(%%esi), %%al\n\t"    // Read byte from esi + 192 to al
        "movb 224(%%esi), %%bl\n\t"    // Read byte from esi + 224 to bl
        "add $256, %%esi\n\t"          // Increment esi by 256
        "nop\n\t"                       // NOP to force instruction pairing
        "sub $256, %%ecx\n\t"          // Subtract 256 from ecx
        "jg 2b\n\t"               // Jump if greater than to PreWarm\n\t
        
        "1:\n\t"
        "mov %2, %%ecx\n\t"            // Reload dwByteCount into ecx
        "mov %1, %%esi\n\t"            // Reload pData into esi
        "sub $32, %%ecx\n\t"           // Subtract 32 from dwByteCount
        "jl 3f\n\t"              // Jump if less than to DoneCopy\n\t
        
        ".align 16\n"                  // Align loop to 16-byte boundary
        "4:\n\t"
        "fildq (%%esi, %%ecx)\n\t"     // Load qword from [esi + ecx] onto the FPU stack
        "fildq 8(%%esi, %%ecx)\n\t"    // Load qword from [esi + ecx + 8] onto the FPU stack
        "fildq 16(%%esi, %%ecx)\n\t"   // Load qword from [esi + ecx + 16] onto the FPU stack
        "fxch %%st(2)\n\t"             // Exchange st(2) and st(0) on the FPU stack
        "fildq 24(%%esi, %%ecx)\n\t"   // Load qword from [esi + ecx + 24] onto the FPU stack
        "fxch %%st(2)\n\t"             // Exchange st(2) and st(0) on the FPU stack
        "fistpq 8(%%edi, %%ecx)\n\t"   // Store qword from st(0) to [edi + ecx + 8] and pop
        "fistpq (%%edi, %%ecx)\n\t"    // Store qword from st(0) to [edi + ecx] and pop
        "fistpq 24(%%edi, %%ecx)\n\t"  // Store qword from st(0) to [edi + ecx + 24] and pop
        "fistpq 16(%%edi, %%ecx)\n\t"  // Store qword from st(0) to [edi + ecx + 16] and pop
        "sub $32, %%ecx\n\t"           // Subtract 32 from ecx
        "jge 4b\n\t"             // Jump if greater than or equal to LoopCopy\n\t
        
        "3:\n\t"
        :
        : "g"(pDest), "g"(pData), "g"(dwByteCount)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
}



void	VID_Update (vrect_t *rects)
{
	//memcpy64(ScreenMem, (basebuffer + (BASEWIDTH*BASEHEIGHT)/2), (BASEWIDTH*BASEHEIGHT)/16);
	memcpy_asm(ScreenMem, basebuffer, (BASEWIDTH*BASEHEIGHT));
	
	//movedata(_my_ds(), (unsigned)basebuffer, vram_dpmi_selector, page_to_draw_to[back_buffer_page], (BASEWIDTH*BASEHEIGHT));

    /*uint8_t tmp = front_buffer_page;
    front_buffer_page = back_buffer_page;
    back_buffer_page = tmp;
	outportb(0xA4, front_buffer_page);*/

	/* Wait until vsync occurs. */
	while (!(inp(GDC_GFX_STATUS) & GDC_STATUS_VSYNC)) {}
	/* Now wait until it's over. */
	while (inp(GDC_GFX_STATUS)   & GDC_STATUS_VSYNC) {}
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


