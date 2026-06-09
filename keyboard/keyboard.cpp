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

#include <SDL2/SDL.h>

extern int  LeftKeys,LeftJoy,RightKeys,RightJoy;


void CheckCvKeysDown( int KeySymbol )
{
     switch ( KeySymbol )
	 {
        case SDLK_z:         LeftKeys&=0xf5; break;
        case SDLK_a:         LeftKeys&=0xee; break;
        case SDLK_q:         LeftKeys&=0xe7; break;
        case SDLK_2:         LeftKeys&=0xcf; break;
        case SDLK_x:         LeftKeys&=0xed; break;
        case SDLK_s:         LeftKeys&=0xde; break;
        case SDLK_w:         LeftKeys&=0xf3; break;
        case SDLK_3:         LeftKeys&=0x9f; break;
        case SDLK_c:         LeftKeys&=0xdd; break;
        case SDLK_d:         LeftKeys&=0xbe; break;
        case SDLK_e:         LeftKeys&=0xeb; break;
        case SDLK_4:         LeftKeys&=0xd7; break;
        case SDLK_v:         LeftKeys&=0xbd; break;
        case SDLK_f:         LeftKeys&=0xfc; break;
        case SDLK_r:         LeftKeys&=0xdb; break;
        case SDLK_5:         LeftKeys&=0xb7; break;
        case SDLK_b:         LeftKeys&=0xf9; break;
        case SDLK_g:         LeftKeys&=0xfa; break;
        case SDLK_t:         LeftKeys&=0xbb; break;
        case SDLK_6:         LeftKeys&=0xaf; break;

        case SDLK_LSHIFT:    LeftKeys&=0x7f; break;
        case SDLK_RSHIFT:    LeftKeys&=0x7f; break;

        case SDLK_BACKSPACE: LeftKeys&=0xf6; break; // tasto <-

        case SDLK_LCTRL:     LeftJoy &=0x7f; break;

        case SDLK_1:         LeftJoy &=0xf3; break;

        case SDLK_DOWN:      LeftJoy &=0xfd; break;
        case SDLK_UP:        LeftJoy &=0xf7; break;
        case SDLK_LEFT:      LeftJoy &=0xdf; break;
        case SDLK_RIGHT:     LeftJoy &=0xfb; break;


        //case SDLK_LEFTBRACKET:     RightKeys&=0xcf; break;

        //case SDLK_EQUALS:    RightKeys&=0xe7; break;

        case SDLK_QUOTE:     RightKeys&=0xf5; break;
        case SDLK_p:         RightKeys&=0xee; break;
        case SDLK_SEMICOLON: RightKeys&=0xe7; break;
        case SDLK_SLASH:     RightKeys&=0xcf; break;//
        case SDLK_0:         RightKeys&=0xed; break;
        case SDLK_o:         RightKeys&=0xde; break;
        case SDLK_l:         RightKeys&=0xf3; break;
        case SDLK_PERIOD:    RightKeys&=0x9f; break;// .
        case SDLK_9:         RightKeys&=0xdd; break;
        case SDLK_i:         RightKeys&=0xbe; break;
        case SDLK_k:         RightKeys&=0xeb; break;
        case SDLK_COMMA:     RightKeys&=0xd7; break;// ,
        case SDLK_8:         RightKeys&=0xbd; break;
        case SDLK_u:         RightKeys&=0xfc; break;
        case SDLK_j:         RightKeys&=0xdb; break;
        case SDLK_m:         RightKeys&=0xb7; break;
        case SDLK_7:         RightKeys&=0xf9; break;
        case SDLK_y:         RightKeys&=0xfa; break;
        case SDLK_h:         RightKeys&=0xbb; break;
        case SDLK_n:         RightKeys&=0xaf; break;
        case SDLK_RETURN:    RightKeys&=0xf6; break;

        case SDLK_SPACE:     RightJoy &=0xf3; break;
        //case SDLK_TAB:       RightJoy &=0x7f; break;

       default: break;
	  }

}

