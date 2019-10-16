// -------------------------------------------------------------------------------------
/**
  * @file    LwIP/LwIP_TCP_Echo_Client/Src/tcp_echoclient.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    31-July-2015 
  * @brief   tcp echoclient application using LwIP RAW API
  *
  *  COPYRIGHT(c) 2015 STMicroelectronics
  *
  * This file was modified by Jae Il Kim, Sang Min Kim, and Donghyuk Cha
			April 15. 2016
  */
  // -------------------------------------------------------------------------------------



// -- <1> �ʿ��� ��� ������ ��Ŭ��� 
#include "main.h"
#include "Nucleo_F429.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"

#include <stdio.h>
#include <string.h>

#if LWIP_TCP

// -- <2> ����, �Լ��� ���� 
struct tcp_pcb 	*echoclient_pcb;
u8_t   				data[100];

// ECHO protocol states  
enum  echoclient_states
{
	  ES_NOT_CONNECTED = 0,
	  ES_CONNECTED,
	  ES_RECEIVED,
	  ES_CLOSING,
};

// structure to be passed as argument to the tcp callbacks 
struct  echoclient
{
	  enum echoclient_states 	state; 			/* connection status */
	  struct tcp_pcb 				*pcb;          	/* pointer on the current tcp_pcb */
	  struct pbuf						*p_tx;			/* pointer on pbuf to be transmitted */
};


// -- <3> �Լ��� ���� 
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

void tcp_echoclient_connect(void);

void key_send(int key_number);
void message_send_string(struct tcp_pcb *tpcb2, char *str);


// -------------------------------------------------------------------------
// -- <4> Server�� ����(TCP connect)�ϴ� �Լ� 
//				- �� �Լ��� ������ �ʿ���� �״�� ����ϸ� �ȴ�.
//
// -------------------------------------------------------------------------

void tcp_echoclient_connect(void)
{
		struct ip4_addr DestIPaddr;
		
		// -- <4-1> create new tcp pcb 
		echoclient_pcb = tcp_new();
		
		if (echoclient_pcb != NULL)  {
			// -- <4-2> ������ server�� IP�� ������ -	
			IP4_ADDR( &DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );
			// -- <4-3> ������ IP(&DestIPaddr), ��Ʈ(DEST_PORT)�� ������
			tcp_connect(echoclient_pcb, &DestIPaddr, DEST_PORT, tcp_echoclient_connected);

		}
		else  {
			/* deallocate the pcb */
			memp_free(MEMP_TCP_PCB, echoclient_pcb);
#ifdef SERIAL_DEBUG
			printf("\n\r can not create tcp pcb");
#endif 
		}
}

// ------------------------------------------------------------------------------------
//  -- <5> ������ TCP connection�� �Ϸ�Ǹ� ȣ��Ǵ� �ݹ� �Լ� 
//				- �� �Լ��� ������ �ʿ���� �״�� ����ϸ� �ȴ�.
/**
  * Function called when TCP connection established
  * param tpcb	: pointer on the connection contol block
  * param err	: when connection correctly established err should be ERR_OK 
  * retval err_t	: returned error 
  */
// ------------------------------------------------------------------------------------

static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
		// --<5-1> echoclient�� ����ü ���� es�� ������ 
		struct echoclient *es = NULL;
		
		if (err == ERR_OK)   {
				/* allocate structure es to maintain tcp connection informations */
				es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));
				
				if (es != NULL)	{
						es->state = ES_CONNECTED;
						es->pcb = tpcb;
						
						// -- <5-2> ���ӿϷ�ÿ� �۽��� �����͸� pbuf (es->p_tx)�� �ִ´� 
					  sprintf((char*)data, "["LOCAL_ID":"LOCAL_PW"]");				
						/* allocate pbuf */
						es->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL);
						 
						if (es->p_tx)  { 
							/* pass newly allocated es structure as argument to tpcb */
							tcp_arg(tpcb, es);		  
							// -- <5-3> ������ �۽��� �Ϸ�Ǹ� ȣ��� �ݹ��Լ��� ���� 
							tcp_recv(tpcb, tcp_echoclient_recv);		  
							// -- <5-4> ������ ������ �Ϸ�Ǹ� ȣ��� �ݹ��Լ��� ���� 
							tcp_sent(tpcb, tcp_echoclient_sent);		  
							// -- <5-5> polling�� ȣ��� �ݹ��Լ��� ���� 
							//             	- �� �ݹ��Լ��� �ֱ������� ȣ��Ǿ�� �Ѵ�.
							//				- �� �ݹ��Լ������� �۽��� ������ �߿��� ���� �̼۽ŵ� ���� �ִ�����
							//				   close�Ǿ�� �� conecction�� �ִ����� üũ�Ѵ�.
							tcp_poll(tpcb, tcp_echoclient_poll, 1);		
							
							// pbuf�� �ִ� �����͸� ������ �۽���
							/* copy data to pbuf */
							pbuf_take(es->p_tx, (char*)data, strlen((char*)data));
							/* send data */
							tcp_echoclient_send(tpcb,es);
							
							return ERR_OK;
						}
				}	
				else	{
						/* close connection */
						tcp_echoclient_connection_close(tpcb, es);		  
						/* return memory allocation error */
						return ERR_MEM;  
				}
		}
		else  {
				/* close connection */
				tcp_echoclient_connection_close(tpcb, es);
		}
		return err;
}
    
