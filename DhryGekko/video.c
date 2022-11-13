/*
 **************************************************************************************
 * Basic screen output using external video framebuffer.
 ************************************************************************************** 
*/

#include <dolphin.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "video.h"
#include "../Common/gc_font.h"

/* Framebuffer for video rendering purposes. */
static u8 *xfb[2];
static int vid_frame;

#ifdef  VIDEO_PAL
	static GXRenderModeObj *render_mode = &GXPal528Int;
#endif
#ifdef  VIDEO_NTSC
	static GXRenderModeObj *render_mode = &GXNtsc480Int;
#endif

static GXColor COLOR_DKGRAY = { 128, 128, 128, 0 };
static GXColor COLOR_BLACK  = { 0, 0, 0, 0 };
static GXColor COLOR_WHITE  = { 255, 255, 255, 0 };

#define LINE_WIDTH      79      /* Max. line width in characters. */
#define LINES           200     /* History buffer memory in lines. */
static  char            message_buf[LINES][LINE_WIDTH+1];
static  int             message_line, message_roller;

/* Initialize GC video hardware. */
void vi_initialize(void)
{
	int xfbsiz;

	/* Allocate frame buffer. */
	xfbsiz = VIPadFrameBufferWidth(render_mode->fbWidth) * render_mode->xfbHeight * VI_DISPLAY_PIX_SZ;
	xfb[0] = (u8 *)OSRoundUp32B(OSGetArenaLo());
	xfb[1] = (u8 *)OSRoundUp32B(xfb[0] + xfbsiz);
	OSSetArenaLo((void *)(xfb[0] + 2 * xfbsiz));
	
	VIInit();
	VIConfigure(render_mode);
	VISetBlack(FALSE);
	VIFlush();
	VIWaitForRetrace();
	
	vid_frame = 0;
	
	memset(message_buf, 0, sizeof(message_buf));
	message_line = message_roller = 0;
}

#define CLAMP(a)                \
{                               \
	if((a) < 0) (a) = 0;        \
	if((a) > 255) (a) = 255;    \
}                               \

#define CLAMPY(a)               \
{                               \
	if((a) < 16) (a) = 16;      \
	if((a) > 235) (a) = 235;    \
}                               \

static inline void yuv_convert(GXColor col, int *y, int *u, int *v)
{
	register int ty, tu, tv;
	float r, g, b;
	
	OSInitFastCast();
	OSu8tof32(&col.r, &r);
	OSu8tof32(&col.g, &g);
	OSu8tof32(&col.b, &b);

	/* As homebrewn manuals. */
//    ty = (int)( 0.29900 * r + 0.58700 * g + 0.11400 * b);
//    tu = (int)(-0.16874 * r - 0.33126 * g + 0.50000 * b + 128);
//    tv = (int)( 0.50000 * r - 0.41869 * g - 0.08131 * b + 128);
	
	/* As official SDK. */
	ty = (int)( 0.257 * r + 0.504 * g + 0.098 * b + 16);
	tu = (int)(-0.148 * r - 0.291 * g + 0.439 * b + 128);
	tv = (int)( 0.439 * r - 0.368 * g - 0.071 * b + 128);

	CLAMPY(ty);
	CLAMP(tu);
	CLAMP(tv);
	
	*y = ty;
	*u = tu;
	*v = tv;
}

static void fill_line(int n, GXColor col)
{
	u8 *ptr, *fb;
	int linesiz;
	int y, u, v;
	u32 packed;
	
	fb = (vid_frame & 1) ? xfb[1] : xfb[0];
	linesiz = VIPadFrameBufferWidth(render_mode->fbWidth) * VI_DISPLAY_PIX_SZ;
	fb += n * linesiz;

	yuv_convert(col, &y, &u, &v);
	
	packed = (u32)((y << 24) | (u << 16) | (y << 8) | (v));

	for(ptr=fb; ptr<fb+linesiz; ptr+=VI_DISPLAY_PIX_SZ*2)
	{
		*(u32 *)ptr = packed;
	}
	DCStoreRange(fb, (u32)linesiz);
}

