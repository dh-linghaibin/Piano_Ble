
#include "usbh_hid_midi.h"
#include "stdio.h"
#include "usart.h"
#include "stm32arduino.h"

MIDI_Machine_t	MIDI_Machine;

extern __ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core __ALIGN_END ;

static USBH_Status 	USBH_MIDI_InterfaceInit(USB_OTG_CORE_HANDLE *pdev , void *phost);

static void 		USBH_MIDI_InterfaceDeInit(USB_OTG_CORE_HANDLE *pdev , void *phost);

static USBH_Status 	USBH_MIDI_ClassRequest(USB_OTG_CORE_HANDLE *pdev , void *phost);

static USBH_Status 	USBH_MIDI_Handle(USB_OTG_CORE_HANDLE *pdev , void *phost);

void MIDI_Decode(uint8_t * outBuf);
	
__IO uint8_t 				start_toggle = 0;

USBH_Class_cb_TypeDef	MIDI_cb =
{
		USBH_MIDI_InterfaceInit,
		USBH_MIDI_InterfaceDeInit,
		USBH_MIDI_ClassRequest,
		USBH_MIDI_Handle
};


static USBH_Status USBH_MIDI_InterfaceInit ( USB_OTG_CORE_HANDLE *pdev,
		void *phost)
{
	int iface = 0;
	USBH_HOST *pphost = phost;
	// USBH_Status status = USBH_BUSY ;
	MIDI_Machine.state = MIDI_ERROR;
	printf("midi_ok\n");
	for(iface=0; iface < pphost->device_prop.Cfg_Desc.bNumInterfaces && iface < USBH_MAX_NUM_INTERFACES; iface++) // looking for a MIDI interface
	{
		if((pphost->device_prop.Itf_Desc[iface].bInterfaceClass == USB_AUDIO_CLASS) && \
				(pphost->device_prop.Itf_Desc[iface].bInterfaceSubClass == USB_MIDISTREAMING_SubCLASS))

		{
			if(pphost->device_prop.Ep_Desc[iface][0].bEndpointAddress & 0x80)
			{
				MIDI_Machine.MIDIBulkInEp      = (pphost->device_prop.Ep_Desc[iface][0].bEndpointAddress);
				MIDI_Machine.MIDIBulkInEpSize  =  pphost->device_prop.Ep_Desc[iface][0].wMaxPacketSize;
			}
			else
			{
				MIDI_Machine.MIDIBulkOutEp      = (pphost->device_prop.Ep_Desc[iface][0].bEndpointAddress);
				MIDI_Machine.MIDIBulkOutEpSize  =  pphost->device_prop.Ep_Desc[iface][0].wMaxPacketSize;
			}

			if(pphost->device_prop.Ep_Desc[iface][1].bEndpointAddress & 0x80)
			{
				MIDI_Machine.MIDIBulkInEp      = (pphost->device_prop.Ep_Desc[iface][1].bEndpointAddress);
				MIDI_Machine.MIDIBulkInEpSize  =  pphost->device_prop.Ep_Desc[iface][1].wMaxPacketSize;
			}
			else
			{
				MIDI_Machine.MIDIBulkOutEp      = (pphost->device_prop.Ep_Desc[iface][1].bEndpointAddress);
				MIDI_Machine.MIDIBulkOutEpSize  =  pphost->device_prop.Ep_Desc[iface][1].wMaxPacketSize;
			}

			MIDI_Machine.hc_num_out = USBH_Alloc_Channel(pdev, MIDI_Machine.MIDIBulkOutEp);
			MIDI_Machine.hc_num_in  = USBH_Alloc_Channel(pdev, MIDI_Machine.MIDIBulkInEp);

			/* Open the new channels */
			USBH_Open_Channel  (pdev,
					MIDI_Machine.hc_num_out,
					pphost->device_prop.address,
					pphost->device_prop.speed,
					EP_TYPE_BULK,
					MIDI_Machine.MIDIBulkOutEpSize);

			USBH_Open_Channel  (pdev,
					MIDI_Machine.hc_num_in,
					pphost->device_prop.address,
					pphost->device_prop.speed,
					EP_TYPE_BULK,
					MIDI_Machine.MIDIBulkInEpSize);

			MIDI_Machine.state  = MIDI_GET_DATA;
			start_toggle =0;
			return USBH_OK;
		}
	}

	pphost->usr_cb->DeviceNotSupported();
	return USBH_FAIL;
}


