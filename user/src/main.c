#include "stm32arduino.h"
#include "usbh_core.h"
#include "usbh_hid_usr.h"
#include "usbh_hid_core.h"
#include "usbh_hid_midi.h"
#include "time.h"

/* ���� -------------------------------------------------------------------	*/
/** @defgroup USBH_USR_MAIN_Private_Variables
* @{
*/
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core __ALIGN_END ;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST 	USB_Host __ALIGN_END ;
/**
* @}
*/ 

//extern volatile uint8_t TimeDisplayFlag;	// RTC���жϱ�־��λ�󣬸�����ʾ
extern uint8_t USBH_HID_EnumDone;			// ö�ٳɹ���־
extern USB_OTG_CORE_HANDLE	USB_OTG_Core_dev ;
/* �������� -------------------------------------------------------------------	*/
static void InitBoard(void);

/* ���� -----------------------------------------------------------------------	*/
void SYSTICK_Configuration(void)
{
	if(SysTick_Config(SystemCoreClock / 1000))
	while(1);
}
/*
*********************************************************************************************************
*	�� �� ��: InitBoard
*	����˵��: ��ʼ��Ӳ���豸
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitBoard(void)
{
	SYSTICK_Configuration();			// ����Systickʱ��1msϵͳʱ���ж�
	USART2_Init(9600);				// ���ô��ڣ�����printf���
	LED_Init();							// LED����
	delay_ms(200);
	
	USBH_Init(&USB_OTG_Core, 
		#ifdef USE_USB_OTG_FS
			USB_OTG_FS_CORE_ID,
		#else
			USB_OTG_HS_CORE_ID,
		#endif
			&USB_Host,
			&MIDI_cb, 		// usbh_hid_core.c
			&USR_Callbacks);
	
	#ifdef USE_USB_OTG_HS 
		//printf(" USB OTG HS HID Host\r\n");
	#else
		//printf(" USB OTG FS HID Host\r\n");
	#endif
	//printf("> USB Host library started\r\n");
	
}

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
extern uint8_t tx_flag;
extern uint8_t box_rx[301];
extern uint16_t box_rx_flag;

extern uint8_t sys_flag_ts;

uint16_t box_tx_flag = 0;

extern uint8_t app_adta;

extern uint16_t time_sec;
extern uint8_t time_f;

uint8_t sys_box[4];

int main(void) {
	InitBoard();	// ��ʼ��Ӳ���豸
	TimeInit();
	//printf("HZJT_OK!\r\n");
	while (1) {
		/* Host Task handler */
		USBH_Process(&USB_OTG_Core , &USB_Host);
		if(tx_flag == 1) {//���յ�������Ҫ����
			u8 box[3];
			box[0] = box_rx[box_tx_flag];
			box_tx_flag++;
			box[1] = box_rx[box_tx_flag];
			box_tx_flag++;
			box[2] = box_rx[box_tx_flag];
			box_tx_flag++;
			MIDI_SendData(box);
			if(box_tx_flag >= 300) {
				box_tx_flag = 0;
			}
			if(box_tx_flag == box_rx_flag) {
				tx_flag = 0;
			}
		}
		if(sys_flag_ts == 1) {
			sys_flag_ts = 0;
			LEDOff(LED1);
			if(app_adta == 0x7c) {
				sys_box[0] = 0x7c;
				sys_box[1] = time_f;
				sys_box[2] = (time_sec >> 8)& 0xff;;
				sys_box[3] = time_sec & 0xff;
			} else if(app_adta == 0x7b) {
				sys_box[0] = 0x7b;
				sys_box[1] = 0x00;
				sys_box[2] = 0x00;
				sys_box[3] = 0xdd;
			}
			usart_app();
		}
	}
}



#ifdef  USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
		ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{}
}
#endif

/**********************************END OF FILE**********************************/
