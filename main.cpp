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
// Compilatore
//     Windows : DEV-C++ 4.9.7.0 gcc
//       Linux : g++
//
//****************************************************************************************


#define PROGRAMRELEASE "0.3.0-beta"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "include/memory.h"
#include "include/defs.h"
#include "font/font.h"

#include "cpu/cpu6502.h"
#include "video/tms9918.h"
#include "mem/cvmemory.h"
#include "pia/6821pia.h"
#include "audio/sn76496.h"
#include "menu/menu.h"
#include "savestate/savestate.h"
#include "keyboard/keyboard.h"

// ==========================================
// --- TUNING PARAMETERS ---
// ==========================================

// Tunes the overall speed/tempo of the game.
// 1.0 = normal (50Hz PAL speed)
// 1.2 = 20% faster (simulates NTSC 60Hz timing)
// 0.8 = 20% slower
const double EMU_TEMPO_MULTIPLIER = 1.00;

// Tunes the physical pitch/frequency of the audio notes.
// 1.0 = normal (FunnyMu reference clock: VDP_CLOCK/15 ~= 3.546MHz)
// 1.1 = 10% higher pitch
// 0.9 = 10% lower pitch
const double EMU_PITCH_MULTIPLIER = 1.0;
const int EMU_PSG_BASE_CLOCK = 3546667;
const int EMU_HBLANK_CYCLES = 128;
const int EMU_VBLANK_CYCLES = 40000 - (EMU_HBLANK_CYCLES * 192);

// ==========================================

#include "main.h"

#ifdef WINDOWS
#include <windows.h>
#endif

//****************
//*     Main     *
//****************

int main(int argc, char *argv[])
{

  //******************************
  // Check sui parametri da linea
  // di comando
  //******************************
  if (!CheckCmdLine(argc, argv))
  {
    FreeEmu();
    exit(0);
  }

  //******************************
  // Inizializzazione Video (SDL)
  //******************************
  InitScreen();

  //******************************
  // Inizializzazione Audio (SDL)
  //******************************
  InitAudio();

  InitMenu();

  // No ROM was given on the command line: RomName is still empty, so we
  // skip loading it here and instead show the ROM selection menu once the
  // hardware is ready (below).
  bool romGivenOnCmdLine = (RomName[0] != '\0');

  //*********************************
  // Inizializzo la memoria della Cv
  //*********************************

  if (romGivenOnCmdLine && !createCvMemory(cvMemory, BiosName, RomName))
  {
    FreeEmu();
    exit(0);
  }

  //*********************************
  // Inizializzazione Hardware (EMU)
  //*********************************

  InitHardware();

  if (romGivenOnCmdLine)
  {
    //******************************
    // Resetto la Cpu
    //******************************
    reset6502();
  }
  else
  {
    //******************************************************
    // Nessuna rom sulla linea di comando: mostro subito il
    // menu di selezione rom per permettere l'hot-load.
    //******************************************************
    RenderScreen(); // Ensure CvScreen exists before drawing menu overlays.
    LoadRom();
    if (!done && RomName[0] == '\0')
      done = true; // User closed the ROM browser without picking one.
  }

  //**************************************************
  // Avvio l'audio
  //**************************************************
  SDL_PauseAudio(0);

  //******************************
  // Ciclo principale
  //******************************
  int cycles_this_slice = 256;//EMU_HBLANK_CYCLES;
  while (!done)
  {

    if (!Paused)
    {

#ifdef DIAGNOSTICS
      // --- Timing diagnostics ---
      static Uint32 dbg_last = 0;
      Uint32 dbg_now = SDL_GetTicks();
      if (dbg_last > 0 && dbg_now - dbg_last > 200)
        fprintf(stderr, "[STALL] %ums gap before Loop9918\n", dbg_now - dbg_last);
      dbg_last = dbg_now;
      // --------------------------
#endif

      exec6502(cycles_this_slice);

      Syncro(cycles_this_slice);

      VdpIrq = Loop9918(Vdp);
      if (VdpIrq)
        irq6502();

      if (Vdp->Line == 0)
      {

#ifdef CHECK_FPS
        // --- VBlank FPS counter ---
        static int dbg_frames = 0;
        static Uint32 dbg_fps_timer = 0;
        dbg_frames++;
        if (dbg_now - dbg_fps_timer >= 2000)
        {
          fprintf(stderr, "[FPS] %d VBlanks in %.1f sec (%.1f fps)\n",
                  dbg_frames, (dbg_now - dbg_fps_timer) / 1000.0,
                  dbg_frames * 1000.0 / (dbg_now - dbg_fps_timer + 1));
          dbg_frames = 0;
          dbg_fps_timer = dbg_now;
        }
        // --------------------------
#endif

        RenderScreen();
      }

#ifdef AUDIO_TELEMETRY
      // --- Timing telemetry (disabled by default) ---
      {
        static Uint32 tele_start = 0;
        static Uint32 tele_last = 0;
        static int tele_vblank = 0;
        if (tele_start == 0)
          tele_start = SDL_GetTicks();
        if (VdpIrq)
          tele_vblank++;
        Uint32 tele_now = SDL_GetTicks();
        if (tele_now - tele_last >= 1000)
        {
          int elapsed = (int)(tele_now - tele_start);
          if (elapsed <= 16000)
          {
            int buffered = audio_write_pos - audio_read_pos;
            if (buffered < 0)
              buffered += AUDIO_BUFFER_SIZE;
            fprintf(stderr, "[TELE t=%ds] vblank_irq/s=%d  audio_buf=%dms\n",
                    elapsed / 1000, tele_vblank,
                    buffered * 1000 / (audio_output_rate > 0 ? audio_output_rate : 44100));
          }
          tele_vblank = 0;
          tele_last = tele_now;
        }
      }
#endif

    } // Fine Pausa

    CheckEvents();

    //******************************
    // Fine loop emulazione
    //******************************
  } // !Done

  //**************************************************
  // Interrompo l'audio
  //**************************************************

  SDL_PauseAudio(1);

  //******************************
  // Distruggo i buffer allocati
  //******************************
  FreeEmu();

  //******************************
  // Fine del programma
  //******************************

  return 1;
}