/*-----------------------------------------------------------------------------------------*/
/**
 * @brief  USBH_MIDI_InterfaceDeInit
 *         The function DeInit the Host Channels used for the MIDI class.
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval None
 */
void USBH_MIDI_InterfaceDeInit ( USB_OTG_CORE_HANDLE *pdev,
		void *phost)
{
	if ( MIDI_Machine.hc_num_out)
	{
		USB_OTG_HC_Halt(pdev, MIDI_Machine.hc_num_out);
		USBH_Free_Channel  (pdev, MIDI_Machine.hc_num_out);
		MIDI_Machine.hc_num_out = 0;     /* Reset the Channel as Free */
	}

	if ( MIDI_Machine.hc_num_in)
	{
		USB_OTG_HC_Halt(pdev, MIDI_Machine.hc_num_in);
		USBH_Free_Channel  (pdev, MIDI_Machine.hc_num_in);
		MIDI_Machine.hc_num_in = 0;     /* Reset the Channel as Free */
	}
	start_toggle = 0;
}


/*-----------------------------------------------------------------------------------------*/
/**
 * @brief  USBH_MIDI_ClassRequest
 *         The function is responsible for handling MIDI Class requests
 *         for MIDI class.
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval  USBH_Status :Response for USB Set Protocol request
 */
static USBH_Status USBH_MIDI_ClassRequest(USB_OTG_CORE_HANDLE *pdev ,
		void *phost)
{
	USBH_Status status = USBH_OK ;

	return status;
}


/*-----------------------------------------------------------------------------------------*/
/**
 * @brief  USBH_MIDI_Handle
 *         The function is for managing state machine for MIDI data transfers
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval USBH_Status
 */
static USBH_Status USBH_MIDI_Handle(USB_OTG_CORE_HANDLE *pdev ,	void   *phost)
{
	USBH_HOST *pphost = phost;
	USBH_Status status = USBH_OK;
	if(HCD_IsDeviceConnected(pdev))
	{
		//appliStatus = pphost->usr_cb->UserApplication(); // this will call USBH_USR_MIDI_Application()
		switch (MIDI_Machine.state)
		{
			case MIDI_GET_DATA:
			USBH_BulkReceiveData( &USB_OTG_Core, MIDI_Machine.buff, 64, MIDI_Machine.hc_num_in);
			start_toggle = 1;
			MIDI_Machine.state = MIDI_POLL;
			break;

			case MIDI_POLL:
			if(HCD_GetURB_State(pdev , MIDI_Machine.hc_num_in) <= URB_DONE)
			{
				if(start_toggle == 1) /* handle data once */
				{
					start_toggle = 0;
					//判断有没有接收到数据
					MIDI_Decode(MIDI_Machine.buff);
					//MIDI_Machine.buff[1] = 0; // the whole buffer should be cleaned...
				}
				MIDI_Machine.state = MIDI_GET_DATA;
			}
			else if(HCD_GetURB_State(pdev, MIDI_Machine.hc_num_in) == URB_STALL) /* IN Endpoint Stalled */
			{
				/* Issue Clear Feature on IN endpoint */
				if( (USBH_ClrFeature(pdev,
				pphost,
				MIDI_Machine.MIDIBulkInEp,
				MIDI_Machine.hc_num_in)) == USBH_OK)
				{
					/* Change state to issue next IN token */
					MIDI_Machine.state = MIDI_GET_DATA;
				}
			}
			break;

			default:
			break;
		}
	}
	return status;
}

/*-----------------------------------------------------------------------------------------*/
/* look up a MIDI message size from spec */
/*Return */
/* 0 : undefined message */
/* > 0 : valid message size (1-3) */
uint8_t MIDI_lookupMsgSize(uint8_t midiMsg)
{
	uint8_t msgSize = 0;

	if( midiMsg < 0xf0 ) midiMsg &= 0xf0;
	switch(midiMsg) {
	//3 bytes messages
	case 0xf2 : //system common message(SPP)
	case 0x80 : //Note off
	case 0x90 : //Note on
	case 0xa0 : //Poly KeyPress
	case 0xb0 : //Control Change
	case 0xe0 : //PitchBend Change
		msgSize = 3;
		break;

		//2 bytes messages
	case 0xf1 : //system common message(MTC)
	case 0xf3 : //system common message(SongSelect)
	case 0xc0 : //Program Change
	case 0xd0 : //Channel Pressure
		msgSize = 2;
		break;

		//1 bytes messages
	case 0xf8 : //system realtime message
	case 0xf9 : //system realtime message
	case 0xfa : //system realtime message
	case 0xfb : //system realtime message
	case 0xfc : //system realtime message
	case 0xfe : //system realtime message
	case 0xff : //system realtime message
		msgSize = 1;
		break;

		//undefine messages
	default :
		break;
	}
	return msgSize;
}


