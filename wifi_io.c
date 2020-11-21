/*--------------------------------the name of allah----------------------------
 * Name:    stdio_wifi.c
 * Purpose: STDIN WIFI
 * Rev.:    1.0.0
 *-----------------------------------------------------------------------------*/
#include "wifi_io.h"
 
 
#if ASSERTION_CAP != 0 
// * FreeRTOS source code is full of calls to the macro configASSERT(). This is an empty macro that
// *	developers can define inside the FreeRTOSConfig.h
void assert(uint8_t x, int32_t arg) {
 if ((x) == 0) {
	//taskDISABLE_INTERRUPTS();
	if((CoreDebug->DHCSR & 0x1) == 0x1) { /* If under debug */
		__asm("BKPT #0");
	} else {
		while(1);
	}
 }
} 
#endif

static osMessageQueueId_t qstdio;
static osMessageQueueId_t qstdin_resp;
static osMutexId_t stdin_mux;
volatile int32_t  sock, tmp;

void stdio_thread (void *arg) {
	
	static uint8_t ip[4] = {HOST_IP1, HOST_IP2, HOST_IP3, HOST_IP4};
	static uint32_t buffer;
	static ARM_WIFI_CONFIG_t wifi_config;
	osStatus_t state;
	static message_t qmsg;

	tmp = ptrWIFI->Initialize(NULL);
	
	#if ASSERTION_CAP != 0 
	/*tmporary*/
	//assert(tmp == ARM_DRIVER_OK, tmp);
	if(tmp != ARM_DRIVER_OK)
		__asm("BKPT #0");
	/*temporary*/
	
	#endif
	
	tmp = ptrWIFI->PowerControl(ARM_POWER_FULL);
	
	#if ASSERTION_CAP != 0 
		/*tmporary*/
	//assert(tmp == ARM_DRIVER_OK, tmp);
	if(tmp != ARM_DRIVER_OK)
		__asm("BKPT #0");
	/*temporary*/
	#endif
	
	wifi_config.ch = 0;/*automatic mode*/
	wifi_config.pass = (char *)(((uint32_t *)arg)[1] + ((uint32_t *)arg)[0] + 1 );
	wifi_config.security = ARM_WIFI_SECURITY_WPA2;
	wifi_config.ssid = (char *)(((uint32_t *)arg)[1]);
	wifi_config.wps_method = ARM_WIFI_WPS_METHOD_NONE;//not used
	//wifi_config.wps_pin
	
	tmp = ptrWIFI->Activate(0/*Access Point*/, &wifi_config);
	
	#if ASSERTION_CAP != 0 
		/*tmporary*/
	//assert(tmp == ARM_DRIVER_OK, tmp);
	if(tmp != ARM_DRIVER_OK)
		__asm("BKPT #0");
	/*temporary*/
	#endif
	
	
	
	#if ASSERTION_CAP != 0 
	
		/*tmporary*/
	//assert( sock >= 0/*socket id*/, sock);	
	if(sock < 0)
		__asm("BKPT #0");
	/*temporary*/
	#endif
	
	while(1){	
		sock = ptrWIFI->SocketCreate( ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
		tmp = ptrWIFI->SocketConnect( sock, ip, 4/*INET SIZE*/, HOST_PORT);
		
		#if WAIT_FOREVER == 0 
		assert( tmp  == ARM_DRIVER_OK, tmp);
		#endif
		
		if(tmp == 0){
		
			while (1){     
				state = osMessageQueueGet(qstdio, &qmsg, NULL, osWaitForever);
				#if ASSERTION_CAP != 0 
					
					/*tmporary*/
					//assert(state == osOK, tmp);
					if(state != osOK)
						__asm("BKPT #0");
					/*temporary*/
				#endif
				if(qmsg.stdout_flag & 1){//stdout
					tmp = ptrWIFI->SocketSend(sock, (uint8_t *)&qmsg.data_size, 1);
					if( tmp < 0/*number of bytes sended*/) break;
					
					tmp = ptrWIFI->SocketSend(sock, qmsg.data, qmsg.data_size);
					if( tmp < 0/*number of bytes sended*/) break;
				}
				else{//stdin
					buffer =  0xffffffff;
					tmp = ptrWIFI->SocketSend(sock, (uint8_t *)&buffer, 4);
					
					buffer = 0;
					tmp = ptrWIFI->SocketRecv(sock, (uint8_t *)&buffer, 4);
					if(tmp < 0/*number of bytes received*/) break;
					
					if(qmsg.data_size == 0)
						qmsg.data_size = buffer;
					
					qmsg.stdout_flag = (uint8_t)(qmsg.stdout_flag << 1);
					if(qmsg.stdout_flag == DYNAMIC_MODE || ((qmsg.stdout_flag == DEFAULT_MODE) && STDIN_ALLOCATION_TYPE))
						qmsg.data =(uint8_t *)calloc(buffer, sizeof(uint8_t));
					
					tmp = ptrWIFI->SocketRecv(sock, (uint8_t *)qmsg.data, qmsg.data_size);
					if( tmp < 0/*number of bytes received*/) break;
					
					state = osMessageQueuePut(qstdin_resp, &qmsg, 0U, osWaitForever);
					#if ASSERTION_CAP != 0 
					/*tmporary*/
					//assert(state == osOK, tmp);
					if(state != osOK)
						__asm("BKPT #0");
					/*temporary*/
				#endif
				}//else
			}//inner while
		}//if
		#if WAIT_FOREVER == 0 
			assert( tmp  == ARM_DRIVER_OK, tmp);
		#endif
		ptrWIFI->SocketClose(sock);
		
	}//while(1)
}
 

int wifi_send(void * data, uint32_t data_number, uint8_t block_size){
	static message_t stdo_msg = {.stdout_flag = 1};
	osStatus_t state;
	stdo_msg.data = data;
	stdo_msg.data_size = block_size * data_number;
	
	state = osMessageQueuePut(qstdio, &stdo_msg, 0U, osWaitForever);
	if(state != osOK ) return -1;
	
  return 0;
}
/*
* @note: In static allocation method just put your destination address on data pointer variable 
* @param size: The amount of data you want to get from the current socket. 
*              If you have no idea about the amount of data you want to get from the socket, just set it to zero.
*
* @retval: size of output data as uint32_t type(-1 means error as int type)
*/

int wifi_get(uint32_t *data, uint32_t size, uint8_t allocation_type){
	static message_t retval, stdin_msg;
	osStatus_t state;
	
	state = osMutexAcquire(stdin_mux, osWaitForever);
	if(state != osOK ) return -1;
	
	stdin_msg.stdout_flag = allocation_type >> 1;
	stdin_msg.data = (uint32_t *)data;
	stdin_msg.data_size = size;
	
	state = osMessageQueuePut(qstdio, &stdin_msg, 0U, osWaitForever);
	if(state != osOK ) return -1;

	state = osMessageQueueGet(qstdin_resp, &retval, NULL, osWaitForever);
	if(state != osOK ) return -1;
	
	if(STDIN_ALLOCATION_TYPE)
		*data = (uint32_t)retval.data;
	osMutexRelease(stdin_mux);
	
	return retval.data_size;
}

/**
  Initialize stdin
 
  \return          0 on success, or -1 on error.
*/
int wifi_init (char *ssid, char *password) {
	osThreadId_t tid0; 
	char *buf = calloc(strlen(ssid) + strlen(password) + 3, sizeof(char));
  static uint32_t databuf[2];
	
  databuf[0] = strlen(ssid);
  databuf[1] = (uint32_t)buf;
  memcpy(buf, ssid, strlen(ssid));
	memcpy(buf + strlen(ssid)+1, password, strlen(password));
	
	qstdio = osMessageQueueNew(QUEUE_SIZE, sizeof(message_t), NULL );
	if (qstdio == NULL) return -1;
	
	qstdin_resp = osMessageQueueNew(1, sizeof(message_t), NULL );
	if (qstdin_resp == NULL) return -1;
	
	stdin_mux = osMutexNew(NULL);
	if (stdin_mux == NULL) return -1;
	
  tid0 = osThreadNew(stdio_thread, databuf, NULL);
  if (tid0 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
 
/**
  Get a character from stdin
 
  \return     The next character from the input, or -1 on read error.
*/
int stdin_getchar (void) {
	static message_t stdin_msg = {.stdout_flag = 0};
	osStatus_t state;
	uint8_t buf;
	
	state = osMutexAcquire(stdin_mux, osWaitForever);
	if(state != osOK ) return -1;
	
		state = osMessageQueuePut(qstdio, &stdin_msg, 0U, osWaitForever);
		if(state == osOK )
			state = osMessageQueueGet(qstdin_resp, &buf, NULL, osWaitForever);
		else return -1;
		
		if(state != osOK ) return -1;
	
	osMutexRelease(stdin_mux);
	
	return buf;
}

int stdout_putchar (int ch) {
	static message_t stdin_msg = {.stdout_flag = 1};
	static uint8_t buffer[STDIO_BUFFER_SIZE];
	osStatus_t state;
	stdin_msg.data = (uint8_t)ch;
	state = osMessageQueuePut(qstdio, &stdin_msg, 0U, osWaitForever);
	if(state != osOK ) return -1;
	
  return (ch);
}

