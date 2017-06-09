#include <stdio.h>
#include <stdlib.h>
#include "STM32CubeInterface.h"
#include "RegistrationAgent.h"

#ifdef AZURE_ENABLE_REGISTRATION


//#define DEBUG_REGISTRATION_AGENT

//#define REG_AGENTE_RELEASE




/* Exported Variables */
Agent_Status_t RegistrationRequest=AGENT_STOP;
uint8_t BufferServiceAnswer[SIZE_BUFFER_ANSWERSERVICE];
char MAC_RegisterdAddress[13];

/* Imported Variables */
extern uint8_t macaddstart[32];

/* Local Defines */

#define REGISTRATIONAGENT_DEFAULT_ENDPOINT_TCPPORT      443
#define AZURE_OFFSET_WEBSERVICE_ANSWER_BODY             8  // FOR "Token":"
#define AZURE_CONNSTRING_TOKEN_SIZE                     56  // TO BE CHECK MAX SIZE TOKEN FOR AZURE TOKEN. ALWAYS 44?
#define AZURE_FIXED_SIZE_CONNSTRING                     104 // Included margin
#define TEMP_BUFFER_SIZE                                512
/* Default IoT HUB */
#define REGISTRATIONAGENT_DEFAULT_IOT_HUB               "STM32IoTHub.azure-devices.net"

/* STM32 Unique ID */
#ifdef USE_STM32F4XX_NUCLEO
#define STM32_UUID ((uint32_t *)0x1FFF7A10)
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO
#define STM32_UUID ((uint32_t *)0x1FFF7590)
#endif /* USE_STM32L4XX_NUCLEO */

/* Local prototypes */
static Agent_Status_t SendWebServiceMessage(uint8_t socketHandle, char *MAC_RegisterdAddress, char *UUID_Nucleo, const char* MessageType);

/* Local Variables */
uint8_t RegistrationDefaultEndPointIPAddress[36];

/**
  * @brief  Register device and retrieve connection string  
  * @param  char **connectionString pointer to the Connection string
  * @retval int32_t value for success/failure (1/0)
  */
