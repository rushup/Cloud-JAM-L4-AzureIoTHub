/**
  ******************************************************************************
  * @file    AzureClient_mqtt_DM_TM.c
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Main MQTT application for Telemetry and Device Management
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "STM32CubeInterface.h"
#include "iothub_client_ll.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"
#include "AzureClient_mqtt_DM_TM.h"
#ifdef USE_MBED_TLS
#include "certificates.h"
#endif /* USE_MBED_TLS */
#ifdef AZURE_ENABLE_REGISTRATION
  #include "RegistrationAgent.h"
#endif /* AZURE_ENABLE_REGISTRATION */

#include "serializer.h"
#include "serializer_devicetwin.h"
#include "jsondecoder.h"

#ifdef USE_MBED_TLS
  #include "azure_c_shared_utility/tls_config.h"
  #include "mbedtls/config.h"
#endif

/* Connection string containing Hostname, Device Id & Device Key */
static char *connectionString = AZUREIOTHUBCONNECTIONSTRING;

DEFINE_ENUM_STRINGS(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES)


BEGIN_NAMESPACE(AzureModel);

DECLARE_MODEL(Azure1_t,
/* Event data: Temperature, Humidity... */
WITH_DATA(ascii_char_ptr, deviceId),
WITH_DATA(int, messageId),
WITH_DATA(float, temperature),
WITH_DATA(float, humidity),
WITH_DATA(int, accX),
WITH_DATA(int, accY),
WITH_DATA(int, accZ),
WITH_DATA(int, gyrX),
WITH_DATA(int, gyrY),
WITH_DATA(int, gyrZ),
WITH_DATA(EDM_DATE_TIME_OFFSET, ts),
/* Methods */
WITH_METHOD(Reboot),
WITH_METHOD(Quit),
WITH_METHOD(FirmwareUpdate, ascii_char_ptr, FwPackageUri),
/* Commands */
WITH_ACTION(Pause),
WITH_ACTION(Play),
WITH_ACTION(LedOn),
WITH_ACTION(LedOff),
WITH_ACTION(LedBlink),
/* Register Direct Methods with IoT Hub */
WITH_REPORTED_PROPERTY(ascii_char_ptr_no_quotes, SupportedMethods),
/* Desired Properties */
WITH_DESIRED_PROPERTY(int, DesiredTelemetryInterval, ChangeTelemetryInterval),
/* Reported Properties */
/* Telemetry Interval in Seconds... value from 1 to: 0xFFFF/2000 = About 30 Seconds */
WITH_REPORTED_PROPERTY(int,TelemetryInterval),
WITH_REPORTED_PROPERTY(ascii_char_ptr,AzureStatus),
WITH_REPORTED_PROPERTY(ascii_char_ptr, AzureFwVersion)
);

END_NAMESPACE(AzureModel);

Azure1_t *Azure1;

typedef struct EVENT_INSTANCE_TAG
{
  IOTHUB_MESSAGE_HANDLE messageHandle;
  size_t messageTrackingId;  // For tracking the messages within the user callback.
  void *this;  // For Memory management (free)
} EVENT_INSTANCE;

IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

#define AZURE_LED_MODE \
  AZURE_LED_ON, \
  AZURE_LED_OFF, \
  AZURE_LED_BLINK
DEFINE_ENUM(AZURE_LED, AZURE_LED_MODE)


static bool g_continueRunning;
static bool g_reboot     = false;
static bool g_ExecuteOTA =false;
static AZURE_LED LedMode = AZURE_LED_BLINK;
/* Counters for Sent/Received Status */
static size_t StatusBackCount = 0;
static size_t SentStatusCount = 0;
/* Counters for Sent/Received Messages */
static size_t MessagesBackCount = 0;
static size_t SentMessagesCount = 0;

static volatile int32_t ReceivedDeviceTwinProperty=0;

static char OTA_HostName[64];
static char OTA_Path[128];
static char OTA_Type='t';
static uint32_t OTA_PortNum=0;

static void SendSNSData(void);

static EDM_DATE_TIME_OFFSET GetDateTimeOffset(time_t time)
{
  struct tm *newTime;
  EDM_DATE_TIME_OFFSET dateTimeOffset;
  newTime = gmtime(&time);
  dateTimeOffset.dateTime = *newTime;
  dateTimeOffset.fractionalSecond = 0;
  dateTimeOffset.hasFractionalSecond = 0;
  dateTimeOffset.hasTimeZone = 0;
  dateTimeOffset.timeZoneHour = 0;
  dateTimeOffset.timeZoneMinute = 0;
  return dateTimeOffset;
}

