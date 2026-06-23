/**********************************************************************

	Motorola 6821 PIA interface and emulation

	This function emulates all the functionality of up to 8 M6821
	peripheral interface adapters.

**********************************************************************/

#include <string.h>
#include <stdio.h>


#include <stdlib.h>
#include <SDL2/SDL.h>


#include "6821pia.h"
#include "../audio/sn76496.h"
#include "../include/defs.h"

int KeyOrJoy=0;
extern int LeftKeys,LeftJoy,RightKeys,RightJoy;
extern bool audio_enabled;

/******************* internal PIA data structure *******************/

struct pia6821
{
	const struct pia6821_interface *intf;
	unsigned char addr;

	unsigned char in_a;
	unsigned char in_ca1;
	unsigned char in_ca2;
	unsigned char out_a;
	unsigned char out_ca2;
	unsigned char ddr_a;
	unsigned char ctl_a;
	unsigned char irq_a1;
	unsigned char irq_a2;
	unsigned char irq_a_state;

	unsigned char in_b;
	unsigned char in_cb1;
	unsigned char in_cb2;
	unsigned char out_b;
	unsigned char out_cb2;
	unsigned char ddr_b;
	unsigned char ctl_b;
	unsigned char irq_b1;
	unsigned char irq_b2;
	unsigned char irq_b_state;
};


/******************* convenince macros and defines *******************/

#define PIA_IRQ1				0x80
#define PIA_IRQ2				0x40

#define IRQ1_ENABLED(c)			(c & 0x01)
#define IRQ1_DISABLED(c)		(!(c & 0x01))
#define C1_LOW_TO_HIGH(c)		(c & 0x02)
#define C1_HIGH_TO_LOW(c)		(!(c & 0x02))
#define OUTPUT_SELECTED(c)		(c & 0x04)
#define DDR_SELECTED(c)			(!(c & 0x04))
#define IRQ2_ENABLED(c)			(c & 0x08)
#define IRQ2_DISABLED(c)		(!(c & 0x08))
#define STROBE_E_RESET(c)		(c & 0x08)
#define STROBE_C1_RESET(c)		(!(c & 0x08))
#define SET_C2(c)				(c & 0x08)
#define RESET_C2(c)				(!(c & 0x08))
#define C2_LOW_TO_HIGH(c)		(c & 0x10)
#define C2_HIGH_TO_LOW(c)		(!(c & 0x10))
#define C2_SET_MODE(c)			(c & 0x10)
#define C2_STROBE_MODE(c)		(!(c & 0x10))
#define C2_OUTPUT(c)			(c & 0x20)
#define C2_INPUT(c)				(!(c & 0x20))

//*************************
// Chip sonoro
//*************************

extern struct sn76496 sn[MAX_76496];


/******************* static variables *******************/

struct pia6821 pia[MAX_PIA];

const unsigned char swizzle_address[4] = { 0, 2, 1, 3 };



/******************* stave state *******************/

static void update_6821_interrupts(struct pia6821 *p);

/******************* un-configuration *******************/

void pia_unconfig(void)
{
	memset(&pia, 0, sizeof(pia));
}


/******************* configuration *******************/

void pia_config(int which, int addressing, const struct pia6821_interface *intf)
{
	if (which >= MAX_PIA) return;
	pia[which].intf = intf;
	pia[which].addr = addressing;

}


/******************* reset *******************/

void pia_reset(void)
{
	int i;

	/* zap each structure, preserving the interface and swizzle */
	for (i = 0; i < MAX_PIA; i++)
	{
		const struct pia6821_interface *intf = pia[i].intf;
		unsigned char addr = pia[i].addr;

		memset(&pia[i], 0, sizeof(pia[i]));

		pia[i].intf = intf;
		pia[i].addr = addr;
	}
}


/******************* wire-OR for all interrupt handlers *******************/

static void update_shared_irq_handler(void (*irq_func)(int state))
{
	int i;

	/* search all PIAs for this same IRQ function */
	for (i = 0; i < MAX_PIA; i++)
		if (pia[i].intf)
		{
			/* check IRQ A */
			if (pia[i].intf->irq_a_func == irq_func && pia[i].irq_a_state)
			{
				(*irq_func)(1);
				return;
			}

			/* check IRQ B */
			if (pia[i].intf->irq_b_func == irq_func && pia[i].irq_b_state)
			{
				(*irq_func)(1);
				return;
			}
		}

	/* if we found nothing, the state is off */
	(*irq_func)(0);
}


