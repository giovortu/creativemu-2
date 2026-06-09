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

//************************************************************************
// createCvMemory
//
// Funzione per inizializzare la memoria della CreatiVision con il bios
// e la rom del gioco
//
//************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvmemory.h"


int loadRom(char *Filename,BYTE *Rom)
{
 FILE *fRom;
 int data,count=0;
 
 if ( (fRom=fopen(Filename,"rb"))==NULL) return -1; 
  
 // Legge la Rom e restituisce la dimensione

 while ((data=fgetc(fRom))!=EOF) Rom[count++]=data;

 return ftell(fRom);
}



int createCvMemory(BYTE *Memory,char *BiosFile, char *RomFile)
{
 //****************
 // Setup del Bios 
 //****************
 
 BYTE *Bios = new BYTE[0x0800];
 BYTE *Rom  = new BYTE[0x4800]; 
 long  RomSize;
 
 //***************************
 // Carico il bios e la Rom
 //***************************
 
 if ((RomSize = loadRom(BiosFile,Bios))<0)
  { printf("Errore nel caricamento del BIOS %s\n",BiosFile); return 0; }
 if ((RomSize = loadRom(RomFile,Rom))<0)
  { printf("Errore nel caricamento della ROM %s\n",RomFile); return 0;}

 //***************************
 // Setto le copie del BIOS
 //***************************
 
 for (int Bank=0xc000; Bank<=0xf800; Bank+=0x800)
 for (int Offs=0; Offs<=0x7ff; Offs++)
 {
  Memory[Bank+Offs]=Bios[Offs]; 
  
 }
 
 switch (RomSize)
 {
  case 0x1000 : // Rom da 4K
   for (int i=0x0000; i<=0x0fff; i++)
   {
    Memory[0x9000+i]=Rom[i]; 
    Memory[0xb000+i]=Rom[i];
    }
   break;
  
  case 0x1800 : // Rom da 6K
   for (int i=0x0000; i<=0x07ff; i++) // i primi 2K...
   {
    Memory[0x9000+i]=Rom[i]; Memory[0x9800+i]=Rom[i];
    Memory[0xb000+i]=Rom[i]; Memory[0xb800+i]=Rom[i];
    }

   for (int i=0x0800; i<=0x17ff; i++) // .. gli altri 4K
   {
    Memory[0x8000+i]=Rom[i]; 
    Memory[0xa000+i]=Rom[i]; 
    }
   break;
 
  case 0x2000 : // Rom da 8K
  for (int i=0x0000; i<=0x1fff; i++)
   {
    Memory[0x8000+i]=Rom[i]; 
    Memory[0xa000+i]=Rom[i];
    }
  
   break;
 
  case 0x2800 : // Rom da 10K
  for (int i=0x0000; i<=0x1fff; i++)
   {
    Memory[0x8000+i]=Rom[i]; 
    Memory[0xa000+i]=Rom[i];
    }
  for (int i=0x2000; i<=0x27ff; i++)
   {
    Memory[0x5800+i]=Rom[i]; 
    Memory[0x7800+i]=Rom[i];
    }
  
   break;
   
  case 0x3000 : // Rom da 12K
  for (int i=0x0000; i<=0x1fff; i++)
   {
    Memory[0x8000+i]=Rom[i]; 
    Memory[0xa000+i]=Rom[i];
    }
  for (int i=0x2000; i<=0x2fff; i++)
   {
    Memory[0x3000+i]=Rom[i]; 
    Memory[0x5000+i]=Rom[i];
    }
   break;
   /*
  case 0x4800 : // Rom da 18K
  for (int i=0x0000; i<0x2000; i++)
  {
    Memory[0xa000+i]=Rom[i];
    Memory[0x8000+i]=Rom[i+0x2000];
  }
  for (int i=0x0000; i<0x0800; i++)
  {
   Memory[0x4800+i]=Rom[0x4000+i]; Memory[0x6800+i]=Rom[0x4000+i];
   Memory[0x5800+i]=Rom[0x4000+i]; Memory[0x7800+i]=Rom[0x4000+i];
   }
   break;
   */
  case 0x4800 : // Rom da 18K
  for (int i=0; i<0x4000; i++) Memory[0x8000+i]=Rom[i];
  for (int i=0x4000; i<0x4800; i++) Memory[0x4000+(i-0x4000)]=Rom[i];

  // Mirror the 2KB bank at 0x4000 throughout the 0x4000-0x7FFF range
  for (int Bank=0x4800; Bank<0x8000; Bank+=0x0800)
    memcpy(Memory+Bank, Memory+0x4000, 0x0800);
  break;

  default: printf("Dimensione della ROM %s non riconosciuta.\n",RomFile); return 0; break;
 
 }
 
 return 1;
 
} // Fine Costruttore di memoria