// ------------------------------------------------------------------------------------
//  -- <6> ������ ������ �Ϸ�Ǹ� ȣ��Ǵ� �ݹ� �Լ� 
/**
  * param arg		: argument to be passed to receive callback 
  * param tpcb	: tcp connection control block 
  * param err		: receive error code 
  * retval err_t	: retuned error  
  */
// ------------------------------------------------------------------------------------

extern recvData mainRecvData;
static err_t   tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{ 
		char * cp;
//+
		unsigned long ulLen;
		char recv_buff[DATABUFF_SIZE];
//		char send_buff[DATABUFF_SIZE]={0};
	  char * pToken;
    char * pArray[RECVDATA_CNT]={0};
		int i=0;
	
	 // -- <6-1> ���Ź��� �����͸� cp�� ����
		cp  = p->payload; 
		ulLen = p->tot_len;
		strncpy(recv_buff,cp,ulLen);
		recv_buff[ulLen]='\0';
#ifdef SERIAL_DEBUG
		printf("Recv00 : %s\r\n",recv_buff);
#endif 	
		
		pToken = strtok(recv_buff,"[@]");
		while(pToken != NULL)
		{
			pArray[i] = pToken;
			if(++i >= RECVDATA_CNT)
				break;
			pToken = strtok(NULL,"[@]");
		}
		if(i < RECVDATA_CNT)
			pArray[i] = NULL;		
		
		if(!strncmp(pArray[1]," New con",strlen(" New con"))) {
			return ERR_OK;
		}	
#ifdef SERIAL_DEBUG
		printf("Recv01 : %s, %s, %s\r\n",pArray[0],pArray[1], pArray[2]!=NULL?pArray[2]:""); 
#endif
 		
		// [STM32]LED@ON
		// pArray[0] : STM32
		// pArray[1] : LED
		// pArray[2] : ON
		mainRecvData.flag = 1;
		for(int j=0;j<i;j++)
		{
			strcpy(mainRecvData.dataArray[j],pArray[j]);
		}
		
/*		if(!strcmp(pArray[1],"LED"))  
		{
			mainRecvData.flag = 0;
			if(strcmp(pArray[2],"OFF") == 0) {
				HAL_GPIO_WritePin(GPIONucleo, GPIO_PIN_Led1, GPIO_PIN_RESET);
			}
			else if(strcmp(pArray[2],"ON") == 0) {
				HAL_GPIO_WritePin(GPIONucleo, GPIO_PIN_Led1, GPIO_PIN_SET);
			}
			sprintf(send_buff,"[%s]%s@%s",pArray[0],pArray[1],pArray[2]);
		}
		if(strncmp(pArray[0],LOCAL_ID,strlen(LOCAL_ID))) //�۽��ڰ� LOCAL_ID �϶��� ���� ���� 
		{	
			message_send_string(tpcb,send_buff);
		}

*/		
		// Free the p buffer 
		pbuf_free(p);
		return ERR_OK;
}


// ------------------------------------------------------------------------------------
//  -- <7> �����͸� �۽��ϴ� �Լ�
//				- �� �Լ��� ������ �ʿ���� �״�� ����ϸ� �ȴ�.
/**
  * param  tpcb	: tcp control block
  * param  es		: pointer on structure of type echoclient containing info on data 
  *             to be sent
  * retval 	None 
  */
// ------------------------------------------------------------------------------------

static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es)
{
		struct pbuf *ptr;
		err_t wr_err = ERR_OK;
	 
		while ((wr_err == ERR_OK) &&
					 (es->p_tx != NULL) && 
					 (es->p_tx->len <= tcp_sndbuf(tpcb)))
		{		
			/* get pointer on pbuf from es structure */
			ptr = es->p_tx;
			/* enqueue data for transmission */
			wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
			
			if (wr_err == ERR_OK)	{ 
					/* continue with next pbuf in chain (if any) */
					es->p_tx = ptr->next;			  
					if(es->p_tx != NULL)  {
						/* increment reference count for es->p */
						pbuf_ref(es->p_tx);
					}
							/* free pbuf:will free pbufs up to es->p (because es->p has a reference count > 0) */
					pbuf_free(ptr);
			 }
			
			 else if(wr_err == ERR_MEM)   {
					/* we are low on memory, try later, defer to poll */
				 es->p_tx = ptr;
			 }
			 
			 else  {
			 /* other problem ?? */
			 }
		}
}

// ------------------------------------------------------------------------------------
//  -- <8> ������ �۽��� �Ϸ�Ǹ� ȣ��Ǵ� �ݹ� �Լ�
//				- �� �Լ��� ������ �ʿ���� �״�� ����ϸ� �ȴ�.
/**
  *  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * param  	arg		: pointer on argument passed to callback
  * param  	tcp_pcb: tcp connection control block
  * param  	len		: length of data sent 
  * retval 	err_t		: returned error code
  */