//**************************************************************************
//************************* Definizione funzioni ***************************
//**************************************************************************

//*******************************************
// FreeEmu :
//*******************************************
// Liber la memoria allocata dall'emulatore
//*******************************************

void FreeEmu(void)
{
  //******************************
  // Distruggo le superfici SDL
  //******************************

  if (WindowSurface)
    SDL_FreeSurface(WindowSurface);
  if (CvScreen)
    SDL_FreeSurface(CvScreen);
  if (Menu)
    SDL_FreeSurface(Menu);

  //******************************
  // Elimino il font usato
  //******************************

  if (font)
    freeFont(font);

  if (Vdp)
    Trash9918(Vdp);
  // Vdp, VdpRam, cvMemory are now static/stack-allocated — no delete needed.

  SDL_Quit();

} // Fine Free Emu

//*******************************************
// InitScreen :
//*******************************************
// Inizializza le SDL ed il layout del video
//*******************************************

void InitScreen(void)
{

  //************************************
  // Initializzo SDL
  //************************************
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
  {
    sprintf(msg, "Couldn't initialize SDL video and timer: %s\n", SDL_GetError());
#ifdef WINDOWS
    MessageBox(0, msg, "Error", MB_ICONHAND);
#else
    printf("%s", msg);
#endif
    exit(1);
  }

  // SDL_WM_SetIcon(SDL_LoadBMP("cvemu2.bmp"), NULL);
  //***************************************************
  //  Create window
  //***************************************************

  Screen = SDL_CreateWindow("CreatiVemu2 for Linux", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VIDEO_WIDTH, VIDEO_HEIGHT, 0);

  if (Screen == NULL)
  {
    sprintf(msg, "Couldn't create window: %s\n", SDL_GetError());
#ifdef WINDOWS
    MessageBox(0, msg, "Error", MB_ICONHAND);
#else
    printf("%s", msg);
#endif
    exit(2);
  }

  // Keep the real (hardware) surface separately.
  // WindowSurface is always a fixed 320x240 virtual surface.
  RealWindowSurface = SDL_GetWindowSurface(Screen);
  WindowSurface = SDL_CreateRGBSurface(0, VIDEO_WIDTH, VIDEO_HEIGHT,
                                       RealWindowSurface->format->BitsPerPixel,
                                       RealWindowSurface->format->Rmask,
                                       RealWindowSurface->format->Gmask,
                                       RealWindowSurface->format->Bmask,
                                       RealWindowSurface->format->Amask);

// Setta la caption della finestra
#ifdef WINDOWS
  SDL_SetWindowTitle(Screen, "CreatiVemu2 for Windows");
#else
  SDL_SetWindowTitle(Screen, "CreatiVemu2 for Linux");
#endif

  SetupScreen();
}

