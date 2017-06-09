// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

/* This sample uses the _LL APIs of iothub_client for example purposes.
   That does not mean that HTTP only works with the _LL APIs.
   Simply changing the using the convenience layer (functions not having _LL)
   and removing calls to _DoWork will yield the same results. */

#ifdef ARDUINO
#include "AzureIoT.h"
#else
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "iothub_client_ll.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"
#endif 

#include "AzureIOTSDKConfig.h"

#if APPLICATION_SCENARIO == AZURE_ENDLESS_LOOP_BINARY  
    #include "RegistrationAgent.h"
#endif

#ifdef MBED_BUILD_TIMESTAMP
#include "certs.h"
#endif // MBED_BUILD_TIMESTAMP


#if APPLICATION_SCENARIO != AZURE_HTTP_CLIENT_SAMPLE  
static char *connectionString;
static int appDelay = 200; // 2 sec
#else
/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = AZUREIOTHUBCONNECTIONSTRING;
#endif

static int callbackCounter;
static bool g_continueRunning;
static char msgText[1024];
static char propText[1024];
#if APPLICATION_SCENARIO != AZURE_HTTP_CLIENT_SAMPLE  
  #define MESSAGE_COUNT 1
#else
  #define MESSAGE_COUNT 5
#endif
#define DOWORK_LOOP_NUM     3



typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    int* counter = (int*)userContextCallback;
    const char* buffer;
    size_t size;
    MAP_HANDLE mapProperties;
#ifdef __NUCLEO_BUILD
    static char msgCmd[16];
    int i;
#endif
	
	
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        printf("[IoTHub][E]. unable to retrieve the message data\r\n");
    }
    else
    {
        (void)printf("[IoTHub]. Received Message [%d] with Data: <<<%.*s>>> & Size=%d\r\n", *counter, (int)size, buffer, (int)size);
        if (memcmp(buffer, "quit", size) == 0)
        {
            g_continueRunning = false;
        }
#ifdef __NUCLEO_BUILD        
        else if (memcmp(buffer, "led on", size) == 0)
        {  
            set_led();
        }
        else if (memcmp(buffer, "led off", size) == 0)
        {
            reset_led();        
        } 
#if APPLICATION_SCENARIO == AZURE_ENDLESS_LOOP_BINARY  
        else if (memcmp(buffer, "delay", 5) == 0)
        {
          i = 1;
          while ( (size-5-i) > 0  )
          {  
              msgCmd[(i-1)] = buffer[5+i];
              i++;
          }   
          msgCmd[i] = '\0';
          appDelay = atoi(msgCmd);
          appDelay *= 100;  
        }
#endif
        
#endif               
    }
    // Retrieve properties from the message
    mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
				size_t index;  
                printf("[IoTHub]. Message Properties:\r\n");
                for (index = 0; index < propertyCount; index++)
                {
                    printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
                printf("\r\n");
            }
        }
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
    (void)printf("[IoTHub]. Confirmation[%d] received for message tracking id = %d with result = %s\r\n", callbackCounter, eventInstance->messageTrackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    /* Some device specific action code goes here... */
    callbackCounter++;
    IoTHubMessage_Destroy(eventInstance->messageHandle);
}