int32_t RegistrationAgent(char **connectionString)
{
  int32_t RetValue=0;
  RegistrationDefaultEndPointIPAddress[0 ] = 's';
  RegistrationDefaultEndPointIPAddress[1 ] = 't';
  RegistrationDefaultEndPointIPAddress[2 ] = 'm';
  RegistrationDefaultEndPointIPAddress[3 ] = '3';
  RegistrationDefaultEndPointIPAddress[4 ] = '2';
  RegistrationDefaultEndPointIPAddress[5 ] = 'o';
  RegistrationDefaultEndPointIPAddress[6 ] = 'd';
  RegistrationDefaultEndPointIPAddress[7 ] = 'e';
  RegistrationDefaultEndPointIPAddress[8 ] = '-';
  RegistrationDefaultEndPointIPAddress[9 ] = 'r';
  RegistrationDefaultEndPointIPAddress[10] = 'e';
  RegistrationDefaultEndPointIPAddress[11] = 'g';
  RegistrationDefaultEndPointIPAddress[12] = 'i';
  RegistrationDefaultEndPointIPAddress[13] = 's';
  RegistrationDefaultEndPointIPAddress[14] = 't';
  RegistrationDefaultEndPointIPAddress[15] = 'e';
  RegistrationDefaultEndPointIPAddress[16] = 'r';
  RegistrationDefaultEndPointIPAddress[17] = '.';
  RegistrationDefaultEndPointIPAddress[18] = 'a';
  RegistrationDefaultEndPointIPAddress[19] = 'z';
  RegistrationDefaultEndPointIPAddress[20] = 'u';
  RegistrationDefaultEndPointIPAddress[21] = 'r';
  RegistrationDefaultEndPointIPAddress[22] = 'e';
  RegistrationDefaultEndPointIPAddress[23] = 'w';
  RegistrationDefaultEndPointIPAddress[24] = 'e';
  RegistrationDefaultEndPointIPAddress[25] = 'b';
  RegistrationDefaultEndPointIPAddress[26] = 's';
  RegistrationDefaultEndPointIPAddress[27] = 'i';
  RegistrationDefaultEndPointIPAddress[28] = 't';
  RegistrationDefaultEndPointIPAddress[29] = 'e';
  RegistrationDefaultEndPointIPAddress[30] = 's';
  RegistrationDefaultEndPointIPAddress[31] = '.';
  RegistrationDefaultEndPointIPAddress[32] = 'n';
  RegistrationDefaultEndPointIPAddress[33] = 'e';
  RegistrationDefaultEndPointIPAddress[34] = 't';
  RegistrationDefaultEndPointIPAddress[35] = '\0';
  
  /* Memory Allocation */
  (*connectionString) = malloc (sizeof(REGISTRATIONAGENT_DEFAULT_IOT_HUB)+AZURE_FIXED_SIZE_CONNSTRING);
  /* Control the pointer */
  if((*connectionString)==NULL) {
     AZURE_PRINTF("Err Allocating the Connection String\r\n");
     RetValue=1;
  } else {
    uint8_t socketHandle;    
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    AZURE_PRINTF("Registration Agent Launched\r\n");
    status = wifi_socket_client_open(RegistrationDefaultEndPointIPAddress,
                                     REGISTRATIONAGENT_DEFAULT_ENDPOINT_TCPPORT, "s", &socketHandle);

    if(status==WiFi_MODULE_SUCCESS) {
      char UUID_Nucleo[32];

      /* Board UID */
      sprintf (UUID_Nucleo,"%.10d%.10d%.10d",STM32_UUID[0],STM32_UUID[1],STM32_UUID[2]);
      /* WIFI Mac Address removing the ":" from MAC like 00:80:E1:B8:87:1F */
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
      //Compose and send GET-Credentials message
      RegistrationRequest = AGENT_START;
      if(SendWebServiceMessage(socketHandle,MAC_RegisterdAddress,UUID_Nucleo,"GET") == AGENT_ERROR ){
        AZURE_PRINTF("[Registration][E]. Failed to send GET message\r\n");
        return AGENT_ERROR;
      }

      /* Wait the Answers of the GET command */
      while(RegistrationRequest!=AGENT_DATA) {
        HAL_Delay(100);
      }
      RegistrationRequest = AGENT_STOP;

      /* Retrieve and parse answer for GET-Credentials message */
      if(strstr((char *)(BufferServiceAnswer), "not found") == NULL) {
        char * pConnString;
        char connToken[AZURE_CONNSTRING_TOKEN_SIZE];
        int i = 0;

        AZURE_PRINTF("[Registration]. Devices founded\r\n");

        pConnString = strstr((char *)(BufferServiceAnswer), "Token");

        if ( pConnString != NULL){
          pConnString += AZURE_OFFSET_WEBSERVICE_ANSWER_BODY;
          while ( *pConnString != '"' ){
                 connToken[i] = *pConnString;
                 pConnString++;
                 i++;
          }
          connToken[i] = '\0';
#ifdef DEBUG_REGISTRATION_AGENT
          AZURE_PRINTF("[Registration]. HostName is is [%s] \r\n", REGISTRATIONAGENT_DEFAULT_IOT_HUB);
          AZURE_PRINTF("[Registration]. DeviceId is is [%s] \r\n", MAC_RegisterdAddress);
          AZURE_PRINTF("[Registration]. Token is is [%s] \r\n", connToken);
#endif /* DEBUG_REGISTRATION_AGENT */

          sprintf(*connectionString,"HostName=%s;DeviceId=%s;SharedAccessKey=%s",REGISTRATIONAGENT_DEFAULT_IOT_HUB, MAC_RegisterdAddress, connToken );
#ifdef DEBUG_REGISTRATION_AGENT
          AZURE_PRINTF("[Registration]. Connection string is %s \r\n", *connectionString);
#endif /* DEBUG_REGISTRATION_AGENT */
        } else {
          AZURE_PRINTF("[Registration] [E]. Err retriving the connection string\r\n");
          RetValue =1;
        }
      } else {
        char * pConnString;
        char connToken[AZURE_CONNSTRING_TOKEN_SIZE];
        int i = 0;
        AZURE_PRINTF("[Registration]. Devices not founded\r\n");
        /* Compose and send POST-Register message */
        RegistrationRequest = AGENT_START;
        if(SendWebServiceMessage(socketHandle,MAC_RegisterdAddress,UUID_Nucleo,"POST") == AGENT_ERROR ){
          AZURE_PRINTF("[Registration][E]. Failed to send POST message\r\n");
          RetValue =1;
          return RetValue;
        }

        /* Wait the Answers of the POST command */
        while(RegistrationRequest!=AGENT_DATA) {
          HAL_Delay(100);
        }
        RegistrationRequest = AGENT_STOP;
        AZURE_PRINTF("[Registration]. Devices founded\r\n");

        pConnString = strstr((char *)(BufferServiceAnswer), "Token");

        if ( pConnString != NULL){
          pConnString += AZURE_OFFSET_WEBSERVICE_ANSWER_BODY;
          while ( *pConnString != '"' ){
                 connToken[i] = *pConnString;
                 pConnString++;
                 i++;
          }
          connToken[i] = '\0';          
          sprintf(*connectionString,"HostName=%s;DeviceId=%s;SharedAccessKey=%s",REGISTRATIONAGENT_DEFAULT_IOT_HUB, MAC_RegisterdAddress, connToken );
#ifdef DEBUG_REGISTRATION_AGENT
          AZURE_PRINTF("[Registration]. Token is is %s \r\n", connToken);
          AZURE_PRINTF("[Registration]. Connection string is %s \r\n", *connectionString);
#endif /* DEBUG_REGISTRATION_AGENT */
        } else {
          AZURE_PRINTF("[Registration] [E]. Err retriving the connection string\r\n");
          RetValue =1;
          return RetValue;
        }
      }
      status = wifi_socket_client_close(socketHandle);
      if(status!=WiFi_MODULE_SUCCESS) {
        AZURE_PRINTF("Err closing socket for Board registration\r\n");
        RetValue =1;
      }
    } else {
      AZURE_PRINTF("[Registration][E].Failed connect with Azure service.\r\n");
      RetValue =1;
    }
  }

  /* If Everything ok */
  if(RetValue==0) {
    AZURE_PRINTF("[Registration]. Device Regsitration to Microsoft Azure Successfully Completed\r\n");
    AZURE_PRINTF("[Registration]. Connection String=\r\n\t%s\r\n",*connectionString);
  }
  return RetValue;
}


