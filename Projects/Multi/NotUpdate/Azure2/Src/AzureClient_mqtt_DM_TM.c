// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include "STM32CubeInterface.h"
#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"
#include "AzureClient_mqtt_DM_TM.h"


/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "HostName=STM-test-iot.azure-devices.net;DeviceId=0080E1B8A9E2;SharedAccessKey=L9LzjVlnvhdlli/4UD3+e6XOFia7zTzmVs2Mij+kcP4=";

#define FIRMWARE_UPDATE_METHOD_NAME "firmwareUpdate"
#define FIRMWARE_QUIT_METHOD_NAME "quit"

DEFINE_ENUM_STRINGS(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES)
FIRMWARE_UPDATE_STATUS UpdateStatus;

static uint32_t callbackCounter;
static char msgText[1024];
static bool g_continueRunning;
static bool g_ExecuteOTA=0;

char OTA_HostName[32];
char OTA_Path[32];
uint32_t  OTA_PortNum=0;

extern volatile int MEMSInterrupt;
extern volatile int ButtonPressed;
extern volatile uint32_t SendData;
extern volatile uint32_t ReadProx;
static void SendSNSData(void);


typedef struct EVENT_INSTANCE_TAG
{
  IOTHUB_MESSAGE_HANDLE messageHandle;
  size_t messageTrackingId;  // For tracking the messages within the user callback.
  void *this;  // For Memory management (free)
} EVENT_INSTANCE;

IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

static void DeviceTwinCallback(int status_code, void* userContextCallback)
{
  (void)(userContextCallback);
  AZURE_PRINTF("-->DeviceTwin CallBack: Status_code = %u\r\n", status_code);
}

void ReportState(FIRMWARE_UPDATE_STATUS status) {
  unsigned const char * reportedState = ( unsigned char *) ENUM_TO_STRING(FIRMWARE_UPDATE_STATUS,status);
  if(IoTHubClient_LL_SendReportedState(iotHubClientHandle, reportedState, strlen((char *)reportedState), DeviceTwinCallback, NULL)!=IOTHUB_CLIENT_OK) {
    AZURE_PRINTF("ERROR reporting State: %s\r\n", reportedState);
  } else {
    AZURE_PRINTF("OK reported State: %s\r\n", reportedState);
  }
}

static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
  (void)userContextCallback;

#if 1
  AZURE_PRINTF("\r\nDevice Method called\r\n");
  AZURE_PRINTF("Device Method name:    %s\r\n", method_name);
  AZURE_PRINTF("Device Method payload: %s\r\n", (const char*)payload);