static void put_char(char c, int xstart, int ystart)
{
	static BOOL first_time = TRUE;
	int linesiz, i, j, sampl[2];
	u8 *x, *fb, *old;
	u8 *bitmap;
	static int y[2], u[2], v[2];
	u32 packed;
	
	x = (vid_frame & 1) ? xfb[1] : xfb[0];
	linesiz = VIPadFrameBufferWidth(render_mode->fbWidth) * VI_DISPLAY_PIX_SZ;
	
	if(first_time)
	{
		yuv_convert(COLOR_DKGRAY, &y[0], &u[0], &v[0]);
		yuv_convert(COLOR_WHITE , &y[1], &u[1], &v[1]);        
		first_time = FALSE;
	}

	bitmap = &gc_font[(int)(u8)c * GC_FONT_HEIGHT];
	
	for(i=0; i<GC_FONT_HEIGHT; i++)
	{
		fb = x + (ystart + i) * linesiz + xstart * VI_DISPLAY_PIX_SZ;    
		old = fb;
		for(j=0; j<GC_FONT_WIDTH; j+=2)
		{
			sampl[0] = (bitmap[i] >> (7 - j)) & 1;
			sampl[1] = (bitmap[i] >> (6 - j)) & 1;
			packed = (u32)((y[sampl[0]] << 24) | (((u[sampl[0]] + u[sampl[1]]) / 2) << 16) | (y[sampl[1]] << 8) | ((v[sampl[0]] + v[sampl[1]]) / 2));
			*(u32 *)fb = packed;
			fb += 2 * VI_DISPLAY_PIX_SZ;
		}
		fb = old;
		DCStoreRange(fb, GC_FONT_WIDTH * VI_DISPLAY_PIX_SZ);
	}    
}

/*
 * Draw (screen height) lines from 'message_roller' location (limited by LINES).
 *             -------------------   0
 *            |      MESSAGE      |  .
 *            |      BUFFER       |  .
 *            |                   |  .
 * roller --->|- - - - - - - - - -|  .
 *            |XXXXXXXXXXXXXXXXXXX|  .
 *            |XXXX DRAW THIS XXXX|  .
 *            |XXXXXXXXXXXXXXXXXXX|  .
 *            |- - - - - - - - - -|  .
 *            |                   |  .                              
 *             -------------------   'LINES' defined
*/
void update_message_buf(void)
{
	int line, y, roller, c, len;
 
	for(line=0, roller=message_roller; line<render_mode->xfbHeight/GC_FONT_HEIGHT; line++, roller++)
	{
		for(y=0; y<GC_FONT_HEIGHT; y++)
		{
			fill_line(GC_FONT_HEIGHT*line + y, COLOR_DKGRAY);
		}
		if(roller < message_line)
		{
			len = (int)strlen(message_buf[roller]);
			for(c=0; c<len, c<LINE_WIDTH; c++)
			{
				put_char(message_buf[roller][c], c*GC_FONT_WIDTH, line*GC_FONT_HEIGHT);
			}
		}
	}
}

static void add_line(char *text)
{
	if(message_line >= LINES)
	{
		message_line = message_roller = 0;
	}    
	strncpy(message_buf[message_line], text, LINE_WIDTH);
	message_line++;
	if((message_line - message_roller) >= (render_mode->xfbHeight/GC_FONT_HEIGHT))
	{
		message_roller++;
	}
}

/* Add lines in message history to message_roller location. */
int my_printf(char *msg, ...)
{
	va_list arg;
	char buf[1000], c, *line;
	int i, len;

	va_start(arg, msg);
	len = vsprintf(buf, msg, arg);
	va_end(arg);
	
	for(i=0, line=buf; i<len; i++)
	{
		c = buf[i];
		if(c == '\n')
		{
			buf[i] = '\0';
			add_line(line);
			line = &buf[i+1];
		}
	}
	return 0;
}

void vi_swap_chain (void)
{
	u8 *fb;

	/* Swap buffers. */
	fb = (vid_frame & 1) ? xfb[1] : xfb[0];
	VISetNextFrameBuffer((void *)fb);
	VIFlush();
	VIWaitForRetrace();
	
	/* Increase frame counter. */
	vid_frame++;
}

int vi_get_frame_count (void)
{
	return vid_frame;
}