void iothub_client_sample_http_run(void)
{
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
    EVENT_INSTANCE messages[MESSAGE_COUNT];
#ifdef __NUCLEO_BUILD     
    int i_ret_status_init = 0;
#else
    double avgWindSpeed = 10.0;
#endif
    int receiveContext = 0;
#if APPLICATION_SCENARIO == AZURE_ENDLESS_LOOP_BINARY  
    HTTP_HANDLE httpServiceHandle = 0;
#endif    
    g_continueRunning = true;
    
    callbackCounter = 0;
	
    // check if ConnectionString is in FLASH
    // otherwise --> registration 
#if APPLICATION_SCENARIO != AZURE_HTTP_CLIENT_SAMPLE  
    #if APPLICATION_SCENARIO == AZURE_ENDLESS_LOOP_BINARY  
        connectionString = malloc (sizeof(REGISTRATIONAGENT_DEFAULT_IOT_HUB)+AZURE_FIXED_SIZE_CONNSTRING);
        if ( retrieve_connection_string( httpServiceHandle, connectionString ) != 0)  {
                printf("[Registration][E]. Failed to retrieve connection string. Application exit. \r\n");
                return; 
        } 
    #else
        if (AZUREIOTHUBCONNECTIONSTRING != NULL) {
            connectionString = malloc (sizeof(AZUREIOTHUBCONNECTIONSTRING));
            strcpy(connectionString, AZUREIOTHUBCONNECTIONSTRING); 
          }
          else {
            printf("[IotHub][E]. Connection string is NULL. Application exit\r\n");      
         }
    #endif   
#endif

#ifndef __NUCLEO_BUILD    
    if ( platform_init() != 0){
              printf("[IotHub][E]. Failed to sync with NTP or to initialize sensors. Application exit\r\n");
#else    
    i_ret_status_init = platform_init();           
    if ( i_ret_status_init > 0 && i_ret_status_init < 2 ){
              printf("[IotHub][E]. Failed to sync with NTP . Application exit. \r\n");
#endif
        
    }
    else
    {  
            (void)printf("[IotHub]. Starting the IoTHub client sample HTTP...\r\n");

#ifdef __NUCLEO_BUILD                
            if (i_ret_status_init > 1)
                  printf("[IotHub][E]. Failed to init sensor board, using dummy data. \r\n");
#endif
            
            srand((unsigned int)time(NULL));          

            if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol)) == NULL)
            {
                (void)printf("[IotHub][E]. iotHubClientHandle is NULL!\r\n");
            }
            else
            {
                unsigned int timeout = 241000;
		// Because it can poll "after 9  for scalabilty, the deseconds" polls will happen effectively // at ~10 seconds.
		// Note thatfault value of minimumPollingTime
		// is 25 minutes. For more information, see:
		// https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging
		unsigned int minimumPollingTime = 2;
                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "timeout", &timeout) != IOTHUB_CLIENT_OK)
                {
                    printf("[IotHub][E]. Failure to set option \"timeout\"\r\n");
                }

                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "MinimumPollingTime", &minimumPollingTime) != IOTHUB_CLIENT_OK)
                {
                    printf("[IotHub][E]. Failure to set option \"MinimumPollingTime\"\r\n");
                }

        #ifdef MBED_BUILD_TIMESTAMP
                // For mbed add the certificate information
                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
                {
                    printf("[IotHub][E]. Failure to set option \"TrustedCerts\"\r\n");
                }
        #endif // MBED_BUILD_TIMESTAMP

                /* Setting Message call back, so we can receive Commands. */
                if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
                {
                    (void)printf("[IotHub][E]. IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
                }
                else
                {
                    (void)printf("[IotHub]. IoTHubClient_LL_SetMessageCallback...successful.\r\n");
              #ifdef __NUCLEO_BUILD        
                     set_led();
              #endif
                          
                    /* Now that we are ready to receive commands, let's send some messages */
		    size_t iterator = 0;
		    do
		    {
			if (iterator < MESSAGE_COUNT)                   
		        {
        #ifdef __NUCLEO_BUILD        
                           if (ComposeMessageSensors(msgText)!=0)
                                    (void)printf("[IotHub]. Sending message: %s\r\n",msgText);
                           else
                                    (void)printf("[IotHub][E]. Failed to create sensors message\r\n");             
        #else
                        sprintf_s(msgText, sizeof(msgText), "{\"deviceId\": \"myFirstDevice\",\"windSpeed\": %d}", (int)(avgWindSpeed + (10*(rand() % 4 + 2))));
        #endif
                        if ((messages[iterator].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
                        {
                            (void)printf("[IotHub][E]. iotHubMessageHandle is NULL!\r\n");
                        }
                        else
                        {
                            MAP_HANDLE propMap;
                            messages[iterator].messageTrackingId = iterator;

                             propMap = IoTHubMessage_Properties(messages[iterator].messageHandle);
                            (void)sprintf_s(propText, sizeof(propText), "PropMsg_%zu", iterator);
                            if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
                            {
                                (void)printf("[IotHub][E]. Map_AddOrUpdate Failed!\r\n");
                            }

                            if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages[iterator].messageHandle, SendConfirmationCallback, &messages[iterator]) != IOTHUB_CLIENT_OK)
                            {
                                (void)printf("[IotHub][E]. IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
                            }
                            else
                            {
                                (void)printf("[IotHub]. IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", iterator);
                            }
                        }
                        
         #if APPLICATION_SCENARIO != AZURE_HTTP_CLIENT_SAMPLE  
                        // Endless trasnmission of sensors data IoT Hub                   
                        if (iterator == (MESSAGE_COUNT-1))
                        {
                           // send messages 
                           for (int j = 0; j < appDelay; j++)
                           {
                                  IoTHubClient_LL_DoWork(iotHubClientHandle);
                                  ThreadAPI_Sleep(10);
                           }       
                           // restart
                           iterator = -1; 
                           (void)printf("[IotHub]. IoTHubClient_LL_DoWork...sent data.\r\n");
                          
                        }
	    #endif                                      
                    }
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(500);
                    iterator++;
                } while (g_continueRunning);

                printf("\n\r"); 
                
                (void)printf("[IotHub]. iothub_client_sample_http has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);
                for (size_t index = 0; index < DOWORK_LOOP_NUM; index++)
                {
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(1);
                }

            }
            IoTHubClient_LL_Destroy(iotHubClientHandle);
        }
        platform_deinit();
#if APPLICATION_SCENARIO != AZURE_HTTP_CLIENT_SAMPLE  
        free(connectionString);
        reset_led();
#endif
        (void)printf("[IotHub]. Application stopped \r\n");
    }
}


#ifndef __NUCLEO_BUILD
int main(void)
{
    iothub_client_sample_http_run();
    return 0;
}
#endif


#if APPLICATION_SCENARIO == AZURE_ENDLESS_LOOP_BINARY  
/**
  * @brief  Register device and retrieve connection string  
  * @param  None
  * @retval int value for success (1) / failure (0)
  */
int retrieve_connection_string( HTTP_HANDLE httpServiceHandle, char *connectionString )
{
  
    if ( RegistrationAgentStart(httpServiceHandle, REGISTRATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS,REGISTRATIONAGENT_DEFAULT_ENDPOINT_TCPPORT, connectionString) != REG_SUCCESS ){
      printf("\r\n[Registration][E]. Failed Registration Procedure\r\n");
      return 1;
    }
    else {  
      printf("\r\n[Registration]. Registration procedure completed. \r\n");

    }
   return 0;
}
#endif


