#ifndef sn76496_H
#define sn76496_H

#include <stdint.h>

#define MAX_76496 2

struct sn76496
{
  int SampleRate;
  unsigned int UpdateStep;
  int VolTable[16];             /* volume table */
  int Register[8];              /* registers */
  int LastRegister;             /* last register written */
  int Volume[4];                /* volume of voice 0-2 and noise */
  unsigned int RNG;             /* noise generator */
  int NoiseFB;                  /* noise feedback mask */
  int Period[4];
  int Count[4];
  int Output[4];
};

extern struct sn76496 sn[MAX_76496];

int sn76496Init(int chip, int clock, int gain, int sample_rate);
void sn76496Write(int chip, int data);
void sn76496Update(int chip, int16_t *buffer, int length);

#endif
