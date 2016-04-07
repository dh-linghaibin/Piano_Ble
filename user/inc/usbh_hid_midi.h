


#ifndef __USBH_MIDI_CORE_H
#define __USBH_MIDI_CORE_H


#include "usbh_core.h"
#include "usbh_stdreq.h"
#include "usb_bsp.h"
#include "usbh_ioreq.h"
#include "usbh_hcs.h"

typedef struct MIDI_cb
{
	void  (*Init)   (void);
	void  (*Decode) (uint8_t *data);
}
MIDI_cb_t;

/* States for MIDI State Machine */
typedef enum
{
	MIDI_IDLE= 0,
	MIDI_SEND_DATA,
	MIDI_BUSY,
	MIDI_GET_DATA,
	MIDI_POLL,
	MIDI_ERROR
}
MIDI_State_t;


#define USBH_MIDI_MPS_SIZE 64

/******************************************************************************/
/* Structure for MIDI process */
typedef struct _MIDI_Process
{
	MIDI_State_t	state;
	uint8_t			buff[USBH_MIDI_MPS_SIZE];
	uint8_t			hc_num_in;
	uint8_t 		hc_num_out;
	uint8_t			MIDIBulkOutEp;
	uint8_t			MIDIBulkInEp;
	uint16_t		MIDIBulkInEpSize;
	uint16_t		MIDIBulkOutEpSize;
	MIDI_cb_t      *cb;
}
MIDI_Machine_t;

extern USBH_Class_cb_TypeDef  MIDI_cb; 

uint8_t MIDI_RcvData(uint8_t *outBuf);
uint8_t MIDI_SendData(uint8_t *outBuf);

#endif /* __USBH_MIDI_CORE_H */



