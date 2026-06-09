#ifndef __DEFS__
#define __DEFS__

typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef BYTE *BYTEptr;


#define VDPDATAWR 0x3000
#define VDPDATARD 0x2000

#define VDPCTRLWR 0x3001
#define VDPCTRLRD 0x2001

#define PIAPORTS  0x1000

#define PIA_PORTS_DDRA  0x1000
#define PIA_PORTS_CTLA  0x1001
#define PIA_PORTS_DDRB  0x1002
#define PIA_PORTS_CTLB  0x1003

#define	PIA_DDRA	0
#define	PIA_CTLA	1
#define	PIA_DDRB	2
#define	PIA_CTLB	3

#define VDPDR 4
#define VDPDW 5
#define VDPRR 6
#define VDPRW 7

#define IN_PORT  0
#define OUT_PORT 1

#define SCR_MODE_FULLSCREEN 0
#define SCR_MODE_WINDOW     1

#define SAMPLE_RATE 44100

// 1/2000000 Hz * 1000 = ms per CPU cycle (6502 at 2 MHz)
#define CPUTIME 0.000500


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define    RMASK  0xff000000
#define    GMASK  0x00ff0000
#define    BMASK  0x0000ff00
#define    AMASK  0x000000ff
#else
#define    RMASK  0x000000ff
#define    GMASK  0x0000ff00
#define    BMASK  0x00ff0000
#define    AMASK  0xff000000
#endif


// Video display refresh: 20ms = 50 fps (PAL)
#define TIMER_SPEED 20
#define FADE_SPEED  50

#define CV_VIDEO_WIDTH  300
#define CV_VIDEO_HEIGHT 230

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240


#endif
