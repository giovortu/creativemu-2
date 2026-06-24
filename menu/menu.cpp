//****************************************************************************************
//
// CvEmu2 versione 0.3.0-beta
//
// Ideato progettato e relizzato da Giovanni Ortu
//
// Il programma utilizza le seguenti librerie e porzioni di codice:
//
// Cpu 6502    : Basato sul codice scritto da Neil Bradley.
// Vdp 9929    : EMULib Emulation Library, Copyright (C) Marat Fayzullin 1996-2002
// Pia 6821    : Emulatore ricavato dal MAME
// Audio 76489 : EMULib Emulation Library, Copyright (C) Marat Fayzullin 1996-2002
//
//
//
// Librerie    : SDL
// Compilatore : DEV-C++ 4.9.7.9 gcc
//
//****************************************************************************************

//******************************************************
//
// Funzioni per la gestione del menu interno
//
//******************************************************

#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#ifdef WINDOWS
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755)
#endif
#include "menu.h"
#include "../savestate/savestate.h"
#include "../mem/cvmemory.h"
#include "../cpu/cpu6502.h"

extern char RomName[255];
extern char BiosName[255];
extern BYTE *cvMemory;

//******************************************************
// Struttura contenente il Menu
//
// String : Stringa descrittiva dell'operazione
// func   : Funzione da richiamare
//******************************************************

#define MENU_ITEMS 6

struct
{

  char String[20];
  void (*func)();

} MenuItems[MENU_ITEMS] =
    {
        "LOAD ROM", LoadRom,
        "-----------", NULL,
        "SAVE STATE", SaveState,
        "LOAD STATE", LoadState,
        "-----------", NULL,
        "SNAPSHOT", Snapshot};

//******************************************************
// Inizializzazione del Menu
//******************************************************

void InitMenu(void)
{

  MenuRect.w = MENU_WIDTH;
  MenuRect.h = MENU_HEIGHT;
  MenuRect.x = (VIDEO_WIDTH - MENU_WIDTH) / 2;
  MenuRect.y = (VIDEO_HEIGHT - MENU_HEIGHT) / 2;

  Menu = SDL_CreateRGBSurface(0, MENU_WIDTH, MENU_HEIGHT, 8, 0, 0, 0, 0);
  SDL_SetPaletteColors(Menu->format->palette, SDL_CvPal, 0, 16);
  SDL_SetSurfaceBlendMode(Menu, SDL_BLENDMODE_BLEND);
  font = initFont(1, 1, 1, 20);
}

//******************************************************
// Ritraccia del Menu
//******************************************************

void DrawMenu(int Selected)
{

  // Cancella lo schermo ricopiando l'ultima immagine dell'emulatore

  SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);

  // Disegna sul menu un rettangolo
  color = SDL_MapRGB(Menu->format, 0x00, 0x3f, 0x7f);
  SDL_FillRect(Menu, NULL, color);

  // Scrive l'intestazione
  drawString(Menu, font, 25, 10, "CVEMU 0.3.0-BETA");

  // Coordinate e dimensione del rettangolo che seleziona la scelta
  rect2.w = MENU_WIDTH - 10;
  rect2.h = 10;
  rect2.x = 5;
  rect2.y = 34 + Selected * 10;
  color = SDL_MapRGB(Menu->format, 0x7f, 0x7f, 0x3f);
  SDL_FillRect(Menu, &rect2, color);

  // Scrivo gli elementi del menu
  for (int i = 0; i < MENU_ITEMS; i++)
    drawString(Menu, font, (int)(0.5 * (MENU_WIDTH - 8 * strlen(MenuItems[i].String))), 30 + i * 10, MenuItems[i].String);

  // Setto la trasparenza del menu...
  SDL_SetSurfaceAlphaMod(Menu, 220);
  // ...lo copio sullo schermo...
  SDL_UpperBlit(Menu, NULL, WindowSurface, &MenuRect);

  // ... e ripristino la trasparenza...
  SDL_SetSurfaceAlphaMod(Menu, 255);

  // ... e lo visualizzo.
  PresentScreen();
}

//******************************************************
// Mostra il menu e gestisce i tasti
//******************************************************

void ShowMenu(void)
{

  int KeySymbol, Selected = 0, OldSel;

  if (sMenu)
  {

    // Metto in pausa l'emulazione
    Paused = true;
    SDL_PauseAudio(1);

    // Inizializzo il menu
    InitMenu();

    // Setto i colori dell'ultima immagine dell'emulazione in scala di grigi
    GrayVideo();

    // Disegno il menu.
    DrawMenu(Selected);

    // Verifica dei tasti...
    while (sMenu)
    {

      // Ritraccio il menu solo se la selezione cambia
      OldSel = Selected;

      // Pool degli eventi SDL
      if (SDL_PollEvent(&event))
      {
        KeySymbol = event.key.keysym.sym;

        switch (event.type)
        {
        case SDL_QUIT:
          done = true;
          sMenu = false;
          break;

        case SDL_KEYDOWN:
          switch (KeySymbol)
          {
          // Uscita dal menu
          case SDLK_TAB:
          case SDLK_ESCAPE:
            sMenu = false;
            break;

          // Scelta dell'elemento del menu
          case SDLK_DOWN:
            Selected++;
            if (Selected >= MENU_ITEMS)
              Selected = 0;
            break;
          case SDLK_UP:
            Selected--;
            if (Selected < 0)
              Selected = MENU_ITEMS - 1;
            break;

          // Selezione
          case SDLK_RETURN:
            if (MenuItems[Selected].func != NULL)
              MenuItems[Selected].func();
            sMenu = false;
            break;

          default:
            break;
          }
        }
      }

      // Ritraccio il menu solo se la selezione cambia
      if (OldSel != Selected)
        DrawMenu(Selected);
    }
  }

  // Se devo invece disattivare il menu...
  if (!sMenu)
  {
    // Disattivo la pausa...
    Paused = false;
    SDL_PauseAudio(0);
  }
}

