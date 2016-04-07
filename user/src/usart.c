/*
*********************************************************************************************************
*
*	ģ������ : ��������
*	�ļ����� : usart.c
*	��    �� : V1.0
*	˵    �� : ���ڿ���
*				ʵ��printf��scanf�����ض��򵽴���2����֧��printf��Ϣ��USART2
*				ʵ���ض���ֻ��Ҫ���2������:
*				int fputc(int ch, FILE *f);		int fgetc(FILE *f);
*				����KEIL MDK������������ѡ������Ҫ�� Options->Target�е�Code Generationѡ���е�use MicorLIBǰ��򹳣����򲻻������ݴ�ӡ�����ڡ�
*	�޸ļ�¼ :
*		�汾��  ����       	����    ˵��
*		V1.0	2013-12-12	JOY
*		V1.1	2014-03-21	JOY		�޸����ж����ȼ�
*		V1.2	2014-10-28	JOY		�޸�RX��GPIO��ΪGPIO_Mode_IPUģʽ����ֹ�ϵ�ʱRX���ֵ͵�ƽ�źš�
*
*********************************************************************************************************
*/
#include "usart.h"
#include "delay.h"
/*
*********************************************************************************************************
*	�� �� ��: fputc
*	����˵��: �ض���putc��������������ʹ��printf�����Ӵ���2��ӡ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
extern u8 midi_flag;
extern uint8_t buf_hc[30];
extern uint8_t buf_num;
extern uint8_t box_x[301];
extern uint16_t box_num;
uint16_t box_num_send = 0;
uint8_t send_flag = 0;
void usart_send(unsigned char ch)
{
	/* дһ���ֽڵ�USART2 */
	//USART_SendData(USART2, (unsigned char) ch);
	/* �ȴ����ͽ��� */
	//while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
//	delay_us(100);
	send_flag = 0;
	USART_ITConfig(USART2, USART_IT_TC, ENABLE); 
	USART_SendData(USART2, (unsigned char) box_x[box_num_send]);
	box_num_send++;
	if(box_num_send >= 300) {
		box_num_send = 0;
	}
}
/*
���ڷ���
*/
extern uint8_t sys_box[4];
uint8_t sys_bos_num;
void usart_app(void)
{
	send_flag = 1;
	USART_ITConfig(USART2, USART_IT_TC, ENABLE); 
	USART_SendData(USART2, (unsigned char) sys_box[0]);
	sys_bos_num = 1;
}
/*
*********************************************************************************************************
*	�� �� ��: fputc
*	����˵��: �ض���putc��������������ʹ��printf�����Ӵ���2��ӡ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
	/* дһ���ֽڵ�USART2 */
	//USART_SendData(USART2, (unsigned char) ch);

	/* �ȴ����ͽ��� */
	//while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
	//{}

	return ch;
}

/*
*********************************************************************************************************
*	�� �� ��: fgetc
*	����˵��: �ض���getc��������������ʹ��scanff�����Ӵ���2��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int fgetc(FILE *f)
{
	/* �ȴ�����1�������� */
	//while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USART2);
}

/*
*********************************************************************************************************
*	�� �� ��: USART2_NVIC
*	����˵��: USART2�����ж�����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void USART2_NVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); 
	
	/* Enable the USART2 Interrupt */ 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
}

/*
*********************************************************************************************************
*	�� �� ��: USART2_Init
*	����˵��: ���ڳ�ʼ��
*	��    ��: baud-������
*	�� �� ֵ: ��
*********************************************************************************************************
*/					    
void USART2_Init(uint32_t baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* ��1��: ��GPIO��USART������ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* ��2��: ��USART Tx��GPIO����Ϊ���츴��ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ��3��: ��USART Rx��GPIO����ΪGPIO_Mode_IPUģʽ����ֹ�ϵ�ʱRX���ֵ͵�ƽ�źš�*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ��4��: ����USART����
	    - ������   = 9600 baud
	    - ���ݳ��� = 8 Bits
	    - 1��ֹͣλ
	    - ��У��
	    - ��ֹӲ������(����ֹRTS��CTS)
	    - ʹ�ܽ��պͷ���
	*/
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	/* ��5��: ʹ�� USART�� ������� */
	USART_Cmd(USART2, ENABLE);

	//���ô����ж�
	USART2_NVIC();
	
	/* 
		CPU��Сȱ��: �������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������: 
	 	�巢����ɱ�־��Transmission Complete flag 
	*/
	USART_ClearFlag(USART2, USART_FLAG_TC);  
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);			// ����жϱ�־λ
	
	//�������ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//���������ж�
}

/*
*********************************************************************************************************
*	�� �� ��: USART2_IRQHandler
*	����˵��: ����2�����ж�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t box_rx[300];
uint16_t box_rx_flag = 0;
uint8_t start_flag = 0;
uint8_t tx_flag = 0;
uint8_t sys_flag = 0;
uint8_t sys_flag_ts = 0;

uint8_t app_adta = 0;

void USART2_IRQHandler(void) {
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		u8 data = 0;
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);			// ����жϱ�־λ	
		data = USART_ReceiveData(USART2);
		if( ( (data == 0x80)||(data == 0x90) ) && (start_flag == 0) ) {
			box_rx[box_rx_flag] = data;
			box_rx_flag++;
			start_flag = 1;
		} else if(start_flag == 1) {
			box_rx[box_rx_flag] = data;
			box_rx_flag++;
			start_flag = 2;
		} else if(start_flag == 2) {
			box_rx[box_rx_flag] = data;
			box_rx_flag++;
			start_flag = 0;
			tx_flag = 1;
		} else if( (data == 0x7c)||(data == 0x7b) ){
			sys_flag = 1;
			app_adta = data;
		} else if(sys_flag == 1) {
			if(data == 0x0a) {
				sys_flag = 0;
				sys_flag_ts = 1;
			} 
		}
		if(box_rx_flag >= 300) {
			box_rx_flag = 0;
		}
	}
	if(USART_GetITStatus(USART2, USART_IT_TC) != RESET) {
		USART_ClearITPendingBit(USART2,USART_IT_TC);
		if(send_flag == 0) {
			if(box_num_send == box_num) {
				USART_ITConfig(USART2, USART_IT_TC, DISABLE);
			} else {
				USART_SendData(USART2, (unsigned char) box_x[box_num_send]);
					box_num_send++;
				if(box_num_send >= 300) {
					box_num_send = 0;
				}
			}
		} else if(send_flag == 1) {
			if(sys_bos_num < 3) {
				USART_SendData(USART2,(unsigned char)sys_box[sys_bos_num]);
				sys_bos_num++;
			} else {
				sys_bos_num = 0;
				send_flag = 0;
				USART_ITConfig(USART2, USART_IT_TC, DISABLE);
			}
		}
	}
	
}