/*-----------------------------------------------------------------------------------------*/

/* Receive data from MIDI device */
uint8_t MIDI_RcvData(uint8_t *outBuf)
{
	URB_STATE urb_status;
	urb_status = HCD_GetURB_State( &USB_OTG_Core , MIDI_Machine.hc_num_in);

	if(urb_status <= URB_DONE)
	{
		USBH_BulkReceiveData( &USB_OTG_Core, MIDI_Machine.buff, 64, MIDI_Machine.hc_num_in);//64
		if( MIDI_Machine.buff[0] == 0 && MIDI_Machine.buff[1] == 0 && MIDI_Machine.buff[2] == 0 && MIDI_Machine.buff[3] == 0 )
			return 0;
		outBuf[0] = MIDI_Machine.buff[1];
		outBuf[1] = MIDI_Machine.buff[2];
		outBuf[2] = MIDI_Machine.buff[3];
		//printf("Send %x--%x--%x\n",outBuf[0],outBuf[1],outBuf[2]);
		return MIDI_lookupMsgSize(MIDI_Machine.buff[1]);
	}
	else return 0;
}

uint8_t MIDI_SendData(uint8_t *outBuf)
{
	URB_STATE urb_status;
	urb_status = HCD_GetURB_State( &USB_OTG_Core , MIDI_Machine.hc_num_out);
	MIDI_Machine.buff[1] = outBuf[0];
	MIDI_Machine.buff[2] = outBuf[1];
	MIDI_Machine.buff[3] = outBuf[2];
	if(urb_status <= URB_DONE)
	{
		USBH_CtlSendData( &USB_OTG_Core, MIDI_Machine.buff, 64, MIDI_Machine.hc_num_out);
	}
	return 0x00;
}

u8 midi_flag = 0;
uint8_t buf_hc[30];
uint8_t buf_num = 0;
uint8_t box_x[301];
uint16_t box_num = 0;

extern uint16_t time_sec;
extern uint8_t time_f;

//midi数据处理
void MIDI_Decode(uint8_t * outBuf)
{
	URB_STATE urb_status;
	urb_status = HCD_GetURB_State( &USB_OTG_Core , MIDI_Machine.hc_num_in);
	if(urb_status <= URB_DONE)
	{
		USBH_BulkReceiveData( &USB_OTG_Core, MIDI_Machine.buff, 64, MIDI_Machine.hc_num_in);//64
		if( MIDI_Machine.buff[0] == 0 && MIDI_Machine.buff[1] == 0 && MIDI_Machine.buff[2] == 0 && MIDI_Machine.buff[3] == 0 ) {
		
		} else {
			for(buf_num = 1;buf_num < 4;buf_num++) {
				if(buf_hc[buf_num] != MIDI_Machine.buff[buf_num]) {
					uint8_t xx = 0;
					for(xx = 1;xx < 4;xx++) {
						buf_hc[xx] = MIDI_Machine.buff[xx];
					}
					box_x[box_num] = buf_hc[1];
					box_num++;
					box_x[box_num] = buf_hc[2];
					box_num++;
					box_x[box_num] = buf_hc[3];
					box_num++;
					box_x[box_num] = time_f;
					box_num++;
					box_x[box_num] = (time_sec >> 8)& 0xff;;
					box_num++;
					box_x[box_num] = time_sec & 0xff;
					box_num++;
					if(box_num >= 300) {
						box_num = 0;
						midi_flag = 1;
					}
					usart_send(0x00);
					LEDToggle(LED1);
					//midi_flag = 1;
					//usart_send(MIDI_Machine.buff[1]);
					//usart_send(MIDI_Machine.buff[2]);
					//usart_send(MIDI_Machine.buff[3]);
					//usart_send(0x00);
					//usart_send(0x00);
					break;
				}
			}
		}
	}
}
