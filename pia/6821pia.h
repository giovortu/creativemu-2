/**********************************************************************

	Motorola 6821 PIA interface and emulation

	This function emulates all the functionality of up to 4 M6821
	peripheral interface adapters.

**********************************************************************/

#ifndef PIA_6821
#define PIA_6821

#include "../include/memory.h"

#define MAX_PIA 1


/* this is the standard ordering of the registers */
/* alternate ordering swaps registers 1 and 2 */
#define	PIA_DDRA	0
#define	PIA_CTLA	1
#define	PIA_DDRB	2
#define	PIA_CTLB	3

/* PIA addressing modes */
#define PIA_STANDARD_ORDERING		0
#define PIA_ALTERNATE_ORDERING		1

struct pia6821_interface
{
	mem_read_handler in_a_func;
	mem_read_handler in_b_func;
	mem_read_handler in_ca1_func;
	mem_read_handler in_cb1_func;
	mem_read_handler in_ca2_func;
	mem_read_handler in_cb2_func;
	mem_write_handler out_a_func;
	mem_write_handler out_b_func;
	mem_write_handler out_ca2_func;
	mem_write_handler out_cb2_func;
	void (*irq_a_func)(int state);
	void (*irq_b_func)(int state);
};

#ifdef __cplusplus
extern "C" {
#endif

void    pia_unconfig(void);
void    pia_config(int which, int addressing, const struct pia6821_interface *intf);
void    pia_reset(void);
int     pia_read(int which, int offset);
void    pia_write(int which, int offset, int data);
void    pia_set_input_a(int which, int data);
void    pia_set_input_ca1(int which, int data);
void    pia_set_input_ca2(int which, int data);
void    pia_set_input_b(int which, int data);
void    pia_set_input_cb1(int which, int data);
void    pia_set_input_cb2(int which, int data);

data8_t RdPiaPortA(offs_t);
data8_t RdPiaPortB(offs_t);
data8_t RdPiaCa1(offs_t);
data8_t RdPiaCb1(offs_t);
data8_t RdPiaCa2(offs_t);
data8_t RdPiaCb2(offs_t);

void    WrPiaPortA(offs_t , data8_t);
void    WrPiaPortB(offs_t , data8_t);
void    WrPiaCa2(offs_t , data8_t);
void    WrPiaCb2(offs_t , data8_t);





#ifdef __cplusplus
}
#endif

#endif
