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
case 0x1000 : // Rom da 4K (Dimensione: 0x1000)
   memcpy(&Memory[0x9000], &Rom[0x0000], 0x1000); 
   memcpy(&Memory[0xB000], &Rom[0x0000], 0x1000);
   break;
  
  case 0x1800 : // Rom da 6K
   // I primi 2K (Dimensione: 0x0800) copiati e specchiati
   memcpy(&Memory[0x9000], &Rom[0x0000], 0x0800); 
   memcpy(&Memory[0x9800], &Rom[0x0000], 0x0800);
   memcpy(&Memory[0xB000], &Rom[0x0000], 0x0800); 
   memcpy(&Memory[0xB800], &Rom[0x0000], 0x0800);

   // Gli altri 4K (Dimensione: 0x1000, da 0x0800 a 0x17FF)
   memcpy(&Memory[0x8000 + 0x0800], &Rom[0x0800], 0x1000); 
   memcpy(&Memory[0xA000 + 0x0800], &Rom[0x0800], 0x1000); 
   break;
 
  case 0x2000 : // Rom da 8K (Dimensione: 0x2000)
   memcpy(&Memory[0x8000], &Rom[0x0000], 0x2000); 
   memcpy(&Memory[0xA000], &Rom[0x0000], 0x2000);
   break;
 
  case 0x2800 : // Rom da 10K
   // Primi 8K (Dimensione: 0x2000)
   memcpy(&Memory[0x8000], &Rom[0x0000], 0x2000); 
   memcpy(&Memory[0xA000], &Rom[0x0000], 0x2000);
   
   // Ultimi 2K (Dimensione: 0x0800, da 0x2000 a 0x27FF)
   // Nota: nel tuo for originale facevi Memory[0x5800 + i], poiché i parte da 0x2000, 
   // la scrittura effettiva inizia da 0x5800 + 0x2000 = 0x7800. 
   // Stessa cosa per l'altro blocco: 0x7800 + 0x2000 = 0x9800.
   memcpy(&Memory[0x7800], &Rom[0x2000], 0x0800); 
   memcpy(&Memory[0x9800], &Rom[0x2000], 0x0800);
   break;
   
  case 0x3000 : // Rom da 12K
   // Primi 8K (Dimensione: 0x2000)
   memcpy(&Memory[0x8000], &Rom[0x0000], 0x2000); 
   memcpy(&Memory[0xA000], &Rom[0x0000], 0x2000);
   
   // Ultimi 4K (Dimensione: 0x1000, da 0x2000 a 0x2FFF)
   // Nota come sopra: 0x3000 + 0x2000 (offset di partenza di i) = 0x5000
   // E per il secondo blocco: 0x5000 + 0x2000 = 0x7000
   memcpy(&Memory[0x5000], &Rom[0x2000], 0x1000); 
   memcpy(&Memory[0x7000], &Rom[0x2000], 0x1000);
   break;
   
  case 0x4800 : // Rom da 18K
   // 1. Copia i 16K principali a 0x8000
   memcpy(&Memory[0x8000], &Rom[0x0000], 0x4000);

   // 2. Copia i primi 2K extra a 0x4000
   memcpy(&Memory[0x4000], &Rom[0x4000], 0x0800);

   // 3. Specchia progressivamente i 2K per riempire i 16K dell'area 0x4000-0x7FFF
   memcpy(&Memory[0x4800], &Memory[0x4000], 0x0800); // Raddoppia a 4K (fino a 0x4FFF)
   memcpy(&Memory[0x5000], &Memory[0x4000], 0x1000); // Raddoppia a 8K (fino a 0x5FFF)
   memcpy(&Memory[0x6000], &Memory[0x4000], 0x2000); // Raddoppia a 16K (fino a 0x7FFF)
   break;


  default: printf("Dimensione della ROM %s non riconosciuta.\n",RomFile); return 0; break;
 
 }
 
 return 1;
 
} // Fine Costruttore di memoria





