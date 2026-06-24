#ifndef __CVEMU_H__
#define __CVEMU_H__

#include <stdint.h>
#include <SDL2/SDL.h>
#include "font/font.h"
#include "video/tms9918.h"
#include "pia/6821pia.h"
#include "audio/sn76496.h"

#define SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE (SAMPLE_RATE * 4)
#define CV_VIDEO_WIDTH  300
#define CV_VIDEO_HEIGHT 230
#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240
#define MAX_PIA 1

class CvEmu
{
public:
  CvEmu();
  ~CvEmu();

  bool Init(int argc, char *argv[]);
  void Run();
  void Reset();

  void InitScreen();
  void SetupScreen();
  void InitHardware();
  void InitAudio();
  int CheckCmdLine(int argc, char **argv);
  void RenderScreen();
  void Syncro(int time);
  void FreeEmu();
  void WaitKey();
  void SwapFullScreen();
  void CheckEvents();
  void ShowStatus();
  void PresentScreen();

  static void AudioCallbackStatic(void *userdata, Uint8 *stream, int len);
  void AudioCallback(Uint8 *stream, int len);

public:
  // --- References to Global Emulator State Variables ---
  unsigned int &m_Ticks;
  bool &m_fullscreen;

  char (&m_RomName)[255];
  char (&m_BiosName)[255];

  SDL_Window *&m_Screen;
  SDL_Surface *&m_WindowSurface;
  SDL_Surface *&m_RealWindowSurface;
  SDL_Surface *&m_CvScreen;
  SDL_Surface *&m_Menu;

  SDLFont *&m_font;

  SDL_Event &m_event;
  SDL_Rect &m_CvRect;
  SDL_Rect &m_rect;
  SDL_Rect &m_rect2;
  SDL_Rect &m_MenuRect;
  Uint32 &m_color;
  Uint32 &m_color2;

  char (&m_msg)[50];
  char (&m_st)[40];
  bool &m_done;
  bool &m_sMenu;

  int &m_Cycle;
  unsigned char &m_VdpIrq;

  tms9918 *&m_Vdp;
  BYTE *&m_VdpRam;

  SDL_AudioSpec *&m_Audio_spec;

  int &m_LeftKeys;
  int &m_LeftJoy;

  int &m_RightKeys;
  int &m_RightJoy;

  bool &m_audio_enabled;
  int &m_audio_output_rate;
  int &m_audio_output_channels;

  volatile int &m_audio_write_pos;
  volatile int &m_audio_read_pos;
  bool &m_Paused;
};

#endif
