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

//********************************************
// Funzioni per il salvataggio / ripristino
// dello stato dell'emulazione.
//********************************************

#include "savestate.h"
#include <zlib.h>
#include "../pia/6821pia.h"

//**********************************************
// Save emulation state
//**********************************************

void SaveState(void)
{
    gzFile gz = gzopen("state.sav", "wb");

    if (gz)
    {
        gzwrite(gz, (void *)cvMemory,  sizeof(BYTE) * 0x10000);
        gzwrite(gz, (void *)Vdp,       sizeof(tms9918));
        gzwrite(gz, (void *)VdpRam,    sizeof(BYTE) * CV_VIDEO_WIDTH * CV_VIDEO_HEIGHT);
        /* Fix: use sizeof(struct pia6821) * MAX_PIA, not sizeof(pointer) */
        gzwrite(gz, (void *)pia,       sizeof(struct pia6821) * MAX_PIA);

        gzwrite(gz, (void *)&a_reg,    sizeof(BYTE));
        gzwrite(gz, (void *)&x_reg,    sizeof(BYTE));
        gzwrite(gz, (void *)&y_reg,    sizeof(BYTE));
        gzwrite(gz, (void *)&flag_reg, sizeof(BYTE));
        gzwrite(gz, (void *)&s_reg,    sizeof(BYTE));
        /* Fix: pc_reg is a WORD (2 bytes), not a BYTE */
        gzwrite(gz, (void *)&pc_reg,   sizeof(WORD));

        gzclose(gz);
        printf("State saved to state.sav\n");
    }
    else
    {
        printf("Error: could not open state.sav for writing\n");
    }
}

//**********************************************
// Load emulation state
//**********************************************

void LoadState(void)
{
    gzFile gz = gzopen("state.sav", "rb");

    if (gz)
    {
        gzread(gz, (void *)cvMemory,  sizeof(BYTE) * 0x10000);
        gzread(gz, (void *)Vdp,       sizeof(tms9918));
        gzread(gz, (void *)VdpRam,    sizeof(BYTE) * CV_VIDEO_WIDTH * CV_VIDEO_HEIGHT);
        /* Fix: use sizeof(struct pia6821) * MAX_PIA, not sizeof(pointer) */
        gzread(gz, (void *)pia,       sizeof(struct pia6821) * MAX_PIA);

        gzread(gz, (void *)&a_reg,    sizeof(BYTE));
        gzread(gz, (void *)&x_reg,    sizeof(BYTE));
        gzread(gz, (void *)&y_reg,    sizeof(BYTE));
        gzread(gz, (void *)&flag_reg, sizeof(BYTE));
        gzread(gz, (void *)&s_reg,    sizeof(BYTE));
        /* Fix: pc_reg is a WORD (2 bytes), not a BYTE */
        gzread(gz, (void *)&pc_reg,   sizeof(WORD));

        gzclose(gz);
        printf("State loaded from state.sav\n");
    }
    else
    {
        printf("Error: could not open state.sav for reading\n");
    }
}
