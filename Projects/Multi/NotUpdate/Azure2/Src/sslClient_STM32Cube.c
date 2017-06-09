// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//#define DEBUG_SSL_CLIENT

#ifdef DEBUG_SSL_CLIENT
#include "STM32CubeInterface.h"
#endif /* DEBUG_SSL_CLIENT */

#include "sslClient_STM32Cube.h"
#include "azure_c_shared_utility/xlogging.h"
#include "wifi_interface.h"
//#include "wifi_module.h"
#include "TLocalBuffer.h"

static uint8_t  wifi_sock_id = 0xFF;
static uint8_t  connected_sockets = 0;

void sslClient_setTimeout(unsigned long timeout)
{
   // EQ. FIXME
#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_setTimeout\r\n");
#endif /* DEBUG_SSL_CLIENT */
   return;	
   // sslClient.setTimeout(timeout);
}

uint8_t sslClient_connected(void)
{
#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_connected\r\n");
#endif /* DEBUG_SSL_CLIENT */
  if (connected_sockets > 0) {
    return 1;
  } else {
    return 0;
  }
}

int sslClient_connect(uint8_t * hostname, uint16_t port)
{
  WiFi_Status_t wifi_status;
#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_connect port=%d [%s]\r\n",port,hostname);
#endif /* DEBUG_SSL_CLIENT */
  if (connected_sockets > 0) {
    // print error
    return 0;
  }

  wifi_status = wifi_socket_client_open((uint8_t*)hostname, (uint32_t)port, "s", &wifi_sock_id);

  if(wifi_status != WiFi_MODULE_SUCCESS) {
    return 0;
  } else {
    connected_sockets++;
#ifdef DEBUG_SSL_CLIENT
    AZURE_PRINTF("Socket open -> connected_sockets=%d\r\n",connected_sockets);
#endif /* DEBUG_SSL_CLIENT */
    return 1;
  }
}

void sslClient_stop(void)
{
  // sslClient.stop();
  // wifi_socket_client_close(uint8_t sock_close_id);
  WiFi_Status_t wifi_status;

#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_stop\r\n");
#endif /* DEBUG_SSL_CLIENT */

  wifi_status = wifi_socket_client_close(wifi_sock_id);

  if(wifi_status != WiFi_MODULE_SUCCESS){
   // print error message
  } else {
   connected_sockets--;
#ifdef DEBUG_SSL_CLIENT
   AZURE_PRINTF("Socket close -> connected_sockets=%d\r\n",connected_sockets);
#endif /* DEBUG_SSL_CLIENT */
  }

  return;
}

size_t sslClient_write(const uint8_t *buf, size_t size)
{
  // return sslClient.write(buf, size);
#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_write size=%d\r\n",size);
#endif /* DEBUG_SSL_CLIENT */

  if ( wifi_socket_client_write(wifi_sock_id, (uint16_t) size,(char *) buf) == WiFi_MODULE_SUCCESS ) {
    return size;
  } else {
    return 0;
  }
}

size_t sslClient_print(const char* str)
{
  //  TBD	
  // EQ. FIXME. Implement or remove
#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_print =%s\r\n",str);
#endif /* DEBUG_SSL_CLIENT */
  return 0;
}

int sslClient_read(uint8_t *buf, size_t size)
{
  //return sslClient.read(buf, size);

#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_read size=%d\r\n",size);
#endif /* DEBUG_SSL_CLIENT */
  int sizeReceived;
  sizeReceived = LocalBufferGetSizeBuffer(&localBufferReading);

  if(sizeReceived > 0){
    if(sizeReceived >= (int)size) {
      LocalBufferPopBuffer(&localBufferReading, buf, (int)size);
      return size;
    } else {
      LocalBufferPopBuffer(&localBufferReading, buf, sizeReceived);
      return sizeReceived;
    }
  }
  return 0;
}

int sslClient_available(void)
{
   // return sslClient.available();
     // EQ. FIXME. Implement or remove
#ifdef DEBUG_SSL_CLIENT
  AZURE_PRINTF(">>sslClient_available\r\n");
#endif /* DEBUG_SSL_CLIENT */
   return 0;
}

/*
uint8_t sslClient_hostByName(const char* hostName, uint32_t* ipAddress)
{
   IPAddress ip;
   uint8_t result = WiFi.hostByName(hostName, ip);
   (*ipAddress) = (uint32_t)ip;
   return result;
}
*/