/******************* external interrupt check *******************/

static void update_6821_interrupts(struct pia6821 *p)
{
	int new_state;

	/* start with IRQ A */
	new_state = 0;
	if ((p->irq_a1 && IRQ1_ENABLED(p->ctl_a)) || (p->irq_a2 && IRQ2_ENABLED(p->ctl_a))) new_state = 1;
	if (new_state != p->irq_a_state)
	{
		p->irq_a_state = new_state;
		if (p->intf->irq_a_func) update_shared_irq_handler(p->intf->irq_a_func);
	}

	/* then do IRQ B */
	new_state = 0;
	if ((p->irq_b1 && IRQ1_ENABLED(p->ctl_b)) || (p->irq_b2 && IRQ2_ENABLED(p->ctl_b))) new_state = 1;
	if (new_state != p->irq_b_state)
	{
		p->irq_b_state = new_state;
		if (p->intf->irq_b_func) update_shared_irq_handler(p->intf->irq_b_func);
	}
}


/******************* CPU interface for PIA read *******************/

int pia_read(int which, int offset)
{
	struct pia6821 *p = pia + which;
	int val = 0;


	/* adjust offset for 16-bit and ordering */
	offset &= 0x03;
	if (p->addr & PIA_ALTERNATE_ORDERING) offset = swizzle_address[offset];

	switch (offset)
	{
		/******************* port A output/DDR read *******************/
		case PIA_DDRA:

			/* read output register */
			if (OUTPUT_SELECTED(p->ctl_a))
			{
				/* update the input */
				if (p->intf->in_a_func) p->in_a = p->intf->in_a_func(0);

				/* combine input and output values */
				val = (p->out_a & p->ddr_a) + (p->in_a & ~p->ddr_a);

				/* IRQ flags implicitly cleared by a read */
				p->irq_a1 = p->irq_a2 = 0;
				update_6821_interrupts(p);

				/* CA2 is configured as output and in read strobe mode */
				if (C2_OUTPUT(p->ctl_a) && C2_STROBE_MODE(p->ctl_a))
				{
					/* this will cause a transition low; call the output function if we're currently high */
					if (p->out_ca2)
						if (p->intf->out_ca2_func) p->intf->out_ca2_func(0, 0);
					p->out_ca2 = 0;

					/* if the CA2 strobe is cleared by the E, reset it right away */
					if (STROBE_E_RESET(p->ctl_a))
					{
						if (p->intf->out_ca2_func) p->intf->out_ca2_func(0, 1);
						p->out_ca2 = 1;
					}
				}

			}

			/* read DDR register */
			else
			{
				val = p->ddr_a;
			}
			break;

		/******************* port B output/DDR read *******************/
		case PIA_DDRB:

			/* read output register */
			if (OUTPUT_SELECTED(p->ctl_b))
			{
				/* update the input */
				if (p->intf->in_b_func) p->in_b = p->intf->in_b_func(0);

				/* combine input and output values */
				val = (p->out_b & p->ddr_b) + (p->in_b & ~p->ddr_b);

				/* IRQ flags implicitly cleared by a read */
				p->irq_b1 = p->irq_b2 = 0;
				update_6821_interrupts(p);
			}

			/* read DDR register */
			else
			{
				val = p->ddr_b;
			}
			break;

		/******************* port A control read *******************/
		case PIA_CTLA:
   
			/* Update CA1 & CA2 if callback exists, these in turn may update IRQ's */
			if (p->intf->in_ca1_func) pia_set_input_ca1(which, p->intf->in_ca1_func(0));
			if (p->intf->in_ca2_func) pia_set_input_ca2(which, p->intf->in_ca2_func(0));

			/* read control register */
			val = p->ctl_a;

			/* set the IRQ flags if we have pending IRQs */
			if (p->irq_a1) val |= PIA_IRQ1;
			if (p->irq_a2 && C2_INPUT(p->ctl_a)) val |= PIA_IRQ2;

			break;

		/******************* port B control read *******************/
		case PIA_CTLB:

			/* Update CB1 & CB2 if callback exists, these in turn may update IRQ's */
			if (p->intf->in_cb1_func) pia_set_input_cb1(which, p->intf->in_cb1_func(0));
			if (p->intf->in_cb2_func) pia_set_input_cb2(which, p->intf->in_cb2_func(0));

			/* read control register */
            val = p->ctl_b ;

			/* set the IRQ flags if we have pending IRQs */
			if (p->irq_b1) val |= PIA_IRQ1;
			if (p->irq_b2 && C2_INPUT(p->ctl_b)) val |= PIA_IRQ2;

			break;
	}

	return val;
}