/* Handles for Commands */

/**
 * @brief Implementation of Pause Command
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval EXECUTE_COMMAND_RESULT Command Result
 */
EXECUTE_COMMAND_RESULT Pause(Azure1_t *Azure1)
{
  (void)(Azure1);
  AZURE_PRINTF("Received Pause command\r\n");
  StopTimer1();
  return EXECUTE_COMMAND_SUCCESS;
}

/**
 * @brief Implementation of Play Command
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval EXECUTE_COMMAND_RESULT Command Result
 */
EXECUTE_COMMAND_RESULT Play(Azure1_t *Azure1)
{
  (void)(Azure1);
  AZURE_PRINTF("Received Play command\r\n");
  StartTimer1();
  return EXECUTE_COMMAND_SUCCESS;
}

/**
 * @brief Implementation of LedOn Command
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval EXECUTE_COMMAND_RESULT Command Result
 */
EXECUTE_COMMAND_RESULT LedOn(Azure1_t *Azure1)
{
  (void)(Azure1);
  AZURE_PRINTF("Received LedOn command\r\n");
  LedMode = AZURE_LED_ON;
  BSP_LED_On(LED2);
  return EXECUTE_COMMAND_SUCCESS;
}

/**
 * @brief Implementation of LedOff Command
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval EXECUTE_COMMAND_RESULT Command Result
 */
EXECUTE_COMMAND_RESULT LedOff(Azure1_t *Azure1)
{
  (void)(Azure1);
  AZURE_PRINTF("Received LedOff command\r\n");
  LedMode = AZURE_LED_OFF;
  BSP_LED_Off(LED2);
  return EXECUTE_COMMAND_SUCCESS;
}

/**
 * @brief Implementation of LedBlink Command
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval EXECUTE_COMMAND_RESULT Command Result
 */
EXECUTE_COMMAND_RESULT LedBlink(Azure1_t *Azure1)
{
  (void)(Azure1);
  AZURE_PRINTF("Received LedBlink command\r\n");
  LedMode = AZURE_LED_BLINK;
  return EXECUTE_COMMAND_SUCCESS;
}

/* Handles for direct methods */

/**
 * @brief Implementation of Reboot Direct Method
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval METHODRETURN_HANDLE Direct Method Result
 */
METHODRETURN_HANDLE Reboot(Azure1_t *Azure1)
{
  (void)(Azure1);
  METHODRETURN_HANDLE result = MethodReturn_Create(201, "\"Rebooting\"");
  AZURE_PRINTF("Received Reboot request\r\n");
  g_reboot = true;
  return result;
}

/**
 * @brief Implementation of Quit Direct Method
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @retval METHODRETURN_HANDLE Direct Method Result
 */
METHODRETURN_HANDLE Quit(Azure1_t *Azure1)
{
  (void)(Azure1);
  METHODRETURN_HANDLE result = MethodReturn_Create(201,  "\"I will stop\"");
  g_continueRunning = false;
  AZURE_PRINTF("Received Quit request\r\n");
  return result;
}


/**
 * @brief Implementation of FirmwareUpdate Direct Method
 * @param Azure1_t *Azure1 Pointer to the Model instance
 * @param ascii_char_ptr *FwPackageUri OTA URL
 * @retval METHODRETURN_HANDLE Direct Method Result
 */
