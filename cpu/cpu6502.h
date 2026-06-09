/* assumes WORD = 16bit, BYTE = 8bit!! */

#ifndef __CPU6502__
#define __CPU6502__

#include "../include/defs.h"
#include "../video/tms9918.h"

#include <SDL2/SDL.h>

/* Macros for convenience */
#define A a_reg
#define X x_reg
#define Y y_reg
#define P flag_reg
#define S s_reg
#define PC pc_reg

extern tms9918 *Vdp;
extern unsigned int Ticks;
extern unsigned char VdpIrq;




extern void UpdateVideo();

/* Initialize 6502 */
void init6502();

/* Reset CPU */
void reset6502();

/* Non maskerable interrupt */
void nmi6502();

/* Maskerable Interrupt */
void irq6502();

/* Execute Instruction */
void exec6502(int timerTicks);

/* cvMemory functions */
int  get6502memory(WORD addr);
void put6502memory(WORD addr, BYTE byte);


#endif
