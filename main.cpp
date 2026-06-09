//****************************************************************************************
//                                                                                        
// CvEmu2 versione 0.5 alpha 1
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
//       Linux : g++ 2.9.6
//
//****************************************************************************************



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

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
#include "keyboard/keyboard.h"

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
  if (!CheckCmdLine(argc,argv))
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

  //*********************************
  // Inizializzo la memoria della Cv 
  //********************************* 

  if (!createCvMemory(cvMemory, BiosName, RomName))
  {
   FreeEmu();
   exit(0);
  }  

  //******************************
  // Se necessario leggo il file
  // .INI. Se non esiste lo creo
  //******************************
  // if (!CmdLine) IniFile();

  //*********************************
  // Inizializzazione Hardware (EMU)
  //*********************************
  
  InitHardware();
  
  //******************************
  // Resetto la Cpu
  //******************************
  reset6502();
  
  //**************************************************
  // Avvio l'audio
  //**************************************************
  SDL_PauseAudio(0);
  
  //******************************
  // Ciclo principale
  //******************************
  

  
  while (!done)
  {
  if (Paused) fprintf(stderr, "[PAUSE] Paused set to true!\n");
  if (!Paused)
  {

  // --- Timing diagnostics ---
  static Uint32 dbg_last = 0;
  Uint32 dbg_now = SDL_GetTicks();
  if (dbg_last > 0 && dbg_now - dbg_last > 200)
    fprintf(stderr, "[STALL] %ums gap before Loop9918\n", dbg_now - dbg_last);
  dbg_last = dbg_now;
  // --------------------------

  VdpIrq = Loop9918(Vdp);
  if (VdpIrq) irq6502();

  if (Vdp->Line == 0) {

    // --- VBlank FPS counter ---
    static int dbg_frames = 0;
    static Uint32 dbg_fps_timer = 0;
    dbg_frames++;
    if (dbg_now - dbg_fps_timer >= 2000) {
      fprintf(stderr, "[FPS] %d VBlanks in %.1f sec (%.1f fps)\n",
              dbg_frames, (dbg_now - dbg_fps_timer) / 1000.0,
              dbg_frames * 1000.0 / (dbg_now - dbg_fps_timer + 1));
      dbg_frames = 0;
      dbg_fps_timer = dbg_now;
    }
    // --------------------------

    Uint32 t0 = SDL_GetTicks();
    RenderScreen();
    Uint32 t1 = SDL_GetTicks();
    if (t1 - t0 > 50)
      fprintf(stderr, "[STALL] RenderScreen took %ums\n", t1 - t0);

    // --- Audio generation (50Hz) ---
    if (audio_enabled) {
      static Uint32 last_audio_time = 0;
      Uint32 now = SDL_GetTicks();
      if (last_audio_time == 0) last_audio_time = now;
      while (now - last_audio_time >= 20) { // 20ms = 50Hz
          int samples_to_write = SAMPLE_RATE / 50;
          
          SDL_LockAudio();
          
          if (audio_write_pos + samples_to_write > AUDIO_BUFFER_SIZE) {
            int first_part = AUDIO_BUFFER_SIZE - audio_write_pos;
            sn76496Update(0, &audio_buffer[audio_write_pos], first_part);
            sn76496Update(0, &audio_buffer[0], samples_to_write - first_part);
          } else {
            sn76496Update(0, &audio_buffer[audio_write_pos], samples_to_write);
          }
          audio_write_pos = (audio_write_pos + samples_to_write) % AUDIO_BUFFER_SIZE;
          
          SDL_UnlockAudio();
          last_audio_time += 20;
      }
    }
  }

  // --- VBlank FPS counter ---
  if (Vdp->Line == 0) {
    // ... (keep FPS counter here) ...
    RenderScreen();
    // ...
  }

  Uint32 te0 = SDL_GetTicks();
  exec6502(207);
  Uint32 te1 = SDL_GetTicks();
  if (te1 - te0 > 50)
    fprintf(stderr, "[STALL] exec6502 took %ums\n", te1 - te0);

  Uint32 ts0 = SDL_GetTicks();
  Syncro(207);
  Uint32 ts1 = SDL_GetTicks();
  if (ts1 - ts0 > 50)
    fprintf(stderr, "[STALL] Syncro took %ums\n", ts1 - ts0);

 } // Fine Pausa

  Uint32 tc0 = SDL_GetTicks();
  CheckEvents();
  Uint32 tc1 = SDL_GetTicks();
  if (tc1 - tc0 > 50)
    fprintf(stderr, "[STALL] CheckEvents took %ums\n", tc1 - tc0);

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

  if (WindowSurface) SDL_FreeSurface(WindowSurface);
  if (CvScreen) SDL_FreeSurface(CvScreen);
  if (Menu)  SDL_FreeSurface(Menu);

  //******************************
  // Elimino il font usato
  //******************************

  if (font) freeFont(font);

  if (Vdp)
  { Trash9918(Vdp);  delete Vdp; }

  if (VdpRam)   delete VdpRam;
  if (cvMemory) delete cvMemory;

  delete Audio_spec;

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
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
     sprintf (msg, "Couldn't initialize SDL video and timer: %s\n", SDL_GetError ());
     #ifdef WINDOWS
     MessageBox (0, msg, "Error", MB_ICONHAND);
     #else
     printf("%s",msg);
     #endif
     exit (1);
    }

    //SDL_WM_SetIcon(SDL_LoadBMP("cvemu2.bmp"), NULL);
    //***************************************************
    // Create window
    //***************************************************

    Screen = SDL_CreateWindow ("CreatiVemu2 for Linux", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VIDEO_WIDTH, VIDEO_HEIGHT, 0);

    if (Screen == NULL)
    {
     sprintf (msg, "Couldn't create window: %s\n", SDL_GetError ());
     #ifdef WINDOWS
     MessageBox (0, msg, "Error", MB_ICONHAND);
     #else
     printf("%s",msg);
     #endif
     exit (2);
    }

    WindowSurface = SDL_GetWindowSurface(Screen);

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
   color = SDL_MapRGB (WindowSurface->format, 0x00,0x00,0x00);
   SDL_FillRect (WindowSurface, NULL, color);

    // Mostra il riquadro video della Cv
   color = SDL_MapRGB (WindowSurface->format, 0x00, 0x00, 0x00);//0x3f);
   CvRect.w = CV_VIDEO_WIDTH;
   CvRect.h = CV_VIDEO_HEIGHT;
   CvRect.x = (VIDEO_WIDTH / 2) - (CvRect.w / 2);
   CvRect.y = (VIDEO_HEIGHT / 2) - (CvRect.h / 2);
   SDL_FillRect (WindowSurface, &CvRect, color);

   SDL_UpdateWindowSurface(Screen);
  // Fine inizializzazione video


}