METHODRETURN_HANDLE FirmwareUpdate(Azure1_t *Azure1, ascii_char_ptr FwPackageUri)
{
  (void)(Azure1);
  char *StartAddress1=NULL;
  char *StartAddress2=NULL;

  METHODRETURN_HANDLE result = MethodReturn_Create(201, "\"Initiating Firmware Update\"");
  AZURE_PRINTF("Received firmware update request. Use package at: [%s]\r\n", FwPackageUri);

  StartAddress1 = (char *) FwPackageUri;

  /* Try to see if it's a secure connection */
  if(strncmp(StartAddress1, "https://", 8)==0) {
    OTA_Type = 's';
    OTA_PortNum = 443;
    StartAddress1+=8; /* Move at the end of "https://" */
  } else if(strncmp(StartAddress1, "http://", 7)==0) {
    OTA_Type = 't';
    OTA_PortNum = 80;
    StartAddress1+=7; /* Move at the end of "http://" */
  } else {
    AZURE_PRINTF("Err: Parsing the URI for connection type\r\n");
    return MethodReturn_Create(201,  "\"Err: Parsing the URI for connection type\"");
  }

  /* Try to see if there is specific port */
  StartAddress2 = strstr( StartAddress1, ":");
  if(StartAddress2!=NULL) {
    char *StartAddress3=NULL;

    StartAddress2++;
    /* overwrite the standard Port Number */
    OTA_PortNum = atoi(StartAddress2);

    StartAddress3 = strstr( StartAddress2, "/");
    if(StartAddress3==NULL) {
      AZURE_PRINTF("Err: Parsing the FOTA command searching Start File Path\r\n");
      return MethodReturn_Create(201,  "\"Err: Parsing the FOTA command searching Start File Path\"");
    }

    snprintf(OTA_HostName,StartAddress2-StartAddress1,"%s",StartAddress1);
    sprintf(OTA_Path,"%s",StartAddress3);
  } else {
    StartAddress2 = strstr( StartAddress1, "/");
    if(StartAddress2==NULL) {
      AZURE_PRINTF("Err: Parsing the FOTA command searching Start File Path\r\n");
      return MethodReturn_Create(201,  "\"Err: Parsing the FOTA command searching Start File Path\"");
    }

    snprintf(OTA_HostName,StartAddress2+1-StartAddress1,"%s",StartAddress1);
    sprintf(OTA_Path,"%s",StartAddress2);
  }

  /* Make the FOTA */
  g_ExecuteOTA=true;

  return result;
}

/* Handle for Desired Properties */

/**
 * @brief Function for changing the Telemetry Interval
 * @param void *argument Pointer to the Model instance
 * @retval None
 */
void ChangeTelemetryInterval(void* argument)
{
  Azure1_t *Azure = argument;
  if((Azure1->DesiredTelemetryInterval>0) & (Azure1->DesiredTelemetryInterval<=30)){
    Azure1->TelemetryInterval = TargetBoardFeatures.TIM_CC1_Pulse = Azure1->DesiredTelemetryInterval*2000 /* 2Hz Timer Frequency */;
  } else {
    AZURE_PRINTF("Err: Received a Not Allowed desired Telemetry Interval=%d (1<=Allowed<=30)\r\n",Azure->DesiredTelemetryInterval);
  }
}

/**
  * @brief  Fill the Model Instance(date, sensors data)
  * @param  None
  * @retval None
  */
static void FillTheModelInstance(void)
{
  time_t now;
  float SensorValueT;
  float SensorValueH;
  uint8_t Status;
  SensorAxes_t ACC_Value;
  SensorAxes_t GYR_Value;

  /* Read or Emulate the Sensors */
  if(!TargetBoardFeatures.EmulateSns) {
    if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_IsInitialized_IKS01A2 : BSP_TEMPERATURE_IsInitialized)(TargetBoardFeatures.HandleTempSensors[0],&Status)==COMPONENT_OK){
      (TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Get_Temp_IKS01A2 : BSP_TEMPERATURE_Get_Temp)(TargetBoardFeatures.HandleTempSensors[0],(float *)&SensorValueT);
    }

    if(TargetBoardFeatures.HandleHumSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_IsInitialized_IKS01A2 : BSP_HUMIDITY_IsInitialized)(TargetBoardFeatures.HandleHumSensor,&Status)==COMPONENT_OK){
        (TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_Get_Hum_IKS01A2 : BSP_HUMIDITY_Get_Hum)(TargetBoardFeatures.HandleHumSensor,(float *)&SensorValueH);
      }
    }

    /* Read the Acc values */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Axes_IKS01A2 : BSP_ACCELERO_Get_Axes)(TargetBoardFeatures.HandleAccSensor,&ACC_Value);

    /* Read the Gyro values */
    (TargetBoardFeatures.SnsAltFunc ? BSP_GYRO_Get_Axes_IKS01A2 : BSP_GYRO_Get_Axes)(TargetBoardFeatures.HandleGyroSensor,&GYR_Value);
  } else {
    /* Emulatation IkS01A1/IKS01A2 */
    ACC_Value.AXIS_X =  100 + (((int32_t)rand())&0x5FF);
    ACC_Value.AXIS_Y =  300 - (((int32_t)rand())&0x5FF);
    ACC_Value.AXIS_Z = -200 + (((int32_t)rand())&0x5FF);

    GYR_Value.AXIS_X =  30000 + (((int32_t)rand())&0x4FFFF);
    GYR_Value.AXIS_Y =  30000 - (((int32_t)rand())&0x4FFFF);
    GYR_Value.AXIS_Z = -30000 + (((int32_t)rand())&0x4FFFF);

    SensorValueT  = 26.0 + (((int16_t)rand())&0x14);
    SensorValueH  = 50.0 + (((int16_t)rand())&0x1F);
  }

  /* Read the Time  from RTC */
  now = TimingSystemGetSystemTime();
  /* Time Stamp */
  Azure1->ts = GetDateTimeOffset(now);

  /* Update the Azure1 Model */
  Azure1->temperature = SensorValueT;

  Azure1->humidity  = SensorValueH;

  Azure1->accX = ACC_Value.AXIS_X;
  Azure1->accY = ACC_Value.AXIS_Y;
  Azure1->accZ = ACC_Value.AXIS_Z;

  Azure1->gyrX = GYR_Value.AXIS_X;
  Azure1->gyrY = GYR_Value.AXIS_Y;
  Azure1->gyrZ = GYR_Value.AXIS_Z;
}