void SetupScreen()
{

  // Colora di "Blu Creativision" la finestra
  color = SDL_MapRGB(WindowSurface->format, 0x00, 0x00, 0x00);
  SDL_FillRect(WindowSurface, NULL, color);

  // Mostra il riquadro video della Cv
  color = SDL_MapRGB(WindowSurface->format, 0x00, 0x00, 0x00); // 0x3f);
  CvRect.w = CV_VIDEO_WIDTH;
  CvRect.h = CV_VIDEO_HEIGHT;
  CvRect.x = (VIDEO_WIDTH / 2) - (CvRect.w / 2);
  CvRect.y = (VIDEO_HEIGHT / 2) - (CvRect.h / 2);
  SDL_FillRect(WindowSurface, &CvRect, color);

  SDL_UpdateWindowSurface(Screen);
  // Fine inizializzazione video
}

//*******************************************
// AudioCallback function
//*******************************************
// Runs in SDL's audio thread. Following FunnyMu design, samples are
// generated directly in the callback from current SN76496 state.
//*******************************************
void AudioCallback(void *udata, Uint8 *stream, int len)
{
  int channels = (audio_output_channels > 0) ? audio_output_channels : 1;
  int frames = len / (sizeof(int16_t) * channels);
  int16_t *out = reinterpret_cast<int16_t *>(stream);

  if (!audio_enabled || frames <= 0)
  {
    memset(stream, 0, len);
    return;
  }

  if (channels == 1)
  {
    sn76496Update(0, out, frames);
    return;
  }

  int16_t mix[1024];
  if (frames > 1024)
    frames = 1024;

  sn76496Update(0, mix, frames);

  for (int i = 0; i < frames; i++)
  {
    int16_t sample = mix[i];
    for (int ch = 0; ch < channels; ch++)
      out[i * channels + ch] = sample;
  }
}

// ... (in InitAudio) ...

void InitAudio(void)
{
  // Use stack-allocated specs — no heap allocation needed.
  SDL_AudioSpec desired{};

  desired.freq     = 22050;
  desired.format   = AUDIO_S16SYS;
  desired.samples  = 256;
  desired.channels = 1;
  desired.callback = AudioCallback;
  desired.userdata = NULL;

  // Passing NULL for `obtained` tells SDL to open the device in whatever
  // native format it needs (e.g. WASAPI's float32 stereo on Windows) and
  // transparently convert to/from our requested format under the hood.
  // If we pass a non-NULL `obtained` instead, SDL skips that conversion and
  // the callback must produce audio in the *obtained* format — which on
  // Windows/WASAPI differs from S16 mono and silently breaks playback.
  if (SDL_OpenAudio(&desired, NULL) < 0)
  {
    printf("Couldn't open audio: %s\n", SDL_GetError());
    exit(2);
  }

  audio_output_rate     = desired.freq;
  audio_output_channels = desired.channels;

  // Store a pointer to the desired spec for legacy references (non-owning).
  static SDL_AudioSpec s_audio_spec = desired;
  Audio_spec = &s_audio_spec;

  // Keep audio paused until hardware is initialized.
  SDL_PauseAudio(1);
}

//*******************************************
// InitHardware :
// Inizializza l'hardware dell'emulatore
//*******************************************

