/**
******************************************************************************
* File    readme.txt  
* Version V3.0.0
* Date    21-April-2017
******************************************************************************
* Attention
*
* COPYRIGHT(c) 2017 STMicroelectronics
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

Application Description:

FP-CLD-AZURE1 provides a software running on STM32 which offers a complete middleware to build applications based on 
WiFi connectivity (SPW01SA) and to connect STM32 Nucleo boards with Microsoft Azure IoT services.

FP-CLD-AZURE1 is an function package software for STM32Cube. The software runs on the STM32 micro-controller and 
includes drivers that recognize WiFi module (SPWF01SA), sensor devices and dynamic NFC/RFID tag (M24SR64-Y).
It also includes the porting of the Microsoft Azure IoT device SDK for easy connection with 
Azure IoT services. The expansion software is built on STM32Cube software technology 
to ease portability across different STM32 microcontrollers. The software comes with examples for registering the 
device on Microsoft Azure IoT Hub, transmit data and receive commands, and allow Device Management with FOTA capability

This application allows the user to control the initialization phase via UART.
Launch a terminal application and set the UART port to 460800 bps, 8 bit, No Parity, 1 stop bit
                               -------------------
                               | VERY IMPORTANT: |
                               -------------------
1) This example support the Firmware-Over-The-Air (FOTA) update.
2) This example must run starting at address 0x08004000 in memory and works ONLY if the BootLoader 
is saved at the beginning of the FLASH (address 0x08000000)

Inside the Binary Directory there are the following binaries:
Binary/
+-- Azure_Sns_DM.bin    (Program without BootLoader. COULD BE USED     for FOTA)
+-- Azure_Sns_DM_BL.bin (Program with BootLoader.    COULD NOT BE USED for FOTA)


For each IDE (IAR/µVision/System Workbench) there are some scripts CleanAzure1mbedTLS.bat/CleanAzure1mbedTLS.sh that makes the following operations:
- Full Flash Erase
- Load the BootLoader on the rigth flash region
- Load the Program (after the compilation) on the rigth flash region (This could be used for a FOTA)
- Dump back one single binary that contain BootLoader+Program that could be
 flashed at the flash beginning (address 0x08000000) (This COULD BE NOT used for FOTA)
- Reset the board
 
This firmware package includes Components Device Drivers, Board Support Package and example application for the following STMicroelectronics elements:
- X-NUCLEO-IDW01M1 Wi-Fi evaluation board based on the SPWF01SA module
- X-NUCLEO-IKS01A1 Expansion board for four MEMS sensor devices:
      HTS221, LPS25H, LSM6DS0, LSM6DS3, LIS3MDL
- X-NUCLEO-IKS01A2 Expansion board for four MEMS sensor devices:
       HTS221, LPS22HB, LSM6DSL, LSM303AGR
- X-NUCLEO-NFC01A1 a Dynamic NFC tag evaluation board
- NUCLEO-L476RG Nucleo board

@par Hardware and Software environment

  - This example runs on Sensor expansion board attached to STM32L476RG device
    can be easily tailored to any other supported device and development board.

@par STM32Cube packages:
  - STM32L4xx drivers from STM32CubeL4 V1.5.1
@par X-CUBE packages:
  - X-CUBE-WIFI1 V2.1.1
  - X-CUBE-NFC1 V2.5.0
  - X-CUBE-MEMS1 V3.0.0 Modified  
@par Middlewares packages:
  - mbedTLS
  - Azure SDK V1.1.6
  - MetaDataManager V0.8.0
  - LibNDEF V1.0.0


@par How to use it ? 

This package contains projects for 3 IDEs viz. IAR, µVision and System Workbench. 
In order to make the  program work, you must do the following:
 - WARNING: before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.

For IAR:
 - Open IAR toolchain (this firmware has been successfully tested with Embedded Workbench V7.80.4).
 - Open the IAR project file EWARM\STM32L476RG-Nucleo\Azure1_Sns_DM.eww
 - Rebuild all files and run the CleanAzure_Sns_DM.bat script that you find on the same directory

For µVision:
 - Open µVision toolchain (this firmware has been  successfully tested with MDK-ARM Professional Version: 5.22.0).
 - Open the µVision project file MDK-ARM\STM32L476RG-Nucleo\Project.uvprojx
 - Rebuild all files and run the CleanAzure_Sns_DM.bat script that you find on the same directory
 
For System Workbench:
 - Open System Workbench for STM32 (this firmware has been successfully tested with System Workbench for STM32 Version 1.14.0.201703061529).
 - Set the default workspace proposed by the IDE (please be sure that there are not spaces in the workspace path).
 - Press "File" -> "Import" -> "Existing Projects into Workspace"; press "Browse" in the "Select root directory" and choose the path where the System
   Workbench project is located (it should be SW4STM32\STM32L476RG-Nucleo\ ). 
 - Rebuild all files and and run:
   - if you are on windows and you had installed the STM32 ST-Link utility:
	     the CleanAzure_Sns_DM.bat script that you find on the same directory
   - Otherwise (Linux/iOS or Windows without the STM32 ST-Link Utility):
	     the CleanAzure_Sns_DM.sh script that you find on the same directory but you must add the right installation path like requested
             
             
Message format for Commands:
{"Name" : "Pause", "Parameters" : {}}

Directed method Payload for FOTA:
{"FwPackageUri" : "https://SERVER_NAME.windows.net/FIRMWARE_PATH/BINARY_NAME.bin"}

 /******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/