//******************************************************
// Cambia la palette dello schermo in toni di grigio
//******************************************************

void GrayVideo(void)
{
  SDL_SetPaletteColors(CvScreen->format->palette, SDL_CvPalBw, 0, 16);
  SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);
  PresentScreen();
}

//******************************************************
// Mette in pausa l'emulazione
//******************************************************

void PauseCv(void)
{
  // Se sono in pausa...
  if (Paused)
  {
    // Disativa il timer video
    if (sMenu)
    {
      sMenu = false;
      ShowMenu();
    }
    GrayVideo();
    font = initFont(0, 0, 1, 20);
    rect2.w = 50;
    rect2.h = 16;
    rect2.x = VIDEO_WIDTH / 2 - 25;
    rect2.y = VIDEO_HEIGHT / 2;
    color = SDL_MapRGB(WindowSurface->format, 0x90, 0x9f, 0xdf);
    SDL_FillRect(WindowSurface, &rect2, color);
    drawString(WindowSurface, font, VIDEO_WIDTH / 2 - 20, VIDEO_HEIGHT / 2, "PAUSE");
    PresentScreen();
  }
}

//******************************************************
// Salva una foto dell'emulatore
//******************************************************

void Snapshot(void)
{
  char name[255];

  if (MKDIR("snapshots") != 0 && errno != EEXIST)
  {
    printf("Impossibile creare la cartella snapshots\n");
    return;
  }

  sprintf(name, "snapshots/shot_%d.bmp", time(NULL));

  SDL_SetPaletteColors(CvScreen->format->palette, SDL_CvPal, 0, 16);

  SDL_SaveBMP(CvScreen, name);
  SDL_SetPaletteColors(CvScreen->format->palette, SDL_CvPalBw, 0, 16);
}

void LoadRom(void)
{
  char typedPath[255] = "";
  int typedLen = 0;
  bool inputDone = false;
  bool inputCancel = false;

  SDL_StartTextInput();

  while (!inputDone && !inputCancel && !done)
  {
    // Draw the greyed-out screen background
    SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);

    // Draw a centered dialog box
    SDL_Rect outerRect = { 10, 90, 300, 70 };
    SDL_Rect innerRect = { 12, 92, 296, 66 };
    SDL_FillRect(WindowSurface, &outerRect, SDL_MapRGB(WindowSurface->format, 255, 255, 255));
    SDL_FillRect(WindowSurface, &innerRect, SDL_MapRGB(WindowSurface->format, 0, 31, 63));

    // Draw labels
    drawString(WindowSurface, font, 20, 98, "ENTER ROM PATH:");
    drawString(WindowSurface, font, 20, 118, typedPath);

    // Draw blinking cursor
    int cursorX = 20 + typedLen * 8;
    if ((SDL_GetTicks() / 500) % 2 == 0) {
      drawString(WindowSurface, font, cursorX, 118, "_");
    }

    PresentScreen();

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
      if (ev.type == SDL_QUIT)
      {
        done = true;
        inputCancel = true;
      }
      else if (ev.type == SDL_TEXTINPUT)
      {
        int len = strlen(ev.text.text);
        if (typedLen + len < 254)
        {
          strcat(typedPath, ev.text.text);
          typedLen += len;
        }
      }
      else if (ev.type == SDL_KEYDOWN)
      {
        if (ev.key.keysym.sym == SDLK_RETURN)
        {
          inputDone = true;
        }
        else if (ev.key.keysym.sym == SDLK_ESCAPE)
        {
          inputCancel = true;
        }
        else if (ev.key.keysym.sym == SDLK_BACKSPACE)
        {
          if (typedLen > 0)
          {
            typedPath[--typedLen] = '\0';
          }
        }
      }
    }
    SDL_Delay(16);
  }

  SDL_StopTextInput();

  if (inputDone && typedLen > 0)
  {
    char oldRomName[255];
    strcpy(oldRomName, RomName);
    strcpy(RomName, typedPath);

    if (createCvMemory(cvMemory, BiosName, RomName))
    {
      reset6502();
    }
    else
    {
      strcpy(RomName, oldRomName);

      // Draw error dialog
      SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);
      SDL_Rect outerRect = { 10, 90, 300, 70 };
      SDL_Rect innerRect = { 12, 92, 296, 66 };
      SDL_FillRect(WindowSurface, &outerRect, SDL_MapRGB(WindowSurface->format, 255, 0, 0));
      SDL_FillRect(WindowSurface, &innerRect, SDL_MapRGB(WindowSurface->format, 63, 0, 0));

      drawString(WindowSurface, font, 20, 98, "LOAD ERROR!");
      drawString(WindowSurface, font, 20, 118, "Press any key to return...");
      PresentScreen();

      bool keyPressed = false;
      while (!keyPressed && !done)
      {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
          if (ev.type == SDL_QUIT)
          {
            done = true;
          }
          else if (ev.type == SDL_KEYDOWN)
          {
            keyPressed = true;
          }
        }
        SDL_Delay(16);
      }
    }
  }
}