/******************* CPU interface for PIA write *******************/

void pia_write(int which, int offset, int data)
{
	struct pia6821 *p = pia + which;

	/* adjust offset for 16-bit and ordering */
	offset &= 3;
	if (p->addr & PIA_ALTERNATE_ORDERING) offset = swizzle_address[offset];

	switch (offset)
	{
		/******************* port A output/DDR write *******************/
		case PIA_DDRA:

			/* write output register */
			if (OUTPUT_SELECTED(p->ctl_a))
			{
				//LOG("PIA%d port A write = %02X\n",  which, data);

				/* update the output value */
				p->out_a = data;/* & p->ddr_a; */	/* NS990130 - don't mask now, DDR could change later */

				/* send it to the output function */
				if (p->intf->out_a_func && p->ddr_a) p->intf->out_a_func(0, p->out_a & p->ddr_a);	/* NS990130 */
			
            }

			/* write DDR register */
			else
			{
				//LOG("PIA%d DDR A write = %02X\n",  which, data);

				if (p->ddr_a != data)
				{
					/* NS990130 - if DDR changed, call the callback again */
					p->ddr_a = data;

					/* send it to the output function */
					if (p->intf->out_a_func && p->ddr_a) p->intf->out_a_func(0, p->out_a & p->ddr_a);
				}
			}
			break;

		/******************* port B output/DDR write *******************/
		case PIA_DDRB:

			/* write output register */
			if (OUTPUT_SELECTED(p->ctl_b))
			{
				//LOG("PIA%d port B write = %02X\n",  which, data);

				/* update the output value */
				p->out_b = data;/* & p->ddr_b */	/* NS990130 - don't mask now, DDR could change later */

				/* send it to the output function */
				if (p->intf->out_b_func && p->ddr_b) p->intf->out_b_func(0, p->out_b & p->ddr_b);	/* NS990130 */

				/* CB2 is configured as output and in write strobe mode */
				if (C2_OUTPUT(p->ctl_b) && C2_STROBE_MODE(p->ctl_b))
				{
					/* this will cause a transition low; call the output function if we're currently high */
					if (p->out_cb2)
						if (p->intf->out_cb2_func) p->intf->out_cb2_func(0, 0);
					p->out_cb2 = 0;

					/* if the CB2 strobe is cleared by the E, reset it right away */
					if (STROBE_E_RESET(p->ctl_b))
					{
						if (p->intf->out_cb2_func) p->intf->out_cb2_func(0, 1);
						p->out_cb2 = 1;
					}
				}
			}

			/* write DDR register */
			else
			{
				//LOG("PIA%d DDR B write = %02X\n",  which, data);

				if (p->ddr_b != data)
				{
					/* NS990130 - if DDR changed, call the callback again */
					p->ddr_b = data;

					/* send it to the output function */
					if (p->intf->out_b_func && p->ddr_b) p->intf->out_b_func(0, p->out_b & p->ddr_b);
				}
			}
			break;

		/******************* port A control write *******************/
		case PIA_CTLA:

			/* Bit 7 and 6 read only - PD 16/01/00 */

			data &= 0x3f;


			//LOG("PIA%d control A write = %02X\n",  which, data);

			/* CA2 is configured as output and in set/reset mode */
			/* 10/22/98 - MAB/FMP - any C2_OUTPUT should affect CA2 */
//			if (C2_OUTPUT(data) && C2_SET_MODE(data))
			if (C2_OUTPUT(data))
			{
				/* determine the new value */
				int temp = SET_C2(data) ? 1 : 0;

				/* if this creates a transition, call the CA2 output function */
				if (p->out_ca2 ^ temp)
					if (p->intf->out_ca2_func) p->intf->out_ca2_func(0, temp);

				/* set the new value */
				p->out_ca2 = temp;
			}

			/* update the control register */
			p->ctl_a = data;

			/* update externals */
			update_6821_interrupts(p);
			break;

		/******************* port B control write *******************/
		case PIA_CTLB:

			/* Bit 7 and 6 read only - PD 16/01/00 */

			data &= 0x3f;

			/* CB2 is configured as output and in set/reset mode */
			/* 10/22/98 - MAB/FMP - any C2_OUTPUT should affect CB2 */
//			if (C2_OUTPUT(data) && C2_SET_MODE(data))
			if (C2_OUTPUT(data))
			{
				/* determine the new value */
				int temp = SET_C2(data) ? 1 : 0;

				/* if this creates a transition, call the CA2 output function */
				if (p->out_cb2 ^ temp)
					if (p->intf->out_cb2_func) p->intf->out_cb2_func(0, temp);

				/* set the new value */
				p->out_cb2 = temp;
			}

			/* update the control register */
			p->ctl_b = data;

			/* update externals */
			update_6821_interrupts(p);
			break;
	}
	
}


