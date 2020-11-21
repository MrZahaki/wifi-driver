#ifndef __WIFI_IO_H
#define __WIFI_IO_H

#include <string.h>
#include <stdlib.h>
#include "stm32f7xx_hal.h"
#include "cmsis_os2.h"   
#include "Driver_WiFi.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>STDIO Wifi Interface 
//   <i>rtx5 must be configured#
//   <o>Connect to hardware via Driver_WiFi#
//   <i>Select driver control block for WIFI interface
#define WIFI_DRV_NUM           0

//   <o>stdio timeout delay
//   <i>default value 2
#define STDIO_TIMEOUT           2

// 	 <h>Host IP Address 
//   <i>IPv4
//   		<o>part1
#define HOST_IP1           192
//   		<o>part2
#define HOST_IP2           168
//   		<o>part3
#define HOST_IP3          1
//   		<o>part4
#define HOST_IP4           46
// 	 </h>

//   <o>Host Port Number
#define HOST_PORT           5050

//   <o>Assertion enable <0=> Disable
//												<1=> Enable
//   <i>With the assertion management feature, you can debug the wifi driver
//   <i>If you want to disable this feature, please enter 0 otherwise the value entered does not matter
//   <i>If you enable this feature you will need to change the 'assert' template function in  text editor view
#define ASSERTION_CAP           1


//   <o>Always wait for connection <0=> Disable
//																 <1=> Enable
//   <i>With this feature, if the server is not available, the client will not interrupt the program and will always wait for new connection to the server. 
//   <i>If you want to disable this feature, please enter 0 otherwise the value entered does not matter
#define WAIT_FOREVER           1

// 	 <h>WiFi station mode config


// 			<o> WiFi Security Type <0=>ARM_WIFI_SECURITY_OPEN
//                          <1=>ARM_WIFI_SECURITY_WEP
//                          <2=>ARM_WIFI_SECURITY_WPA
//                          <3=>ARM_WIFI_SECURITY_WPA2
//                          <255=>ARM_WIFI_SECURITY_UNKNOWN
// 			<i> Default: ARM_WIFI_SECURITY_WPA2
#define WIFI_SECURITY_TYPE        3

// 			<o> WiFi Protected Setup (WPS) Method <0=>ARM_WIFI_WPS_METHOD_NONE
//                          <1=>ARM_WIFI_WPS_METHOD_PBC
//                          <2=>ARM_WIFI_WPS_METHOD_PIN
// 			<i> Default: ARM_WIFI_WPS_METHOD_NONE
#define WIFI_WPS_TYPE        0


// 	 </h>
//<o>STDIN allocation default mode <0=> static
//	   														 <1=> dynamic
#define STDIN_ALLOCATION_TYPE 0

//<o>STDIO buffer size
#define STDIO_BUFFER_SIZE 50
// </h>
 
 
#define QUEUE_SIZE           1         

#define _WiFi_Driver_(n)  Driver_WiFi##n
#define  WiFi_Driver_(n) _WiFi_Driver_(n)
 
extern ARM_DRIVER_WIFI  WiFi_Driver_(WIFI_DRV_NUM);
#define ptrWIFI       (&WiFi_Driver_(WIFI_DRV_NUM))


typedef struct {
  uint8_t stdout_flag;
	uint32_t data_size;
  void *data;
} message_t;


typedef enum{
  STATIC_MODE,
	DYNAMIC_MODE,
	DEFAULT_MODE
} allocation_type;
	

extern int wifi_send(void * data, uint32_t data_number, uint8_t block_size);
extern int wifi_init (char *ssid, char *password);
extern void stdio_thread (void *arg);
extern int wifi_get(uint32_t *data, uint32_t size, uint8_t allocation_type);


#endif /* __WIFI_IO_H */