void InitHardware(void)
{
  //**********************************
  // Inizializzo l'emulazione del PIA
  //**********************************
  pia_config(0, PIA_STANDARD_ORDERING, &PiaInterface);
  pia_reset();
  pia_set_input_ca1(0, 1);
  pia_set_input_ca2(0, 1);

  //************************************
  // Inizializzo l'emulazione del suono
  // Inizializzo l'emulazione del chip audio
  // Reset76489(&SoundChip,0);
  // Sync76489(&SoundChip,SN76489_SYNC);

  sn76496Init(0, (int)(EMU_PSG_BASE_CLOCK * EMU_PITCH_MULTIPLIER), 150, audio_output_rate);

  //******************************************
  // Inizializzo l'emulazione del chip video
  // e la palette dei colori
  //******************************************
  New9918(Vdp, VdpRam, CV_VIDEO_WIDTH, CV_VIDEO_HEIGHT);
  SetPalette(Vdp);

  //******************************
  // Inizializzo la Cpu
  //******************************
  init6502();
}

//*******************************************
// RefreshVideo :
//*******************************************
// Scrivo il buffer video sullo schermo
// 50 volte al secondo
//*******************************************

void RenderScreen(void)
{
  if (!CvScreen)
  {
    CvScreen = SDL_CreateRGBSurfaceFrom((BYTE *)Vdp->XBuf, CV_VIDEO_WIDTH, CV_VIDEO_HEIGHT, 8, CV_VIDEO_WIDTH, 0, 0, 0, 0);
  }
  SDL_SetPaletteColors(CvScreen->format->palette, SDL_CvPal, 0, 16);
  SDL_UpperBlit(CvScreen, NULL, WindowSurface, &CvRect);
  PresentScreen();
}

//*******************************************
// PresentScreen :
//*******************************************
// Scales the virtual 320x240 WindowSurface onto RealWindowSurface
// with 4:3 aspect-ratio letterboxing, then flushes to the window.
//*******************************************
void PresentScreen(void)
{
  if (!WindowSurface || !RealWindowSurface)
    return;

  if (RealWindowSurface->w == WindowSurface->w &&
      RealWindowSurface->h == WindowSurface->h)
  {
    // Windowed, same size: simple blit, no scaling needed.
    SDL_BlitSurface(WindowSurface, NULL, RealWindowSurface, NULL);
  }
  else
  {
    // Fullscreen or resized: scale with 4:3 aspect ratio.
    SDL_Rect dest;
    int rw = RealWindowSurface->w;
    int rh = RealWindowSurface->h;

    if (rw * 3 > rh * 4)
    {
      // Screen wider than 4:3 -> pillarbox (black left/right bars)
      dest.h = rh;
      dest.w = rh * 4 / 3;
      dest.x = (rw - dest.w) / 2;
      dest.y = 0;
    }
    else
    {
      // Screen taller than 4:3 -> letterbox (black top/bottom bars)
      dest.w = rw;
      dest.h = rw * 3 / 4;
      dest.x = 0;
      dest.y = (rh - dest.h) / 2;
    }

    SDL_FillRect(RealWindowSurface, NULL,
                 SDL_MapRGB(RealWindowSurface->format, 0, 0, 0));
    SDL_BlitScaled(WindowSurface, NULL, RealWindowSurface, &dest);
  }

  SDL_UpdateWindowSurface(Screen);
}

//*******************************************
// Syncro :
//*******************************************
// Sincronizzazione dell'eulatore
//*******************************************

void Syncro(int time)
{
  (void)time;
  static Uint32 old_timer = 0;
  const Uint32 frame_delay_ms = (Uint32)((20.0 / EMU_TEMPO_MULTIPLIER) + 0.5);

  // FunnyMu-style pacing: throttle once per completed frame.
  if (Vdp->Line != 0)
    return;

  if (old_timer == 0)
    old_timer = SDL_GetTicks();

  // Fix: use SDL_Delay() to yield the CPU instead of a busy-wait spin.
  Uint32 now = SDL_GetTicks();
  Uint32 elapsed = now - old_timer;
  if (elapsed < frame_delay_ms)
    SDL_Delay(frame_delay_ms - elapsed);

  old_timer = SDL_GetTicks();
}

//*******************************************
// CheckCmdLine :
//*******************************************
// Verifica le opzioni della linea di comando
//*******************************************
// Versione 2: accetta sulla linea di comando il bios