#endif

  /* Check if we need to make the FOTA */
  if(!strncmp(FIRMWARE_UPDATE_METHOD_NAME,method_name,strlen(FIRMWARE_UPDATE_METHOD_NAME))) {
    // If we receive the word 'FIRMWARE_UPDATE_METHOD_NAME' then we apply the Firmware-Over-The-Air update
    char* RESPONSE_STRING = "I will Make the FOTA update";
    char *StartAddress1= ((char *) payload)+1;
    char *StartAddress2=NULL;
    char *StartAddress3=NULL;
    char *StartAddress4=NULL;

    /* Search the port Number starting point */
    StartAddress2 = strstr( StartAddress1, ",");
    if(StartAddress2==NULL) {
      AZURE_PRINTF("Error Parsing the FOTA command searching the portNumber\r\n");
      return -1;
    }
    StartAddress2++;

    /* Search the FOTA path starting point */
    StartAddress3 =  strstr( StartAddress2, ",");
    if(StartAddress3==NULL) {
      AZURE_PRINTF("Error Parsing the FOTA command searching the start of FileName\r\n");
      return -2;
    }
    StartAddress3++;

    /* Search the FOTA path ending point */
    StartAddress4 =  strstr( StartAddress3, "\"");
    if(StartAddress4==NULL) {
      AZURE_PRINTF("Error Parsing the FOTA command searching the end of FileName\r\n");
      return -3;
    }
    StartAddress4++;

    snprintf(OTA_HostName,StartAddress2-StartAddress1,"%s",StartAddress1);
    OTA_PortNum = atoi(StartAddress2);
    snprintf(OTA_Path,StartAddress4-StartAddress3,"%s",StartAddress3);

    /* Make the FOTA */
    g_ExecuteOTA=1;

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL) {
      return -4;
    } else {
      memcpy(*response, RESPONSE_STRING, *resp_size);
    }
    return 1;
  } else if(!strncmp(FIRMWARE_QUIT_METHOD_NAME,method_name,strlen(FIRMWARE_QUIT_METHOD_NAME))) {
    char* RESPONSE_STRING = "I will stop";
    // If we receive the word 'FIRMWARE_QUIT_METHOD_NAME' then we stop running
    g_continueRunning = false;
    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL) {
      return -5;
    } else {
      memcpy(*response, RESPONSE_STRING, *resp_size);
    }
    return 2;
  } else {
    char* RESPONSE_STRING = "Method Not Implemented";

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL) {
      return -6;
    } else {
      memcpy(*response, RESPONSE_STRING, *resp_size);
    }
    return 0;
  }
}

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
  int* counter = (int*)userContextCallback;
  const char* buffer;
  size_t size;

  if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK) {
    AZURE_PRINTF("unable to retrieve the message data\r\n");
  } else {
    AZURE_PRINTF("Received Message [%d] with Data: <<<%.*s>>> & Size=%d\r\n", *counter, (int)size, buffer, (int)size);    
    if(!strncmp("quit",(char *)(buffer),4)) {
      // If we receive the word 'quit' then we stop running
      g_continueRunning = false;
    } else if(!strncmp("FOTA",(char *)(buffer),4)) {
      // If we receive the word 'FOTA' then we apply the Firmware-Over-The-Air update
      char *StartAddress1=NULL;
      char *StartAddress2=NULL;
      char *StartAddress3=NULL;
      /* Search the hostname starting point */
      StartAddress1 = strstr( (char *) buffer+4, ",");
      if(StartAddress1==NULL) {
        AZURE_PRINTF("Error Parsing the FOTA command searching the HostName\r\n");
        return IOTHUBMESSAGE_REJECTED;
      }
      StartAddress1++;

      /* Search the port Number starting point */
      StartAddress2 = strstr( StartAddress1, ",");
      if(StartAddress2==NULL) {
        AZURE_PRINTF("Error Parsing the FOTA command searching the portNumber\r\n");
        return IOTHUBMESSAGE_REJECTED;
      }
      StartAddress2++;

      /* Search the FOTA path starting point */
      StartAddress3 =  strstr( StartAddress2, ",");
      if(StartAddress3==NULL) {
        AZURE_PRINTF("Error Parsing the FOTA command searching the FileName\r\n");
        return IOTHUBMESSAGE_REJECTED;
      }
      StartAddress3++;

      snprintf(OTA_HostName,StartAddress2-StartAddress1,"%s",StartAddress1);
      OTA_PortNum = atoi(StartAddress2);
      snprintf(OTA_Path,size-(StartAddress3-buffer-1),"%s",StartAddress3);

      /* Make the FOTA */
      g_ExecuteOTA=1;
    }
  }

  // Retrieve properties from the message
  MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
  if (mapProperties != NULL) {
    const char*const* keys;
    const char*const* values;
    size_t propertyCount = 0;
    if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK) {
      if (propertyCount > 0) {
        for (size_t index = 0; index < propertyCount; index++){
          AZURE_PRINTF("\tKey: %s Value: %s\r\n", keys[index], values[index]);
        }
        AZURE_PRINTF("\r\n");
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
  AZURE_PRINTF("Confirmation[%ld] received for message tracking id = %d with result = %s\r\n", callbackCounter, eventInstance->messageTrackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
  callbackCounter++;
  IoTHubMessage_Destroy(eventInstance->messageHandle);
  free(eventInstance->this);
}


/**
  * @brief  Send Sensors' Data to IoT Hub
  * @param  None
  * @retval None
  */
static void SendSNSData(void)
{
  static size_t iterator = 0;
  EVENT_INSTANCE *messages;
  ComposeMessageSensors(msgText);
  AZURE_PRINTF("MsgText=%s\r\n",msgText);

  messages = (EVENT_INSTANCE *) calloc(1,sizeof(EVENT_INSTANCE));
  if(messages==NULL) {
    AZURE_PRINTF("Error Allocating Memory for messages to IoT Hub\r\n");
    while(1) {
    }
  } else {
    messages->this = (void *)messages;
  }

  if ((messages->messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL) {
    AZURE_PRINTF("ERROR: iotHubMessageHandle is NULL!\r\n");
  } else {
    messages->messageTrackingId = iterator;
    MAP_HANDLE propMap = IoTHubMessage_Properties(messages->messageHandle);
    sprintf_s(msgText, sizeof(msgText), "PropMsg_%zu", iterator);
    if (Map_AddOrUpdate(propMap, "PropName", msgText) != MAP_OK){
      AZURE_PRINTF("ERROR: Map_AddOrUpdate Failed!\r\n");
    }
    if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages->messageHandle, SendConfirmationCallback, messages) != IOTHUB_CLIENT_OK) {
      AZURE_PRINTF("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", (int)iterator);
    }
  }
  IoTHubClient_LL_DoWork(iotHubClientHandle);
  iterator++;
}

void AzureClient_mqtt_DM_TM(void)
{
  g_continueRunning = true;  
  callbackCounter = 0;
  int receiveContext = 0;

  if (platform_init() != 0){
    AZURE_PRINTF("Failed to initialize the platform.\r\n");
  } else {
    bool traceOn = true;
    AZURE_PRINTF("Platform Init Done\r\n");
    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL){
      AZURE_PRINTF("ERROR: iotHubClientHandle is NULL!\r\n");
      return;
    } else {
      AZURE_PRINTF("iotHubClientHandle Created\r\n");
    }

    if(IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn)==IOTHUB_CLIENT_OK) {
      AZURE_PRINTF("iotHubClientSetOption OK\r\n");
    }
    
    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK){
      AZURE_PRINTF("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
      return;
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SetMessageCallback...successful.\r\n");
    }

    if(IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, &receiveContext) != IOTHUB_CLIENT_OK){
      AZURE_PRINTF("ERROR: IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
      return;
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SetDeviceMethodCallback...successful.\r\n");
    }
    
    StartTimer1();
    StartTimer2();

    UpdateStatus = waiting;
    ReportState(UpdateStatus);

    /* Now that we are ready to receive commands, let's send some messages */
    while(g_continueRunning) {
      /* Handle Interrupt from MEMS */
      if(MEMSInterrupt) {
        MEMSCallback();
        MEMSInterrupt=0;
      }

      /* Handle user button */
      if(ButtonPressed) {
        ButtonCallback();
        ButtonPressed=0;
      }

      /* Environmental Data */
      if(SendData) {
        SendData=0;
        SendSNSData();
      }

      /* 53L0X */
      if(ReadProx) {
        ReadProx=0;
        ReadSingle53L0X();
      }

      /* Execute the FOTA */
      if(g_ExecuteOTA) {
        g_ExecuteOTA=0;
        FOTACallback(OTA_HostName,OTA_PortNum,OTA_Path);
      }

      /* Wait for event */
      __WFI();
    }

    StopTimer1();
    StopTimer2();

    AZURE_PRINTF("iothub_client_sample_mqtt has gotten quit message\r\n");
    IoTHubClient_LL_Destroy(iotHubClientHandle);
    AZURE_PRINTF("iotHubClientHandle Destroied\r\n");
    platform_deinit();
    AZURE_PRINTF("Platform DeInit\r\n");
  }
}