/******************* interface setting PIA port A input *******************/

void pia_set_input_a(int which, int data)
{
	struct pia6821 *p = pia + which;

	/* set the input, what could be easier? */
	p->in_a = data;
}



/******************* interface setting PIA port CA1 input *******************/

void pia_set_input_ca1(int which, int data)
{
	struct pia6821 *p = pia + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* the new state has caused a transition */
	if (p->in_ca1 ^ data)
	{
		/* handle the active transition */
		if ((data && C1_LOW_TO_HIGH(p->ctl_a)) || (!data && C1_HIGH_TO_LOW(p->ctl_a)))
		{
			/* mark the IRQ */
			p->irq_a1 = 1;

			/* update externals */
			update_6821_interrupts(p);

			/* CA2 is configured as output and in read strobe mode and cleared by a CA1 transition */
			if (C2_OUTPUT(p->ctl_a) && C2_STROBE_MODE(p->ctl_a) && STROBE_C1_RESET(p->ctl_a))
			{
				/* call the CA2 output function */
				if (!p->out_ca2)
					if (p->intf->out_ca2_func) p->intf->out_ca2_func(0, 1);

				/* clear CA2 */
				p->out_ca2 = 1;
			}
		}
	}

	/* set the new value for CA1 */
	p->in_ca1 = data;
}



/******************* interface setting PIA port CA2 input *******************/

void pia_set_input_ca2(int which, int data)
{
	struct pia6821 *p = pia + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* CA2 is in input mode */
	if (C2_INPUT(p->ctl_a))
	{
		/* the new state has caused a transition */
		if (p->in_ca2 ^ data)
		{
			/* handle the active transition */
			if ((data && C2_LOW_TO_HIGH(p->ctl_a)) || (!data && C2_HIGH_TO_LOW(p->ctl_a)))
			{
				/* mark the IRQ */
				p->irq_a2 = 1;

				/* update externals */
				update_6821_interrupts(p);
			}
		}
	}

	/* set the new value for CA2 */
	p->in_ca2 = data;
}



/******************* interface setting PIA port B input *******************/

void pia_set_input_b(int which, int data)
{
	struct pia6821 *p = pia + which;

	/* set the input, what could be easier? */
	p->in_b = data;
}



/******************* interface setting PIA port CB1 input *******************/

