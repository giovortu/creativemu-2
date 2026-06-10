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
// Compilatore : DEV-C++ 4.9.7.9 gcc                                                     
//
//****************************************************************************************


  //***************************************
  // Funzioni e definizioni usate dal main
  //***************************************

  #include <stdint.h>

  //****************************
  // Costanti
  //****************************

// Undef this constant to compile under *Unix/Linux
  
//#define WINDOWS 1 
  
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


#define SAMPLE_RATE 44100

#define CPUTIME 0.000500

#define SYNCRO_TIME 100

#define MAX_PIA 1

#define CV_NUMCOLORS 16


  //*****************************
  // Typedefs per compatibilit�*
  // codice                     *
  //*****************************
  typedef unsigned short WORD;
  typedef unsigned char BYTE;
  typedef BYTE *BYTEptr;


  //*****************************
  // Contatore sincronizzazione                   
  //*****************************
  unsigned int  Ticks;
  bool fullscreen=false;
  
  //****************************
  // Nome del file della rom
  //****************************
  char RomName[255];
  char BiosName[255]="bios/Biosdsw.rom";  
  
  //**************************
  // Variabili globali
  //**************************
  
  SDL_Window *Screen = NULL;
  SDL_Surface *WindowSurface = NULL;
  SDL_Surface *RealWindowSurface = NULL; // Actual hardware surface (varies in fullscreen)
  SDL_Surface *CvScreen = NULL;
  SDL_Surface *Menu = NULL;


  SDLFont *font;
    
  SDL_Event event;
  SDL_Rect CvRect,rect,rect2,MenuRect;
  Uint32 color,color2;

  char msg[50],st[40];
  bool done=false;
  bool sMenu=false;
  
  int Cycle=0;
  unsigned char VdpIrq=0;
  
 
  //******************************
  // Cpu 6502
  //******************************
  extern BYTE *cvMemory;

  //*****************************
  // Registri 
  // flags = NVRBDIZC 
  //*****************************
  extern BYTE a_reg,x_reg,y_reg,flag_reg,s_reg;
  extern WORD pc_reg;
 
  //******************************
  // Processore video
  //******************************
  tms9918 *Vdp=new tms9918;

  //******************************
  // Interfaccia del PIA  
  //******************************

  const struct pia6821_interface PiaInterface=
   { 
    RdPiaPortA,           // Lettura porta A
    RdPiaPortB,           //    "    porta B
    0,                    //    "    Ca1 disabilitata
    RdPiaCb1,             //    "    Cb1 (Ready chip audio)
    0,                    //    "    Ca2 disabilitata
    RdPiaCb2,             //    "    Cb2 solo uscita per enable audio
    WrPiaPortA,           // Scrittura porta A
    WrPiaPortB,           //    "      porta B
    0,                    // Scrittura Ca2 disabilitata
    WrPiaCb2,             //    "      Cb2 (Abilitazioni chip audio)
    0,                    // Interrupts disabilitati
    0                     //    "            "
    };

  //******************************
  // Chip sonoro
  //******************************  
  extern struct sn76496 sn[MAX_76496];
  
  SDL_AudioSpec *Audio_spec;

  //********************************
  // Puntatore alla memoria video  *
  //********************************
  BYTE *VdpRam=new BYTE[CV_VIDEO_WIDTH * CV_VIDEO_HEIGHT];
  
  //************************************  
  // Palette dei colori CreatiVision   *
  //************************************
  SDL_Color SDL_CvPal[16]=
  {
  {0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00},{0x20,0xC0,0x20,0x00},{0x60,0xE0,0x60,0x00},
  {0x20,0x20,0xE0,0x00},{0x40,0x60,0xE0,0x00},{0xA0,0x20,0x20,0x00},{0x40,0xC0,0xE0,0x00},
  {0xE0,0x20,0x20,0x00},{0xE0,0x60,0x60,0x00},{0xC0,0xC0,0x20,0x00},{0xC0,0xC0,0x80,0x00},
  {0x20,0x80,0x20,0x00},{0xC0,0x40,0xA0,0x00},{0xA0,0xA0,0xA0,0x00},{0xE0,0xE0,0xE0,0x00}
  };


  SDL_Color SDL_CvPalBw[16]=
  {
  {0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00},{0x7f,0x7f,0x7f,0x00},{0xad,0xad,0xad,0x00},
  {0x35,0x35,0x35,0x00},{0x66,0x66,0x66,0x00},{0x47,0x47,0x47,0x00},{0x9f,0x9f,0x9f,0x00},
  {0x5b,0x5b,0x5b,0x00},{0x89,0x89,0x89,0x00},{0xb1,0xb1,0xb1,0x00},{0xbb,0xbb,0xbb,0x00},
  {0x59,0x59,0x59,0x00},{0x72,0x72,0x72,0x00},{0xa2,0xa2,0xa2,0x00},{0xe3,0xe3,0xe3,0x00}
  };
  
  //*************************************
  // Codici tastiera
  //*************************************    

  int LeftKeys=0xff;
  int LeftJoy=0xff;
  
  int RightKeys=0xff;
  int RightJoy=0xff;
  
  //*************************************
  // Audio
  //*************************************

  bool audio_enabled = true;

  #define AUDIO_BUFFER_SIZE (SAMPLE_RATE * 4)   // 4-second ring buffer — fits any ALSA/PulseAudio latency
  int16_t audio_buffer[AUDIO_BUFFER_SIZE];
  volatile int audio_write_pos = 8192;
  volatile int audio_read_pos = 0;
  bool Paused=false;

  
  //*************************************
  // Funxioni di inizializzazione
  //*************************************
  void InitScreen(void);
  void SetupScreen();
  void InitHardware(void);
  void InitAudio(void);
  
  //*************************
  // Check linea di comando
  //*************************
  int CheckCmdLine(int argc,char **argv);

  //*************************
  // Render screen (main thread only)
  //*************************
  void RenderScreen(void);
  
  //****************************
  // Sincronizzazione emulatore
  //****************************
  void Syncro( int time );
  
  //*****************************
  // Trash Emulazione
  //*****************************
  void FreeEmu(void);
  
  
  //*****************************
  // Wait for a keypress
  //*****************************
  void WaitKey(void);

  //*****************************
  // Toggle Fullscreen/Window
  //*****************************
  void SwapFullScreen(void);
  
  //*****************************
  // Check SDL Events
  //*****************************
  void CheckEvents(void);
  
  //*****************************
  // Show Cpu Status
  //*****************************
  void ShowStatus(void);
  
  //*****************************
  // Show Menu
  //*****************************
  void ShowMenu(void);
  
  //*****************************
  // Audio callback function
  //*****************************
  void AudioCallback(void *userdata, Uint8 *stream, int len);

  //*****************************
  // Present scaled frame to window (fullscreen-aware)
  //*****************************
  void PresentScreen(void);