void CheckCvKeysUp( int KeySymbol )
{

        switch ( KeySymbol )
        {
        case SDLK_z:         LeftKeys|=~0xf5; break;
        case SDLK_a:         LeftKeys|=~0xee; break;
        case SDLK_q:         LeftKeys|=~0xe7; break;
        case SDLK_2:         LeftKeys|=~0xcf; break;
        case SDLK_x:         LeftKeys|=~0xed; break;
        case SDLK_s:         LeftKeys|=~0xde; break;
        case SDLK_w:         LeftKeys|=~0xf3; break;
        case SDLK_3:         LeftKeys|=~0x9f; break;
        case SDLK_c:         LeftKeys|=~0xdd; break;
        case SDLK_d:         LeftKeys|=~0xbe; break;
        case SDLK_e:         LeftKeys|=~0xeb; break;
        case SDLK_4:         LeftKeys|=~0xd7; break;
        case SDLK_v:         LeftKeys|=~0xbd; break;
        case SDLK_f:         LeftKeys|=~0xfc; break;
        case SDLK_r:         LeftKeys|=~0xdb; break;
        case SDLK_5:         LeftKeys|=~0xb7; break;
        case SDLK_b:         LeftKeys|=~0xf9; break;
        case SDLK_g:         LeftKeys|=~0xfa; break;
        case SDLK_t:         LeftKeys|=~0xbb; break;
        case SDLK_6:         LeftKeys|=~0xaf; break;

        case SDLK_LSHIFT:    LeftKeys|=~0x7f; break;
        case SDLK_RSHIFT:    LeftKeys|=~0x7f; break;

        case SDLK_BACKSPACE: LeftKeys|=~0xf6; break; // tasto <-

        case SDLK_LCTRL:     LeftJoy |=~0x7f; break;

        case SDLK_1:         LeftJoy |=~0xf3; break;

        case SDLK_DOWN:      LeftJoy |=~0xfd; break;
        case SDLK_UP:        LeftJoy |=~0xf7; break;
        case SDLK_LEFT:      LeftJoy |=~0xdf; break;
        case SDLK_RIGHT:     LeftJoy |=~0xfb; break;


        //case SDLK_LEFTBRACKET:     RightKeys|=~0xcf; break;

        //case SDLK_EQUALS:    RightKeys|=~0xe7; break;

        case SDLK_QUOTE:     RightKeys|=~0xf5; break;
        case SDLK_p:         RightKeys|=~0xee; break;
        case SDLK_SEMICOLON: RightKeys|=~0xe7; break;
        case SDLK_SLASH:     RightKeys|=~0xcf; break;//
        case SDLK_0:         RightKeys|=~0xed; break;
        case SDLK_o:         RightKeys|=~0xde; break;
        case SDLK_l:         RightKeys|=~0xf3; break;
        case SDLK_PERIOD:    RightKeys|=~0x9f; break;// .
        case SDLK_9:         RightKeys|=~0xdd; break;
        case SDLK_i:         RightKeys|=~0xbe; break;
        case SDLK_k:         RightKeys|=~0xeb; break;
        case SDLK_COMMA:     RightKeys|=~0xd7; break;// ,
        case SDLK_8:         RightKeys|=~0xbd; break;
        case SDLK_u:         RightKeys|=~0xfc; break;
        case SDLK_j:         RightKeys|=~0xdb; break;
        case SDLK_m:         RightKeys|=~0xb7; break;
        case SDLK_7:         RightKeys|=~0xf9; break;
        case SDLK_y:         RightKeys|=~0xfa; break;
        case SDLK_h:         RightKeys|=~0xbb; break;
        case SDLK_n:         RightKeys|=~0xaf; break;
        case SDLK_RETURN:    RightKeys|=~0xf6; break;

        case SDLK_SPACE:     RightJoy |=~0xf3; break;
        //case SDLK_TAB:       RightJoy |=~0x7f; break;

       default: break; }
} 