void pia_set_input_cb1(int which, int data)
{
	struct pia6821 *p = pia + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* the new state has caused a transition */
	if (p->in_cb1 ^ data)
	{
		/* handle the active transition */
		if ((data && C1_LOW_TO_HIGH(p->ctl_b)) || (!data && C1_HIGH_TO_LOW(p->ctl_b)))
		{
			/* mark the IRQ */
			p->irq_b1 = 1;

			/* update externals */
			update_6821_interrupts(p);

			/* CB2 is configured as output and in write strobe mode and cleared by a CA1 transition */
			if (C2_OUTPUT(p->ctl_b) && C2_STROBE_MODE(p->ctl_b) && STROBE_C1_RESET(p->ctl_b))
			{
				/* the IRQ1 flag must have also been cleared */
				if (!p->irq_b1)
				{
					/* call the CB2 output function */
					if (!p->out_cb2)
						if (p->intf->out_cb2_func) p->intf->out_cb2_func(0, 1);

					/* clear CB2 */
					p->out_cb2 = 1;
				}
			}
		}
	}

	/* set the new value for CB1 */
	p->in_cb1 = data;
}



/******************* interface setting PIA port CB2 input *******************/

void pia_set_input_cb2(int which, int data)
{
	struct pia6821 *p = pia + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* CB2 is in input mode */
	if (C2_INPUT(p->ctl_b))
	{
		/* the new state has caused a transition */
		if (p->in_cb2 ^ data)
		{
			/* handle the active transition */
			if ((data && C2_LOW_TO_HIGH(p->ctl_b)) || (!data && C2_HIGH_TO_LOW(p->ctl_b)))
			{
				/* mark the IRQ */
				p->irq_b2 = 1;

				/* update externals */
				update_6821_interrupts(p);
			}
		}
	}

	/* set the new value for CA2 */
	p->in_cb2 = data;
}

//*******************************
// Funzioni per l'IO sul PIA    *
//*******************************

//*******************************
// Dati in input dalla porta A  *
// (Tape In/out)                * 
//*******************************

data8_t RdPiaPortA(offs_t A)
{
 // Used for tape input (Basic) - bits 4-7 for Tape In/Out
 // Not implemented: return inactive state
 return 0;
}


//***********************************
// Dati in input dalla porta B      *
// (Tastiera, audio sono in uscita) *
//***********************************

data8_t RdPiaPortB(offs_t B)
{
 // Keyboard/joystick read - all bits high means no key pressed
 
 switch (KeyOrJoy)
 {
  case 0x0F: break; // 1111 : Nothing to do
  
  case 0x0E:        // 1110 : PA0 low -> Left  Joystick
  return LeftJoy;
  
  case 0x0D:        // 1101 : PA1 low -> Left  Keyboard
  return LeftKeys;
  
  case 0x0B:        // 1011 : PA2 low -> Right Joystick
  return RightJoy;
  
  case 0x07:        // 0111 : PA3 low -> Right Keyboard
  return RightKeys;
  
  default: break;
 }

 return 0xFF;
}

//*******************************
// Controllo tastiera (keyscan) *
// e tape                       *
//*******************************



void WrPiaPortA(offs_t A , data8_t Value)
{
 // Output: bits 0-3 select keyboard half or joystick
 //         bits 5-7 = Tape (Pa6 = motor On/Off; Pa7 = Tape Data I/O) - not implemented

 /**************************/
 /*  Keyboard scan         */
 /**************************/

 // Pin Low         What
 //--------------------------------
 // PA3             right hand keys
 // PA2             right joystick
 // PA1             left hand keys
 // PA0             left joystick

 KeyOrJoy = (Value & 0x0f);

}

//***********************************
// Invio dati alla periferica audio *
//***********************************

void WrPiaPortB(offs_t B , data8_t Value)
{
 // La porta in output invia i dati al chip audio
	SDL_LockAudio();
  sn76496Write(0,Value);
	SDL_UnlockAudio();

//exit(0);



}

//********************************
// Abilitazione periferica audio *
//********************************

void WrPiaCb2(offs_t Cb2 , data8_t Value)
{
 // CB2 gates the SN76496: high = enabled, low = disabled
 audio_enabled = (Value != 0);
}

//**********************************
// Segnale di ready del chip audio *
//**********************************

// Segnale di controllo in input dalla CB1 (Dall'audio)
data8_t RdPiaCb1(offs_t Cb1)
{
 // Qui deve andare il ciclo audio che restituisce 1 quando � pronto
 

 return 0xff;

}


data8_t RdPiaCb2(offs_t Cb1)
{


 return 0xff;


}