int CheckCmdLine(int argc, char **argv)
{
  //****************************************
  // Se ho switch sulla linea di comando...
  //****************************************

  if (argc > 1)
  {
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
      printf("\nCreatiVemu2 v0.5-beta - CreatiVision / Dick Smith Wizard emulator\n\n");
      printf("Usage:\n");
      printf("  %s romname [biosname]\n\n", argv[0]);
      printf("Arguments:\n");
      printf("  romname            Path to the ROM cartridge file (optional: omit to pick\n");
      printf("                     one from the internal ROM load menu at startup)\n");
      printf("  biosname           Path to the BIOS file (default: bios/Biosdsw.rom)\n\n");
      printf("  --help             This help\n\n");
      printf("  --version          Application version\n\n");
      printf("\n\n");

      printf("Emulator keys:\n");
      printf("  TAB                Open/close in-emulator menu\n");
      printf("  F2                 Pause / unpause\n");
      printf("  F3                 Toggle fullscreen / windowed\n");
      printf("  F5                 Reset (NMI)\n");
      printf("  ESC                Quit\n\n");
      return 0;
    }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
    {
        printf("%s\n", PROGRAMRELEASE );
        return 0;
    }

    strcpy(RomName, argv[1]);
    if (argc > 2)
      strcpy(BiosName, argv[2]);
    return 1;
  }

  //****************************************
  // Nessun argomento: RomName resta vuoto e
  // l'utente sceglierà una rom dal menu
  // interno all'avvio (hot-load).
  //****************************************

  printf("\nNo ROM specified on the command line: opening the internal ROM load menu...\n\n");

  return 1;
}

//*******************************************
// WaitKey :
//*******************************************
// Attende la pressione di un tasto
//*******************************************

void WaitKey(void)
{
  while (1)
  {
    SDL_PollEvent(&event);
    if (event.type == SDL_KEYDOWN)
      break;
  }
}

//*******************************************
// SwapFullScreen :
//*******************************************
// Cambia Fullscreen / windowed
//*******************************************

void SwapFullScreen()
{
  fullscreen = (fullscreen ? false : true);

  // SDL_WINDOW_FULLSCREEN_DESKTOP keeps the desktop resolution
  // and lets the OS scale; far more compatible than SDL_WINDOW_FULLSCREEN.
  SDL_SetWindowFullscreen(Screen, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

  // The window surface is invalidated after a mode change — must re-acquire.
  RealWindowSurface = SDL_GetWindowSurface(Screen);

  if (fullscreen)
    SDL_ShowCursor(SDL_DISABLE);
  else
    SDL_ShowCursor(SDL_ENABLE);

  if (sMenu)
    ShowMenu();
  else if (Paused)
    PauseCv();
  else
    RenderScreen();
}

void CheckEvents(void)
{
  int KeySymbol;

  // Use while (not if) to drain the entire event queue each frame.
  // This prevents stalls when SDL floods the queue (e.g., on fullscreen toggle).
  while (SDL_PollEvent(&event))
  {
    KeySymbol = event.key.keysym.sym;

    switch (event.type)
    {
    case SDL_QUIT:
      done = true;
      break;

    case SDL_KEYDOWN:
      switch (KeySymbol)
      {
      case SDLK_ESCAPE:
        done = true;
        break;

      case SDLK_TAB:
        sMenu = !sMenu;
        ShowMenu();
        break;

      case SDLK_F3:
        SwapFullScreen();
        break;

      case SDLK_F2:
        Paused = !Paused;
        PauseCv();
        break;

      case SDLK_F5:
        nmi6502();
        break;

      default:
        CheckCvKeysDown(KeySymbol);
        break;
      }
      break;

    case SDL_KEYUP:
      CheckCvKeysUp(KeySymbol);
      break;

    case SDL_WINDOWEVENT:
      // On any window-size event (e.g., after fullscreen toggle), re-acquire
      // the real window surface so subsequent renders are not stale.
      if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
          event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
      {
        RealWindowSurface = SDL_GetWindowSurface(Screen);
      }
      break;

    default:
      break;
    }
  }
}
