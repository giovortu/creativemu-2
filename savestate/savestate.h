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


//********************************************
// Funzioni per il salvataggio / ripristino
// dello stato dell'emulazione.
//********************************************

#ifndef __savestate__
#define __savestate__

#define A a_reg
#define X x_reg
#define Y y_reg
#define P flag_reg
#define S s_reg
#define PC pc_reg


#include "../cpu/cpu6502.h"

extern tms9918 *Vdp;
extern BYTE *cvMemory;
extern struct sn76496 sn[1];
extern struct pia6821 pia[1];
extern BYTE *VdpRam;
extern BYTE a_reg,x_reg,y_reg,flag_reg,s_reg;
extern WORD pc_reg;

void SaveState( void );
void LoadState( void );




#endif