static Agent_Status_t SendWebServiceMessage(uint8_t socketHandle, char *MAC_RegisterdAddress, char *UUID_Nucleo, const char* MessageType)
{
  char    buf[TEMP_BUFFER_SIZE];
  char    postContent[AZURE_CONNSTRING_TOKEN_SIZE];
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  if (strcmp(MessageType,"GET")==0) {
    sprintf(buf,"GET /api/tokens/%s HTTP/1.1\r\n",MAC_RegisterdAddress);
  } else if (strcmp(MessageType,"POST")==0) {
#ifdef REG_AGENTE_RELEASE
    //Release
    sprintf(buf,"POST /api/tokens/ HTTP/1.1\r\n");
#else /* REG_AGENTE_RELEASE */
    //Demo
    sprintf(buf,"POST /api/tokens/?demo=true HTTP/1.1\r\n");    
#endif /* REG_AGENTE_RELEASE */
    sprintf(postContent,"\"%s+%s\"", UUID_Nucleo, MAC_RegisterdAddress);
  } else {
    return AGENT_ERROR;
  }
   
#ifdef DEBUG_REGISTRATION_AGENT
  AZURE_PRINTF("\r\n[Registration]. Sending request: %s \r\n", MessageType);
  AZURE_PRINTF("\r\n[Registration]. 1) String: %s \r\n", buf);
#endif /* DEBUG_REGISTRATION_AGENT */

  status = wifi_socket_client_write(socketHandle, strlen(buf), buf);
  if(status!=WiFi_MODULE_SUCCESS){
    return AGENT_ERROR;
  } else {
    HAL_Delay(200);
  }

  /* Add Header fields */
  sprintf(buf,"Host: %s\r\n",RegistrationDefaultEndPointIPAddress);
#ifdef DEBUG_REGISTRATION_AGENT  
  AZURE_PRINTF("\r\n[Registration]. 2) String: %s \r\n", buf);
#endif /* DEBUG_REGISTRATION_AGENT */
  status = wifi_socket_client_write(socketHandle, strlen(buf), buf);
  if(status!=WiFi_MODULE_SUCCESS){
    return AGENT_ERROR;
  } else {
    HAL_Delay(200);
  }

  if (strcmp(MessageType,"POST")==0) {
    /* Add content lenght */
    sprintf(buf,"Content-Length: %d\r\n", strlen(postContent));
#ifdef DEBUG_REGISTRATION_AGENT
    AZURE_PRINTF("\r\n[Registration]. 3) String: %s \r\n", postContent);
#endif /* DEBUG_REGISTRATION_AGENT */
    status = wifi_socket_client_write(socketHandle, strlen(buf), buf);
    if(status!=WiFi_MODULE_SUCCESS){
      return AGENT_ERROR;
    } else {
      HAL_Delay(200);
    }

    /* add content-type */
    sprintf(buf,"Content-Type: application/json\r\n");
#ifdef DEBUG_REGISTRATION_AGENT
    AZURE_PRINTF("\r\n[Registration]. 4) String: %s \r\n", buf);
#endif /* DEBUG_REGISTRATION_AGENT */
    status = wifi_socket_client_write(socketHandle, strlen(buf), buf);
    if(status!=WiFi_MODULE_SUCCESS){
      return AGENT_ERROR;
    } else {
      HAL_Delay(200);
    }
  }

  /* Send end line */
  sprintf(buf,"\r\n");
#ifdef DEBUG_REGISTRATION_AGENT
  AZURE_PRINTF("\r\n[Registration]. 5) String: %s \r\n", buf);
#endif /*DEBUG_REGISTRATION_AGENT */
  status = wifi_socket_client_write(socketHandle, strlen(buf), buf);
  if(status!=WiFi_MODULE_SUCCESS){
    return AGENT_ERROR;
  } else {
    HAL_Delay(200);
  }

  if (strcmp(MessageType,"POST")==0) {
#ifdef DEBUG_REGISTRATION_AGENT
  AZURE_PRINTF("\r\n[Registration]. 6) String: %s \r\n", postContent);
#endif /*DEBUG_REGISTRATION_AGENT */
    status = wifi_socket_client_write(socketHandle,strlen(postContent), postContent);
    if(status!=WiFi_MODULE_SUCCESS){
      return AGENT_ERROR;
    } else {
      HAL_Delay(200);
    }
    /* Send end line */
    sprintf(buf,"\r\n");
#ifdef DEBUG_REGISTRATION_AGENT
  AZURE_PRINTF("\r\n[Registration]. 7) String: %s \r\n", buf);
#endif /*DEBUG_REGISTRATION_AGENT */
    status = wifi_socket_client_write(socketHandle, strlen(buf), buf);
    if(status!=WiFi_MODULE_SUCCESS){
      return AGENT_ERROR;
    } else {
      HAL_Delay(200);
    }
  }
  return AGENT_SUCCESS;
}

#endif /* AZURE_ENABLE_REGISTRATION */
