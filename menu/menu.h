
#ifndef __menu__
#define __menu__

#include <SDL2/SDL.h>
#include <string.h>
#include "../include/defs.h"
#include "../video/tms9918.h"


#define MENU_WIDTH 150
#define MENU_HEIGHT 140


extern SDL_Surface *Menu;
extern SDL_Rect MenuRect,CvRect,rect2;
extern SDL_Window *Screen;
extern SDL_Surface *WindowSurface;
extern SDL_Surface *CvScreen;
extern tms9918 *Vdp;
extern bool sMenu,Paused,done;
extern struct SDLFont *font;
extern SDL_Color SDL_CvPal[16],SDL_CvPalBw[16];
extern Uint32 color;
extern void RenderScreen(void);
extern SDLFont *initFont(float r, float g, float b, float a);
extern void drawString(SDL_Surface *screen, SDLFont *font, int x, int y, char *str,...);
extern SDL_Event event;


void ShowMenu( void );
void PauseCv( void );
void GrayVideo( void );
void DrawMenu( int  );
void InitMenu( void );
void Snapshot( void );
void LoadRom( void );

// Present the scaled virtual surface to the real window.
extern void PresentScreen(void);

#endif