// ------------------------------------------------------------------------------------

static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
		struct echoclient *es;

		LWIP_UNUSED_ARG(len);

		es = (struct echoclient *)arg;
		
		if(es->p_tx != NULL)
		{
			/* still got pbufs to send */
			tcp_echoclient_send(tpcb, es);
		}

		return ERR_OK;
}

// ------------------------------------------------------------------------------------
//  -- <9> ������ ȣ��Ǵ� �ݹ� �Լ�
//				- �� �Լ��� ������ �ʿ���� �״�� ����ϸ� �ȴ�.
/**
  *  This function implements the tcp_poll callback function
  * param  	arg	: pointer on argument passed to callback
  * param  	tpcb	: tcp connection control block
  * retval 	err_t	: error code
  */
// ------------------------------------------------------------------------------------

static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb)
{
		err_t ret_err;
		struct echoclient *es;

		es = (struct echoclient*)arg;
		if (es != NULL)
		{
			if (es->p_tx != NULL)
			{
				/* there is a remaining pbuf (chain) , try to send data */
				tcp_echoclient_send(tpcb, es);
			}
			else
			{
				/* no remaining pbuf (chain)  */
				if(es->state == ES_CLOSING)
				{
					/* close tcp connection */
					tcp_echoclient_connection_close(tpcb, es);
				}
			}
			ret_err = ERR_OK;
		}
		else
		{
			/* nothing to be done */
			tcp_abort(tpcb);
			ret_err = ERR_ABRT;
		}
		return ret_err;
}

// ------------------------------------------------------------------------------------
//  -- <10> tcp connection�� close�� �� ����ϴ� �Լ�
//				- �� �Լ��� ������ �ʿ���� �״�� ����ϸ� �ȴ�.
/**
  *  This function is used to close the tcp connection with server
  * param 	tpcb	: tcp connection control block
  * param 	es	: pointer on echoclient structure
  * retval 	None
  */
// ------------------------------------------------------------------------------------

static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es )
{
		/* remove callbacks */
		tcp_recv(tpcb, NULL);
		tcp_sent(tpcb, NULL);
		tcp_poll(tpcb, NULL,0);

		if (es != NULL) {
			mem_free(es);
		}

		/* close tcp connection */
		tcp_close(tpcb);  
}

#endif /* LWIP_TCP */

	
// -------------------------------------------------------------------------
//  -- <11> F107 Board(Client)����  PC(Server)�� �޽����� �����ϴ� �Լ�
//
// -------------------------------------------------------------------------

void message_send_string(struct tcp_pcb *tpcb, char *str)
{
			struct echoclient *es = NULL;

			/* allocate structure es to maintain tcp connection informations */
			es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));
			es->pcb = tpcb;
	
			strcpy((char*)data,str);

			// -- <11-2> �����͸� ���� : - server�� message_send() �� ����� ����� ���
			// �۽��� �����͸� es->p_tx (pbuf)�� �ִ´�	 (allocate pbuf )
			es->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL);
			
			if (es->p_tx) {
						// copy data to pbuf 
						pbuf_take(es->p_tx, (char*)data, strlen((char*)data));
						// send data 
						tcp_echoclient_send(tpcb, es);
			}		
}

// -------------------------------------------------------------------------
//  -- <12> Key�� �������� �����Ǵ� �޽����� Client --> Server�� �����ϴ� �Լ�
//
// -------------------------------------------------------------------------

void key_send(int key_number)
{
			struct echoclient *es = NULL;
			/* allocate structure es to maintain tcp connection informations */
			es = (struct echoclient *) mem_malloc( sizeof(struct echoclient) );
				
			if (es != NULL)	{
				es->pcb = echoclient_pcb;
						
				switch(key_number) {
					// --  <12-1> key_number�� ���� ���� �����Ǵ� �޽����� data�� ����.
					case 1 :	
							sprintf((char*)data, "[SENDFCM]pushbutton@LED@%s", HAL_GPIO_ReadPin(GPIONucleo, GPIO_PIN_Led1)?"ON":"OFF");
							break;					
				}

				// -- <12-2> �����͸� ���� 
				// �۽��� �����͸� es->p_tx (pbuf)�� �ִ´�	  /* allocate pbuf */
				es->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL);
				
				if (es->p_tx) {
							/* copy data to pbuf */
							pbuf_take(es->p_tx, (char*)data, strlen((char*)data));
							/* send data */
							tcp_echoclient_send(echoclient_pcb, es);
				}
			}			
}


// ------------------------------------------------------------------------
// -- <13> GPIO�� EXTI �� �߻��ϸ� (��, Key�� ��������) ȣ��Ǵ� �Լ�
//
// ------------------------------------------------------------------------

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
			// -- <13-1> �������� Key ���� ���� key_send( ) �Լ��� ȣ����	
			if (GPIO_Pin == GPIO_PIN_Nucleo_Sw)  {
				key_send(1);
			}
			// -- <13-2> Key�� chattering ������ �����ϱ� ���� �ð������� �ش� 
			for (int i=0; i<=100000; i++) ;
}
// ------------------------------------------------------------------------