//*******************************************
// AudioCallback function
//*******************************************
// Runs in SDL's audio thread — never acquires the main thread's resources.
// Reads behind audio_write_pos so it always plays the most recently
// generated audio.  Ring buffer is 4 seconds deep, so it handles any
// ALSA/PulseAudio hardware buffer size without wrapping.
//*******************************************
void AudioCallback(void *udata, Uint8 *stream, int len)
{
    int samples = len / sizeof(int16_t);
    int16_t *out = reinterpret_cast<int16_t*>(stream);
    
    for (int i = 0; i < samples; i++) {
        if (audio_read_pos == audio_write_pos) {
            out[i] = 0; // Buffer underrun: play silence
        } else {
            out[i] = audio_buffer[audio_read_pos];
            audio_read_pos = (audio_read_pos + 1) % AUDIO_BUFFER_SIZE;
        }
    }
}

// ... (in InitAudio) ...

void InitAudio(void)
{
 SDL_AudioSpec *desired = new SDL_AudioSpec;

 desired->freq = SAMPLE_RATE;
 desired->format = AUDIO_S16SYS;
 desired->samples = 1024;
 desired->channels = 1;
 desired->callback = AudioCallback;
 desired->userdata = NULL;

 SDL_AudioSpec *obtained = new SDL_AudioSpec; // Add obtained spec

 if (SDL_OpenAudio(desired, obtained) < 0)
     {
      // ... (error handling)
      exit (2);
     }
     
  // If desired format is not supported, this might be an issue.
  // For now, let's keep it simple and just make sure it opens.

  Audio_spec=desired;
  
  // Explicitly resume audio
  SDL_PauseAudio(0);
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
  pia_config(0,PIA_STANDARD_ORDERING,&PiaInterface);
  pia_reset();
  pia_set_input_ca1(0,1);
  pia_set_input_ca2(0,1);
 

  //************************************
  // Inizializzo l'emulazione del suono
  //************************************
  //Reset76489(&SoundChip,0);
  //Sync76489(&SoundChip,SN76489_SYNC);

  sn76496Init(0, 3579545, 150, SAMPLE_RATE);

  //******************************************
  // Inizializzo l'emulazione del chip video
  // e la palette dei colori
  //******************************************
  New9918(Vdp,VdpRam,CV_VIDEO_WIDTH,CV_VIDEO_HEIGHT);
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
  if (!CvScreen) {
    CvScreen = SDL_CreateRGBSurfaceFrom((BYTE*) Vdp->XBuf,CV_VIDEO_WIDTH,CV_VIDEO_HEIGHT,8,CV_VIDEO_WIDTH,0,0,0,0);
  }
  SDL_SetPaletteColors(CvScreen->format->palette, SDL_CvPal, 0, 16);
  SDL_UpperBlit(CvScreen,NULL,WindowSurface,&CvRect);
  SDL_UpdateWindowSurface(Screen);
}

