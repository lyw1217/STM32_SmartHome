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



// -- <1> 필요한 헤더 파일을 인클루드 
#include "main.h"
#include "Nucleo_F429.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"

#include <stdio.h>
#include <string.h>

#if LWIP_TCP

// -- <2> 변수, 함수의 선언 
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


// -- <3> 함수의 선언 
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
// -- <4> Server에 접속(TCP connect)하는 함수 
//				- 이 함수는 변경할 필요없이 그대로 사용하면 된다.
//
// -------------------------------------------------------------------------

void tcp_echoclient_connect(void)
{
		struct ip4_addr DestIPaddr;
		
		// -- <4-1> create new tcp pcb 
		echoclient_pcb = tcp_new();
		
		if (echoclient_pcb != NULL)  {
			// -- <4-2> 접속할 server의 IP를 지정함 -	
			IP4_ADDR( &DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );
			// -- <4-3> 지정된 IP(&DestIPaddr), 포트(DEST_PORT)에 접속함
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
//  -- <5> 서버에 TCP connection이 완료되면 호출되는 콜백 함수 
//				- 이 함수는 변경할 필요없이 그대로 사용하면 된다.
/**
  * Function called when TCP connection established
  * param tpcb	: pointer on the connection contol block
  * param err	: when connection correctly established err should be ERR_OK 
  * retval err_t	: returned error 
  */
// ------------------------------------------------------------------------------------

static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
		// --<5-1> echoclient형 구조체 변수 es를 선언함 
		struct echoclient *es = NULL;
		
		if (err == ERR_OK)   {
				/* allocate structure es to maintain tcp connection informations */
				es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));
				
				if (es != NULL)	{
						es->state = ES_CONNECTED;
						es->pcb = tpcb;
						
						// -- <5-2> 접속완료시에 송신할 데이터를 pbuf (es->p_tx)에 넣는다 
					  sprintf((char*)data, "["LOCAL_ID":"LOCAL_PW"]");				
						/* allocate pbuf */
						es->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL);
						 
						if (es->p_tx)  { 
							/* pass newly allocated es structure as argument to tpcb */
							tcp_arg(tpcb, es);		  
							// -- <5-3> 데이터 송신이 완료되면 호출될 콜백함수를 지정 
							tcp_recv(tpcb, tcp_echoclient_recv);		  
							// -- <5-4> 데이터 수신이 완료되면 호출될 콜백함수를 지정 
							tcp_sent(tpcb, tcp_echoclient_sent);		  
							// -- <5-5> polling시 호출될 콜백함수를 지정 
							//             	- 이 콜백함수는 주기적으로 호출되어야 한다.
							//				- 이 콜백함수에서는 송신할 데이터 중에서 아직 미송신된 것이 있는지와
							//				   close되어야 할 conecction이 있는지를 체크한다.
							tcp_poll(tpcb, tcp_echoclient_poll, 1);		
							
							// pbuf에 있는 데이터를 서버로 송신함
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
//  -- <6> 데이터 수신이 완료되면 호출되는 콜백 함수 
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
	
	 // -- <6-1> 수신받은 데이터를 cp에 대입
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
		if(strncmp(pArray[0],LOCAL_ID,strlen(LOCAL_ID))) //송신자가 LOCAL_ID 일때는 전송 안함 
		{	
			message_send_string(tpcb,send_buff);
		}

*/		
		// Free the p buffer 
		pbuf_free(p);
		return ERR_OK;
}


// ------------------------------------------------------------------------------------
//  -- <7> 데이터를 송신하는 함수
//				- 이 함수는 변경할 필요없이 그대로 사용하면 된다.
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
//  -- <8> 데이터 송신이 완료되면 호출되는 콜백 함수
//				- 이 함수는 변경할 필요없이 그대로 사용하면 된다.
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
//  -- <9> 폴링시 호출되는 콜백 함수
//				- 이 함수는 변경할 필요없이 그대로 사용하면 된다.
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
//  -- <10> tcp connection을 close할 때 사용하는 함수
//				- 이 함수는 변경할 필요없이 그대로 사용하면 된다.
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
//  -- <11> F107 Board(Client)에서  PC(Server)로 메시지를 전송하는 함수
//
// -------------------------------------------------------------------------

void message_send_string(struct tcp_pcb *tpcb, char *str)
{
			struct echoclient *es = NULL;

			/* allocate structure es to maintain tcp connection informations */
			es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));
			es->pcb = tpcb;
	
			strcpy((char*)data,str);

			// -- <11-2> 데이터를 보냄 : - server의 message_send() 와 비슷한 방법을 사용
			// 송신할 데이터를 es->p_tx (pbuf)에 넣는다	 (allocate pbuf )
			es->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL);
			
			if (es->p_tx) {
						// copy data to pbuf 
						pbuf_take(es->p_tx, (char*)data, strlen((char*)data));
						// send data 
						tcp_echoclient_send(tpcb, es);
			}		
}

// -------------------------------------------------------------------------
//  -- <12> Key가 눌러지면 대응되는 메시지를 Client --> Server로 전송하는 함수
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
					// --  <12-1> key_number의 값에 따라 대응되는 메시지를 data에 저장.
					case 1 :	
							sprintf((char*)data, "[SENDFCM]pushbutton@LED@%s", HAL_GPIO_ReadPin(GPIONucleo, GPIO_PIN_Led1)?"ON":"OFF");
							break;					
				}

				// -- <12-2> 데이터를 보냄 
				// 송신할 데이터를 es->p_tx (pbuf)에 넣는다	  /* allocate pbuf */
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
// -- <13> GPIO의 EXTI 가 발생하면 (즉, Key가 눌러지면) 호출되는 함수
//
// ------------------------------------------------------------------------

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
			// -- <13-1> 눌러지는 Key 값에 따라 key_send( ) 함수를 호출함	
			if (GPIO_Pin == GPIO_PIN_Nucleo_Sw)  {
				key_send(1);
			}
			// -- <13-2> Key의 chattering 현상을 방지하기 위해 시간지연을 준다 
			for (int i=0; i<=100000; i++) ;
}
// ------------------------------------------------------------------------
