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
#include <stdlib.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include <dirent.h>
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
  fontDark = initFont(0, 0, 0, 20);

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
    drawString(Menu, i==Selected ? fontDark : font, (int)(0.5 * (MENU_WIDTH - 8 * strlen(MenuItems[i].String))), 30 + i * 10, MenuItems[i].String);

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
  char roms[256][256];
  int romCount = 0;
  int selected = 0;
  int scrollOffset = 0;
  bool inputDone = false;
  bool inputCancel = false;

  // Get executable path to resolve root directory
  char exePath[1024];
  char rootDir[1024];
  #ifdef WINDOWS
    GetModuleFileName(NULL, exePath, sizeof(exePath));
  #else
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len != -1) exePath[len] = '\0';
    else strcpy(rootDir, ".");
  #endif

  if (strlen(exePath) > 0) {
    char *lastSlash = strrchr(exePath, '/');
    #ifdef WINDOWS
      if (!lastSlash) lastSlash = strrchr(exePath, '\\');
    #endif

    if (lastSlash) {
      strncpy(rootDir, exePath, lastSlash - exePath);
      rootDir[lastSlash - exePath] = '\0';
    } else {
      strcpy(rootDir, ".");
    }
  } else {
    strcpy(rootDir, ".");
  }

  char romsFolderPath[1024];
  sprintf(romsFolderPath, "%s/roms", rootDir);
  printf("Searching for ROMs in: %s\n", romsFolderPath);

  // Scan roms folder
  #ifdef WINDOWS
    struct _finddata_t c_file;
    char searchPath[1024];
    sprintf(searchPath, "%s/*.bin", romsFolderPath);
    if (_findfirst(searchPath, &c_file) == 0) {
      do {
        strncpy(roms[romCount], c_file.filenam, 255);
        roms[romCount][255] = '\0';
        romCount++;
        if (romCount >= 256) break;
      } while (_findnext(&c_file) == 0);
    }
  #else
    DIR *dir = opendir(romsFolderPath);
    if (dir == NULL) {
      printf("Error: Could not open directory %s: %s\n", romsFolderPath, strerror(errno));
    } else {
      struct dirent *ent;
      printf("Scanning directory...\n");
      while ((ent = readdir(dir)) != NULL && romCount < 256) {
        printf("  Checking file: %s\n", ent->d_name);
        if (strstr(ent->d_name, ".rom") || strstr(ent->d_name, ".bin")) {
          strncpy(roms[romCount], ent->d_name, 255);
          roms[romCount][255] = '\0';
          romCount++;
        }
      }
      closedir(dir);
    }
  #endif

  if (romCount > 0) {
    printf("Found %d ROMs:\n", romCount);
    for (int i = 0; i < romCount; i++) {
      printf("  [%d] %s\n", i, roms[i]);
    }
  }

  if (romCount == 0) {
    SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);
    SDL_Rect outerRect = { 10, 90, 300, 70 };
    SDL_Rect innerRect = { 12, 92, 296, 66 };
    SDL_FillRect(WindowSurface, &outerRect, SDL_MapRGB(WindowSurface->format, 255, 0, 0));
    SDL_FillRect(WindowSurface, &innerRect, SDL_MapRGB(WindowSurface->format, 63, 0, 0));
    
    char errorMsg[256];
    sprintf(errorMsg, "NO ROMS FOUND in %s", romsFolderPath);
    drawString(WindowSurface, font, 20, 98, "%s", errorMsg);
    drawString(WindowSurface, font, 20, 118, "Check folder...");
    PresentScreen();
    bool keyPressed = false;
    while (!keyPressed && !done) {
      SDL_Event ev;
      while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) done = true;
        else if (ev.type == SDL_KEYDOWN) keyPressed = true;
      }
      SDL_Delay(16);
    }
    return;
  }

  SDL_StartTextInput();
  int maxVisible = 10;
  while (!inputDone && !inputCancel && !done)
  {
    SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);
    
    SDL_Rect outerRect = { 10, 40, 300, 160 };
    SDL_Rect innerRect = { 12, 42, 296, 156 };
    SDL_FillRect(WindowSurface, &outerRect, SDL_MapRGB(WindowSurface->format, 255, 255, 255));
    SDL_FillRect(WindowSurface, &innerRect, SDL_MapRGB(WindowSurface->format, 0, 31, 63));

    drawString(WindowSurface, font, 20, 50, "SELECT ROM:");

    for (int i = 0; i < maxVisible && (i + scrollOffset) < romCount; i++) {
      int idx = i + scrollOffset;
      char *filename = roms[idx];
      
      char *nameOnly = filename;
      char *lastSlashLocal = strrchr(filename, '/');
      if (lastSlashLocal) nameOnly = lastSlashLocal + 1;
      
      // Convert to uppercase for display
      char upperName[256];
      int j = 0;
      while (nameOnly[j] && j < 255) {
        upperName[j] = toupper((unsigned char)nameOnly[j]);
        j++;
      }
      upperName[j] = '\0';
      
      if (idx == selected) {
        SDL_Rect selRect = { 15, 70 + i * 12, 280, 11 };
        SDL_FillRect(WindowSurface, &selRect, SDL_MapRGB(WindowSurface->format, 0x7f, 0x7f, 0x3f));
      }
      drawString(WindowSurface, font, 20, 70 + i * 12, "%s", upperName);
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
      else if (ev.type == SDL_KEYDOWN)
      {
        switch(ev.key.keysym.sym)
        {
          case SDLK_RETURN:
            inputDone = true;
            break;
          case SDLK_ESCAPE:
            inputCancel = true;
            break;
          case SDLK_UP:
            if (selected > 0) {
              selected--;
              if (selected < scrollOffset) scrollOffset--;
            }
            break;
          case SDLK_DOWN:
            if (selected < romCount - 1) {
              selected++;
              if (selected >= scrollOffset + maxVisible) scrollOffset++;
            }
            break;
        }
      }
    }
    SDL_Delay(16);
  }
  SDL_StopTextInput();

  if (inputDone && !done)
  {
    char fullPath[1024];
    sprintf(fullPath, "%s/roms/%s", rootDir, roms[selected]);

    strcpy(RomName, fullPath);
    if (createCvMemory(cvMemory, BiosName, RomName))
    {
      reset6502();
    }
    else
    {
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
          if (ev.type == SDL_QUIT) done = true;
          else if (ev.type == SDL_KEYDOWN) keyPressed = true;
        }
        SDL_Delay(16);
      }
    }
  }
}