//*******************************************
// Syncro :
//*******************************************
// Sincronizzazione dell'eulatore
//*******************************************

void Syncro( int time )
{
 unsigned int Diff,TimeCycle;
  
 Cycle++;
 if (Cycle>=SYNCRO_TIME)
 {
  Diff=SDL_GetTicks()-Ticks; 
  TimeCycle=(unsigned int)(CPUTIME*time*SYNCRO_TIME);

  // Print once to verify timing constants
  static bool once = true;
  if (once) {
    fprintf(stderr, "[SYNCRO] TimeCycle=%u Diff=%u CPUTIME=%.6f\n", TimeCycle, Diff, (double)CPUTIME);
    once = false;
  }
  
  if (Diff<TimeCycle) { SDL_Delay(TimeCycle-Diff); }
  Ticks=SDL_GetTicks();
  Cycle=0;


  
 }
 
}

//*******************************************
// CheckCmdLine :
//*******************************************
// Verifica le opzioni della linea di comando
//*******************************************
// Versione 2: accetta sulla linea di comando il bios

int CheckCmdLine(int argc,char **argv)
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
   printf("  romname            Path to the ROM cartridge file\n");
   printf("  biosname           Path to the BIOS file (default: bios/Biosdsw.rom)\n\n");
   printf("Keys:\n");
   printf("  TAB                Open/close in-emulator menu\n");
   printf("  F2                 Pause / unpause\n");
   printf("  F3                 Toggle fullscreen / windowed\n");
   printf("  F5                 Reset (NMI)\n");
   printf("  ESC                Quit\n\n");
   return 0;
  }

  strcpy(RomName,argv[1]);
  if (argc>2)
   strcpy(BiosName,argv[2]);
  return 1;
 }
 
 printf("\nNo argument passed on command line!\n\n") ;
 printf("Usage:\n%s romname [ biosname ]\n\n",(argv[0]));
 
 return 0;

}

//*******************************************
// WaitKey :
//*******************************************
// Attende la pressione di un tasto
//*******************************************
   
void WaitKey(void)
{
   while(1)
   {
     SDL_PollEvent(&event);
     if (event.type==SDL_KEYDOWN) break;
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

  SDL_SetWindowFullscreen(Screen, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

  if (fullscreen) SDL_ShowCursor(SDL_DISABLE);
  else SDL_ShowCursor(SDL_ENABLE);

  if (sMenu) ShowMenu();
  else if (Paused) PauseCv();
  else RenderScreen();
}


void CheckEvents( void )
{
 int KeySymbol;

 if (SDL_PollEvent(&event))
 {
  KeySymbol=event.key.keysym.sym;

  switch (event.type)
  {
   // Uscita dall'applicazione
   case SDL_QUIT:
    done = true;
    break;
   // Tasto premuto
   case SDL_KEYDOWN:
     {
      switch ( KeySymbol )
      {

	  // tasti che devono essere sempre attivi.
       case SDLK_ESCAPE:
        done = true;
        break;

       case SDLK_TAB: // Apre il menu interno
          sMenu=!sMenu; ShowMenu();
          break;

       case SDLK_F3: // Toggle fullscreen
         SwapFullScreen();  break;


	   case SDLK_F2: // Pausa
          Paused=!Paused; PauseCv();
          break;

	    case SDLK_F5: // Pulsante di reset della Cv
         nmi6502(); break;

		default: CheckCvKeysDown( KeySymbol ); break;

     }
     break;


  case SDL_KEYUP:
        {
         CheckCvKeysUp( KeySymbol );   break;
        }


   default:  break;
    
   }
   
 } // Pool eventi

 }// if
 

}



/*
void SetVideoSize(int Width, int Height, char* Caption)
{

int flags;


if (fullscreen)
	flags = SDL_HWPALETTE | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;
else
	flags = SDL_HWPALETTE | SDL_HWSURFACE | SDL_DOUBLEBUF;

    // Libero la vecchia superficie
    if (Screen) SDL_FreeSurface(Screen);

    //***************************************************
    // Set VIDEO_WIDTH x VIDEO_HEIGHT  video mode
    //***************************************************

    Screen = SDL_SetVideoMode (Width, Height, CV_NUMCOLORS, flags);// | SDL_FULLSCREEN);

    if (Screen == NULL)
    {
     sprintf (msg, "Couldn't set video resolution : %s\n",
     SDL_GetError ());

     #ifdef WINDOWS
     MessageBox (0, msg, "Error", MB_ICONHAND);
     #else
     printf("%s",msg);
     #endif

     exit (2);
    }


   // Setto la palette
   SDL_SetColors(Screen, SDL_CvPal, 0, 16);

   // Setta la caption della finestra
   SDL_WM_SetCaption (Caption, NULL);
}
*/