/**
 * @brief CallBack received for Device Twin status update
 * @param int status_code Status received
 * @param void* userContextCallback unused
 * @retval None
 */
static void DeviceTwinCallback(int status_code, __attribute__((unused)) void* userContextCallback)
{
  StatusBackCount++;
  AZURE_PRINTF("-->DeviceTwin CallBack [%d]: Status_code = %d\r\n", StatusBackCount, status_code);
}

/**
 * @brief CallBack received at the beginning on when there is a change on desired properties
 * @param DEVICE_TWIN_UPDATE_STATE status_code Status received
 * @param unsigned char* payload Payload received (json)
 * @param size_t size size of the Payload received
 * @param void* userContextCallback pointer to the model instance
 * @retval None
 */
static void DeviceTwinCallbackStatus(DEVICE_TWIN_UPDATE_STATE status_code,  const unsigned char* payload, size_t size, void* userContextCallback)
{
  /* Only for Debug */
  //AZURE_PRINTF("DeviceTwinCallbackStatus Method payload: %.*s\r\nStatus_code = %d\r\n", size,(const char*)payload,status_code);

  /* query the DeviceTwin Status */
  {
    JSON_DECODER_RESULT JSONDec;
    MULTITREE_HANDLE multiTreeHandle;
    char* temp = malloc(size + 1);
    if (temp == NULL) {
      AZURE_PRINTF("Err: failed to malloc\r\n");
      return;
    }
    /* We need to add the missing termination char */
    (void)memcpy(temp, payload, size);
    temp[size] = '\0';
    if((JSONDec = JSONDecoder_JSON_To_MultiTree(temp,&multiTreeHandle))!=JSON_DECODER_OK) {
      AZURE_PRINTF("Err: Decoding JSON Code=%d\r\n",JSONDec);
      free(temp);
      return;
    } else {
      AZURE_PRINTF("JSON Decoded\r\n");
    }

    /* Search the Reported Properties Only at the beginning */
    if(ReceivedDeviceTwinProperty==0){
      MULTITREE_HANDLE childHandle;
      ReceivedDeviceTwinProperty=1;
      if(MultiTree_GetChildByName(multiTreeHandle, "reported", &childHandle)== MULTITREE_OK) {
        MULTITREE_HANDLE childHandle2;
        AZURE_PRINTF("Reported Property Found\r\n");
        if(MultiTree_GetChildByName(childHandle, "AzureStatus", &childHandle2) != MULTITREE_OK) {
          AZURE_PRINTF("AzureStatus Reported Property not Found\r\n");
        } else {
          void const *ResultStatus;
          //AZURE_PRINTF("AzureStatus reported Property Found\r\n");
          if(MultiTree_GetValue(childHandle2, &ResultStatus)!= MULTITREE_OK) {
            AZURE_PRINTF("Err: Reading the AzureStatus Reported Property value\r\n");
          } else {
            //AZURE_PRINTF("AzureStatus reported Property value=%s\r\n",(unsigned char *)ResultStatus);
            /* We need to avoid to check the \" because the property is "Applying"  or "Downloading" */
            if(!strncmp(ENUM_TO_STRING(FIRMWARE_UPDATE_STATUS,Applying),((char *)ResultStatus)+1,strlen(ENUM_TO_STRING(FIRMWARE_UPDATE_STATUS,Applying))-1)) {
              /* This for cleanning the Supported Methods list */
              Azure1->SupportedMethods = "null";
              ReportState(ApplyComplete);
            } else if (!strncmp(ENUM_TO_STRING(FIRMWARE_UPDATE_STATUS,Downloading),((char *)ResultStatus)+1,strlen(ENUM_TO_STRING(FIRMWARE_UPDATE_STATUS,Downloading))-1)) {
              ReportState(DownloadFailed);
            }
          }
        }
      }
      /* Check also the Desired Properties */
      if(MultiTree_GetChildByName(multiTreeHandle, "desired", &childHandle)== MULTITREE_OK) {
        MULTITREE_HANDLE childHandle2;
        AZURE_PRINTF("Desired Property Found\r\n");
        if(MultiTree_GetChildByName(childHandle, "DesiredTelemetryInterval", &childHandle2) != MULTITREE_OK) {
          AZURE_PRINTF("DesiredTelemetryInterval Desired Property not Found\r\n");
        } else {
          void const *ResultInterval;
          //AZURE_PRINTF("AzureStatus reported Property Found\r\n");
          if(MultiTree_GetValue(childHandle2, &ResultInterval)!= MULTITREE_OK) {
            AZURE_PRINTF("Err: Reading the DesiredTelemetryInterval Desired Property value\r\n");
          } else {
            Azure1_t *Azure = userContextCallback;
            Azure->DesiredTelemetryInterval= atoi(((char *) ResultInterval));
            AZURE_PRINTF("Desired Telemetry Interval= %d\r\n", Azure->DesiredTelemetryInterval);
            ChangeTelemetryInterval(userContextCallback);
          }
        }
      }
    } else {
      /* if we are not at the beginning... there is a change on the Desided properties */
      MULTITREE_HANDLE childHandle;
      if(MultiTree_GetChildByName(multiTreeHandle, "DesiredTelemetryInterval", &childHandle) != MULTITREE_OK) {
        AZURE_PRINTF("DesiredTelemetryInterval Desired Property not Found\r\n");
      } else {
        void const *ResultInterval;
        if(MultiTree_GetValue(childHandle, &ResultInterval)!= MULTITREE_OK) {
          AZURE_PRINTF("Err: Reading the DesiredTelemetryInterval Desired Property value\r\n");
        } else {
          Azure1_t *Azure = userContextCallback;
          Azure->DesiredTelemetryInterval= atoi(((char *) ResultInterval));
          AZURE_PRINTF("Received a new Desired Telemetry Interval= %d\r\n", Azure->DesiredTelemetryInterval);
          ChangeTelemetryInterval(userContextCallback);
        }
      }
    }
    MultiTree_Destroy(multiTreeHandle);
    free(temp);
  }
}

