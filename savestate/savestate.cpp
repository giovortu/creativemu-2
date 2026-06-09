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

#include "savestate.h"
//#include <zlib.h>z

//**********************************************
// Save emulation state
//**********************************************

void SaveState( void )
{
 FILE *fp;

 if ((fp=fopen("state.raw","wb")))
 {

 fwrite( (void *) cvMemory,sizeof( BYTE ),0xffff,fp);
 fwrite( (void *) Vdp, sizeof( tms9918 ), 1,fp);
 fwrite( (void *) VdpRam, sizeof( BYTE ), CV_VIDEO_WIDTH * CV_VIDEO_HEIGHT,fp);
 fwrite( (void *) pia, sizeof( struct pia6821 * ), 1,fp);

 fwrite( (void *) &a_reg, sizeof( BYTE ),1,fp);
 fwrite( (void *) &x_reg, sizeof( BYTE ),1,fp);
 fwrite( (void *) &y_reg, sizeof( BYTE ),1,fp);
 fwrite( (void *) &flag_reg, sizeof( BYTE ),1,fp);
 fwrite( (void *) &s_reg, sizeof( BYTE ),1,fp);
 fwrite( (void *) &pc_reg, sizeof( BYTE ),1,fp);

 fclose (fp);
 }

}

//**********************************************
// Load emulation state
//**********************************************

void LoadState( void )
{
 FILE *fp;

 if ((fp=fopen("state.raw","rb")))
 {

 fread( (BYTE *) cvMemory,sizeof( BYTE ),0xffff,fp);
 fread( (tms9918 *) Vdp, sizeof( tms9918 ), 1,fp);
 fread( (BYTE *) VdpRam, sizeof( BYTE ), CV_VIDEO_WIDTH * CV_VIDEO_HEIGHT,fp);
 fread( (struct pia6821 *) pia, sizeof( struct pia6821 * ), 1,fp);

 fread( (BYTE *) &a_reg, sizeof( BYTE ),1,fp);
 fread( (BYTE *) &x_reg, sizeof( BYTE ),1,fp);
 fread( (BYTE *) &y_reg, sizeof( BYTE ),1,fp);
 fread( (BYTE *) &flag_reg, sizeof( BYTE ),1,fp);
 fread( (BYTE *) &s_reg, sizeof( BYTE ),1,fp);
 fread( (BYTE *) &pc_reg, sizeof( BYTE ),1,fp);

 fclose (fp);
 }

}