/**
 * @brief Function for reporting the Model Status
 * @param FIRMWARE_UPDATE_STATUS status that we want send (Running/Downloading...)
 * @retval None
 */
void ReportState(FIRMWARE_UPDATE_STATUS status)
{
  unsigned char *buffer;
  size_t         bufferSize;

  /* Fill the report State */
  sprintf(Azure1->AzureStatus,"%s",ENUM_TO_STRING(FIRMWARE_UPDATE_STATUS,status));

  /*serialize the model using SERIALIZE_REPORTED_PROPERTIES */
  if (SERIALIZE_REPORTED_PROPERTIES(&buffer, &bufferSize, *Azure1) != CODEFIRST_OK) {
    AZURE_PRINTF("Err: Serializing Reported State\r\n");
  } else {
    /* send the data up stream*/
    if(IoTHubClient_LL_SendReportedState(iotHubClientHandle, buffer, bufferSize, DeviceTwinCallback, NULL)!= IOTHUB_CLIENT_OK) {
      AZURE_PRINTF("Err: Failure Sending Reported State\r\n");
    } else {
      SentStatusCount++;
      AZURE_PRINTF("Ok reported State [%d]: %s\r\n", SentStatusCount,Azure1->AzureStatus);
    }
    free(buffer);
  }
}

/* @brief Callback called when one C2D message is received
 * @param IOTHUB_MESSAGE_HANDLE message Pointer to the received message
 * @param void* userContextCallback pointer to the model instance
 * @retval IOTHUBMESSAGE_DISPOSITION_RESULT Results of the messagge (Abandoned/Accepted/...)
 */
static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
  IOTHUBMESSAGE_DISPOSITION_RESULT result;
  const unsigned char* buffer;
  size_t size;
  if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK){
    AZURE_PRINTF("unable to retrieve the message data\r\n");
    result = IOTHUBMESSAGE_ABANDONED;
  } else {
    /*buffer is not zero terminated*/
    char* temp = malloc(size + 1);
    if (temp == NULL) {
      AZURE_PRINTF("Err: failed to malloc\r\n");
      result = IOTHUBMESSAGE_ABANDONED;
    } else {
      EXECUTE_COMMAND_RESULT executeCommandResult;

      AZURE_PRINTF("Received Message with Data: <<<%.*s>>> & Size=%d\r\n", (int)size, buffer, (int)size);

      (void)memcpy(temp, buffer, size);
      temp[size] = '\0';
      executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
      result =
          (executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
          (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
          IOTHUBMESSAGE_REJECTED;
      free(temp);
    }
  }
  return result;
}

/* @brief Callback received when a message is received from IoT
 * @param IOTHUB_CLIENT_CONFIRMATION_RESULT result confirmation result from IoTs
 * @param void* userContextCallback pointer to event instance
 * @retval None
 */
static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
  EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;

  AZURE_PRINTF("Confirmation received for message [%d] with result = %s\r\n", eventInstance->messageTrackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
  MessagesBackCount++;
  free(eventInstance->this);
}

/**
  * @brief  Send Sensors' Data to IoT Hub
  * @param  None
  * @retval None
  */
static void SendSNSData(void)
{
  EVENT_INSTANCE *messages;
  unsigned char* destination;
  size_t destinationSize;

  /* Read the Data from the Sensors */
  FillTheModelInstance();

  messages = (EVENT_INSTANCE *) calloc(1,sizeof(EVENT_INSTANCE));
  if(messages==NULL) {
    AZURE_PRINTF("Err: Allocating Memory for messages to IoT Hub\r\n");
    while(1) {
      ;
    }
  } else {
    messages->this = (void *)messages;
  }

  SentMessagesCount++;
  Azure1->messageId = messages->messageTrackingId = SentMessagesCount;

  if (SERIALIZE(&destination, &destinationSize,
                Azure1->deviceId, Azure1->messageId,
                Azure1->temperature, Azure1->humidity,
                Azure1->accX , Azure1->accY, Azure1->accZ,
                Azure1->gyrX , Azure1->gyrY, Azure1->gyrZ,
                Azure1->ts) != CODEFIRST_OK){
    AZURE_PRINTF("Err: Failed to serialize\r\n");
    while(1) {
      ;
    }
  }

  /* Only for Debug */
  //AZURE_PRINTF("MessageToSend=%.*s\r\n",destinationSize,destination);

  if ((messages->messageHandle = IoTHubMessage_CreateFromByteArray(destination, destinationSize)) == NULL) {
    AZURE_PRINTF("Err: iotHubMessageHandle is NULL!\r\n");
  } else {
    char msgText[32];
    MAP_HANDLE propMap = IoTHubMessage_Properties(messages->messageHandle);
    sprintf_s(msgText, sizeof(msgText), "PropMsg_%zu", SentMessagesCount);
    if (Map_AddOrUpdate(propMap, "PropName", msgText) != MAP_OK){
      AZURE_PRINTF("Err: Map_AddOrUpdate Failed!\r\n");
    }
    if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages->messageHandle, SendConfirmationCallback, messages) != IOTHUB_CLIENT_OK) {
      AZURE_PRINTF("Err: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", SentMessagesCount);
    }
    free(destination);
  }
  IoTHubMessage_Destroy(messages->messageHandle);
}

/**
  * @brief  Callback for user button
  * @param  None
  * @retval None
  */
static void ButtonCallback(void)
{
  AZURE_PRINTF("Button Pressed\r\n");
  g_continueRunning = false;
}

/**
  * @brief  Function for waiting that the IoT has received all the messages/status sent
  * @param  None
  * @retval None
  */
void WaitAllTheMessages(void)
{
  /* Wait All the Reported State/Message confirmation */
  while((StatusBackCount<SentStatusCount) | (MessagesBackCount<SentMessagesCount)) {
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    HAL_Delay(10);
  }
}

/**
  * @brief  Main MQTT application for Telemetry and Device Management
  * @param  None
  * @retval None
  */
void AzureClient_mqtt_DM_TM(void)
{
  if (platform_init() != 0){
    AZURE_PRINTF("Failed to initialize the platform.\r\n");
    AzureExit(AZURE_ERR_PLATFORM_INIT);
  } else {
    char MAC_RegisterdAddress[13];
    g_continueRunning = true;
    AZURE_PRINTF("Platform Init Done\r\n");
    {
        int32_t Read;
        int32_t Write=0;
        for(Read=0;Read<17;Read++) {
          if(macaddstart[Read]!=':') {
            MAC_RegisterdAddress[Write] = macaddstart[Read];
            Write++;
          }
          MAC_RegisterdAddress[Write]='\0'; // Termination
        }
      }

    /* Led on for signaling the end of Platform init phase */
    BSP_LED_On(LED2);

    if (serializer_init(NULL) != SERIALIZER_OK) {
      AZURE_PRINTF("Err: Unable to Initialize Serializer\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    }
    AZURE_PRINTF("Serializer Initialized\r\n");

#ifdef AZURE_ENABLE_REGISTRATION
    /* Board Registration Procedure */
    if(RegistrationAgent(&connectionString)!=0) {
      AZURE_PRINTF("Err: During Board Registration\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("Board Registration Done\r\n\r\n");
      AZURE_PRINTF("Connect to:\r\n");
      AZURE_PRINTF("https://stm32ode.azurewebsites.net/Home/Index/%s\r\n\r\n",MAC_RegisterdAddress);
    }
#else /* AZURE_ENABLE_REGISTRATION */
    if(connectionString==NULL) {
      AZURE_PRINTF("Err: connectionString is NULL!\r\n");
      AZURE_PRINTF("\tchange AZUREIOTHUBCONNECTIONSTRING on Inc/azure1_config.h file\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    }
#endif /* AZURE_ENABLE_REGISTRATION */

    if((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL){
      AZURE_PRINTF("Err: iotHubClientHandle is NULL!\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("iotHubClientHandle Created\r\n");
    }

    {
      bool traceOn = true;
      if(IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn)==IOTHUB_CLIENT_OK) {
        AZURE_PRINTF("iotHubClientSetOption logtrace Ok\r\n");
      }
    }

    {
      int32_t KeepAlive = 10*60; /* 10 Minutes */
      if(IoTHubClient_LL_SetOption(iotHubClientHandle, "keepalive", &KeepAlive)==IOTHUB_CLIENT_OK) {
        AZURE_PRINTF("iotHubClientSetOption keepalive Ok\r\n");
      }
    }

    Azure1 = CREATE_MODEL_INSTANCE(AzureModel, Azure1_t);
    if (Azure1 == NULL){
      AZURE_PRINTF("Err: Creating the Model Instance\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("Model Instance Created\r\n");
    }

    /* Allocate Space for storing Board ID */
    Azure1->deviceId = (char *) calloc(1,32);
    if(Azure1->deviceId==NULL) {
      AZURE_PRINTF("Err Allocating memory\r\n\tFile %s Line %d\r\n",__FILE__, __LINE__);
      AzureExit(AZURE_ERR_MALLOC);
    }

    /* Allocate Space for storing Device Status */
    Azure1->AzureStatus = (char *) calloc(1,32);
    if(Azure1->AzureStatus==NULL) {
      AZURE_PRINTF("Err Allocating memory\r\n\tFile %s Line %d\r\n",__FILE__, __LINE__);
      AzureExit(AZURE_ERR_MALLOC);
    }

    /* Allocate Space for storing Device Firmware Version */
    Azure1->AzureFwVersion = (char *) calloc(1,64);
    if(Azure1->AzureFwVersion==NULL) {
      AZURE_PRINTF("Err Allocating memory\r\n\tFile %s Line %d\r\n",__FILE__, __LINE__);
      AzureExit(AZURE_ERR_MALLOC);
    }

    /* Supported Methods */
    Azure1->SupportedMethods = "{\"Reboot\": \"Reboot the device\",\
                                 \"Quit\": \"Stop the trasmission\",\
                                 \"FirmwareUpdate--FwPackageUri-string\": \"Updates device Firmware.\
                                   Use parameter FwPackageUri to specifiy the URI of the firmware file\"}";

    /* Fixed Data Fields an reported Properties */
    sprintf(Azure1->AzureFwVersion,"%s V%c.%c.%c SDK=%s", AZURE_PACKAGENAME,
            AZURE_VERSION_MAJOR,AZURE_VERSION_MINOR,AZURE_VERSION_PATCH,IoTHubClient_GetVersionString());

    /* Board Identification */
    sprintf(Azure1->deviceId,"%s",MAC_RegisterdAddress);

    /* Telemetry Intervall */
    Azure1->TelemetryInterval = TargetBoardFeatures.TIM_CC1_Pulse/2000 /* 2Hz Timer Frequency */;

    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, Azure1) != IOTHUB_CLIENT_OK){
      AZURE_PRINTF("Err: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SetMessageCallback...successful.\r\n");
    }

    if(IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, Azure1) != IOTHUB_CLIENT_OK){
      AZURE_PRINTF("Err: IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SetDeviceMethodCallback...successful.\r\n");
    }

#ifdef USE_MBED_TLS
    /* Load certificates */
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK ) {
      AZURE_PRINTF("Err: IoTHubClient_LL_SetOption(certificates)..........FAILED!\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SetOption(certificates)...successful.\r\n");
    }
#endif  /* USE_MBED_TLS */

    if(IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, DeviceTwinCallbackStatus, Azure1) != IOTHUB_CLIENT_OK){
      AZURE_PRINTF("Err: IoTHubClient_LL_SetDeviceTwinCallback..........FAILED!\r\n");
      AzureExit(AZURE_ERR_IOT_START);
    } else {
      AZURE_PRINTF("IoTHubClient_LL_SetDeviceTwinCallback...successful.\r\n");
    }

    /* Wait the Answer from IOT */
    while(ReceivedDeviceTwinProperty==0){
      IoTHubClient_LL_DoWork(iotHubClientHandle);
      HAL_Delay(100);
    }

    /* Led off for signaling the end IOT connection procedure */
    BSP_LED_Off(LED2);

    /* Supported Methods */
    Azure1->SupportedMethods = "{\"Reboot\": \"Reboot the device\",\
                                 \"Quit\": \"Stop the trasmission\",\
                                 \"FirmwareUpdate--FwPackageUri-string\": \"Updates device Firmware.\
                                   Use parameter FwPackageUri to specifiy the URI of the firmware file\"}";

    StartTimer1();

    ReportState(Running);

    /* Now that we are ready to receive commands and send messages */
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
        /* Led Toggle */
        if(LedMode == AZURE_LED_BLINK) {
          BSP_LED_Toggle(LED2);
        }
        SendSNSData();
      }

      /* Execute the FOTA */
      if(g_ExecuteOTA) {
        g_ExecuteOTA=false;
        /* Stop the Sensor Data Trasmission */
        StopTimer1();
        if(FOTACallback(OTA_HostName,OTA_Type,OTA_PortNum,OTA_Path)==OTA_STATUS_NULL) {
          /* Everything was ok */
          AZURE_PRINTF("The Board will restart in for Appling the OTA\r\n");
          ReportState(Applying);
          WaitAllTheMessages();
          AZURE_PRINTF("Call to HAL_NVIC_SystemReset\r\n");
          HAL_NVIC_SystemReset();
        } else {
          /* Something Wrong */
          ReportState(DownloadFailed);
          WaitAllTheMessages();
          AZURE_PRINTF("The Board will wait 5 Seconds before to send Sensor Data again\r\n");
          HAL_Delay(5000);
          /* Restart the Sensor Data Trasmission */
          StartTimer1();
          ReportState(Running);
        }
      }

      /* Reboot System */
      if(g_reboot) {
        g_reboot = false;
        ReportState(Rebootting);
        WaitAllTheMessages();
        AZURE_PRINTF("Call to HAL_NVIC_SystemReset\r\n");
        HAL_NVIC_SystemReset();
      }
      IoTHubClient_LL_DoWork(iotHubClientHandle);
    }

    StopTimer1();

    AZURE_PRINTF("iothub_client_sample_mqtt has gotten quit message\r\n");

    ReportState(Ended);

    WaitAllTheMessages();

    free(Azure1->deviceId);
    free(Azure1->AzureStatus);
    free(Azure1->AzureFwVersion);
    DESTROY_MODEL_INSTANCE(Azure1);

    IoTHubClient_LL_Destroy(iotHubClientHandle);
    AZURE_PRINTF("iotHubClientHandle Destroied\r\n");

    serializer_deinit();
    platform_deinit();
    AZURE_PRINTF("Platform DeInit\r\n");
  }
}


#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT

#include "mbedtls/entropy_poll.h"
/**
  * @brief Function for initializing the HW for Random Generator unit
  * @param void* Data
  * @param unsigned char* Output
  * @param size_t Len Lenght of the Input
  * @param size_t* oLen Lenght of the Output
  * @retval int ok/error (0/-1)
  */
int mbedtls_hardware_poll( void *Data, unsigned char *Output, size_t Len, size_t *oLen )
{
  uint32_t index;
  uint32_t randomValue;
		
  for (index = 0; index < Len/4; index++) {
    if (HAL_RNG_GenerateRandomNumber(&TargetBoardFeatures.RngHandle, &randomValue) == HAL_OK) {
      *oLen += 4;
      memset(&(Output[index * 4]), (int)randomValue, 4);
    } else {
      AZURE_PRINTF("Err random number\r\n");
      return -1;
    }
  }
  return 0;
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */

/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
