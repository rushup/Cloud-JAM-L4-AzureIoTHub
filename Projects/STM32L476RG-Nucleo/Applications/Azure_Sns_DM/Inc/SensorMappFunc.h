/**
  ******************************************************************************
  * @file    SensorMappFunc.h 
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Remapping function for Not Nucleo platforms
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSOR_MAP_FUNC_H
#define __SENSOR_MAP_FUNC_H

__SENSOR_MAP_FUNC_H

/* Exported define ------------------------------------------------------------*/

/* Humidity */
#define BSP_HUMIDITY_Init_IKS01A2 BSP_HUMIDITY_Init
#define BSP_HUMIDITY_DeInit_IKS01A2 BSP_HUMIDITY_DeInit
#define BSP_HUMIDITY_Sensor_Enable_IKS01A2 BSP_HUMIDITY_Sensor_Enable
#define BSP_HUMIDITY_Sensor_Disable_IKS01A2 BSP_HUMIDITY_Sensor_Disable
#define BSP_HUMIDITY_IsInitialized_IKS01A2 BSP_HUMIDITY_IsInitialized
#define BSP_HUMIDITY_IsEnabled_IKS01A2 BSP_HUMIDITY_IsEnabled
#define BSP_HUMIDITY_IsCombo_IKS01A2 BSP_HUMIDITY_IsCombo
#define BSP_HUMIDITY_Get_Instance_IKS01A2 BSP_HUMIDITY_Get_Instance
#define BSP_HUMIDITY_Get_WhoAmI_IKS01A2 BSP_HUMIDITY_Get_WhoAmI
#define BSP_HUMIDITY_Check_WhoAmI_IKS01A2 BSP_HUMIDITY_Check_WhoAmI
#define BSP_HUMIDITY_Get_Hum_IKS01A2 BSP_HUMIDITY_Get_Hum
#define BSP_HUMIDITY_Get_ODR_IKS01A2 BSP_HUMIDITY_Get_ODR
#define BSP_HUMIDITY_Set_ODR_IKS01A2 BSP_HUMIDITY_Set_ODR
#define BSP_HUMIDITY_Set_ODR_Value_IKS01A2 BSP_HUMIDITY_Set_ODR_Value
#define BSP_HUMIDITY_Read_Reg_IKS01A2 BSP_HUMIDITY_Read_Reg
#define BSP_HUMIDITY_Write_Reg_IKS01A2 BSP_HUMIDITY_Write_Reg
#define BSP_HUMIDITY_Get_DRDY_Status_IKS01A2 BSP_HUMIDITY_Get_DRDY_Status

/* Gryo */
#define BSP_GYRO_Init_IKS01A2 BSP_GYRO_Init
#define BSP_GYRO_DeInit_IKS01A2 BSP_GYRO_DeInit
#define BSP_GYRO_Sensor_Enable_IKS01A2 BSP_GYRO_Sensor_Enable
#define BSP_GYRO_Sensor_Disable_IKS01A2 BSP_GYRO_Sensor_Disable
#define BSP_GYRO_IsInitialized_IKS01A2 BSP_GYRO_IsInitialized
#define BSP_GYRO_IsEnabled_IKS01A2 BSP_GYRO_IsEnabled
#define BSP_GYRO_IsCombo_IKS01A2 BSP_GYRO_IsCombo
#define BSP_GYRO_Get_Instance_IKS01A2 BSP_GYRO_Get_Instance
#define BSP_GYRO_Get_WhoAmI_IKS01A2 BSP_GYRO_Get_WhoAmI
#define BSP_GYRO_Check_WhoAmI_IKS01A2 BSP_GYRO_Check_WhoAmI
#define BSP_GYRO_Get_Axes_IKS01A2 BSP_GYRO_Get_Axes
#define BSP_GYRO_Get_AxesRaw_IKS01A2 BSP_GYRO_Get_AxesRaw
#define BSP_GYRO_Get_Sensitivity_IKS01A2 BSP_GYRO_Get_Sensitivity
#define BSP_GYRO_Get_ODR_IKS01A2 BSP_GYRO_Get_ODR
#define BSP_GYRO_Set_ODR_IKS01A2 BSP_GYRO_Set_ODR
#define BSP_GYRO_Set_ODR_Value_IKS01A2 BSP_GYRO_Set_ODR_Value
#define BSP_GYRO_Get_FS_IKS01A2 BSP_GYRO_Get_FS
#define BSP_GYRO_Set_FS_IKS01A2 BSP_GYRO_Set_FS
#define BSP_GYRO_Set_FS_Value_IKS01A2 BSP_GYRO_Set_FS_Value
#define BSP_GYRO_Get_Axes_Status_IKS01A2 BSP_GYRO_Get_Axes_Status
#define BSP_GYRO_Set_Axes_Status_IKS01A2 BSP_GYRO_Set_Axes_Status
#define BSP_GYRO_Read_Reg_IKS01A2 BSP_GYRO_Read_Reg
#define BSP_GYRO_Write_Reg_IKS01A2 BSP_GYRO_Write_Reg
#define BSP_GYRO_Get_DRDY_Status_IKS01A2 BSP_GYRO_Get_DRDY_Status

#define BSP_GYRO_FIFO_Set_ODR_Value_Ext_IKS01A2 BSP_GYRO_FIFO_Set_ODR_Value_Ext
#define BSP_GYRO_FIFO_Get_Full_Status_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Full_Status_Ext
#define BSP_GYRO_FIFO_Get_Empty_Status_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Empty_Status_Ext
#define BSP_GYRO_FIFO_Get_Overrun_Status_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Overrun_Status_Ext
#define BSP_GYRO_FIFO_Get_Pattern_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Pattern_Ext
#define BSP_GYRO_FIFO_Get_Data_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Data_Ext
#define BSP_GYRO_FIFO_Get_Num_Of_Samples_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Num_Of_Samples_Ext
#define BSP_GYRO_FIFO_Set_Decimation_Ext_IKS01A2 BSP_GYRO_FIFO_Set_Decimation_Ext
#define BSP_GYRO_FIFO_Get_Axis_Ext_IKS01A2 BSP_GYRO_FIFO_Get_Axis_Ext
#define BSP_GYRO_FIFO_Set_Mode_Ext_IKS01A2 BSP_GYRO_FIFO_Set_Mode_Ext
#define BSP_GYRO_FIFO_Set_INT1_FIFO_Full_Ext_IKS01A2 BSP_GYRO_FIFO_Set_INT1_FIFO_Full_Ext
#define BSP_GYRO_FIFO_Set_Watermark_Level_Ext_IKS01A2 BSP_GYRO_FIFO_Set_Watermark_Level_Ext
#define BSP_GYRO_FIFO_Set_Stop_On_Fth_Ext_IKS01A2 BSP_GYRO_FIFO_Set_Stop_On_Fth_Ext

#define BSP_GYRO_Set_Interrupt_Latch_Ext_IKS01A2 BSP_GYRO_Set_Interrupt_Latch_Ext
#define BSP_GYRO_Set_SelfTest_Ext_IKS01A2 BSP_GYRO_Set_SelfTest_Ext

/* Accelero */
#define BSP_ACCELERO_Init_IKS01A2 BSP_ACCELERO_Init
#define BSP_ACCELERO_DeInit_IKS01A2 BSP_ACCELERO_DeInit
#define BSP_ACCELERO_Sensor_Enable_IKS01A2 BSP_ACCELERO_Sensor_Enable
#define BSP_ACCELERO_Sensor_Disable_IKS01A2 BSP_ACCELERO_Sensor_Disable
#define BSP_ACCELERO_IsInitialized_IKS01A2 BSP_ACCELERO_IsInitialized
#define BSP_ACCELERO_IsEnabled_IKS01A2 BSP_ACCELERO_IsEnabled
#define BSP_ACCELERO_IsCombo_IKS01A2 BSP_ACCELERO_IsCombo
#define BSP_ACCELERO_Get_Instance_IKS01A2 BSP_ACCELERO_Get_Instance
#define BSP_ACCELERO_Get_WhoAmI_IKS01A2 BSP_ACCELERO_Get_WhoAmI
#define BSP_ACCELERO_Check_WhoAmI_IKS01A2 BSP_ACCELERO_Check_WhoAmI
#define BSP_ACCELERO_Get_Axes_IKS01A2 BSP_ACCELERO_Get_Axes
#define BSP_ACCELERO_Get_AxesRaw_IKS01A2 BSP_ACCELERO_Get_AxesRaw
#define BSP_ACCELERO_Get_Sensitivity_IKS01A2 BSP_ACCELERO_Get_Sensitivity
#define BSP_ACCELERO_Get_ODR_IKS01A2 BSP_ACCELERO_Get_ODR
#define BSP_ACCELERO_Set_ODR_IKS01A2 BSP_ACCELERO_Set_ODR
#define BSP_ACCELERO_Set_ODR_Value_IKS01A2 BSP_ACCELERO_Set_ODR_Value
#define BSP_ACCELERO_Get_FS_IKS01A2 BSP_ACCELERO_Get_FS
#define BSP_ACCELERO_Set_FS_IKS01A2 BSP_ACCELERO_Set_FS
#define BSP_ACCELERO_Set_FS_Value_IKS01A2 BSP_ACCELERO_Set_FS_Value
#define BSP_ACCELERO_Get_Axes_Status_IKS01A2 BSP_ACCELERO_Get_Axes_Status
#define BSP_ACCELERO_Set_Axes_Status_IKS01A2 BSP_ACCELERO_Set_Axes_Status
#define BSP_ACCELERO_Read_Reg_IKS01A2 BSP_ACCELERO_Read_Reg
#define BSP_ACCELERO_Write_Reg_IKS01A2 BSP_ACCELERO_Write_Reg
#define BSP_ACCELERO_Get_DRDY_Status_IKS01A2 BSP_ACCELERO_Get_DRDY_Status

#define BSP_ACCELERO_Enable_Free_Fall_Detection_Ext_IKS01A2 BSP_ACCELERO_Enable_Free_Fall_Detection_Ext
#define BSP_ACCELERO_Disable_Free_Fall_Detection_Ext_IKS01A2 BSP_ACCELERO_Disable_Free_Fall_Detection_Ext
#define BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext_IKS01A2 BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext
#define BSP_ACCELERO_Set_Free_Fall_Threshold_Ext_IKS01A2 BSP_ACCELERO_Set_Free_Fall_Threshold_Ext

#define BSP_ACCELERO_Enable_Pedometer_Ext_IKS01A2 BSP_ACCELERO_Enable_Pedometer_Ext
#define BSP_ACCELERO_Disable_Pedometer_Ext_IKS01A2 BSP_ACCELERO_Disable_Pedometer_Ext
#define BSP_ACCELERO_Get_Pedometer_Status_Ext_IKS01A2 BSP_ACCELERO_Get_Pedometer_Status_Ext
#define BSP_ACCELERO_Get_Step_Count_Ext_IKS01A2 BSP_ACCELERO_Get_Step_Count_Ext
#define BSP_ACCELERO_Reset_Step_Counter_Ext_IKS01A2 BSP_ACCELERO_Reset_Step_Counter_Ext
#define BSP_ACCELERO_Set_Pedometer_Threshold_Ext_IKS01A2 BSP_ACCELERO_Set_Pedometer_Threshold_Ext

#define BSP_ACCELERO_Enable_Tilt_Detection_Ext_IKS01A2 BSP_ACCELERO_Enable_Tilt_Detection_Ext
#define BSP_ACCELERO_Disable_Tilt_Detection_Ext_IKS01A2 BSP_ACCELERO_Disable_Tilt_Detection_Ext
#define BSP_ACCELERO_Get_Tilt_Detection_Status_Ext_IKS01A2 BSP_ACCELERO_Get_Tilt_Detection_Status_Ext

#define BSP_ACCELERO_Enable_Wake_Up_Detection_Ext_IKS01A2 BSP_ACCELERO_Enable_Wake_Up_Detection_Ext
#define BSP_ACCELERO_Disable_Wake_Up_Detection_Ext_IKS01A2 BSP_ACCELERO_Disable_Wake_Up_Detection_Ext
#define BSP_ACCELERO_Get_Wake_Up_Detection_Status_Ext_IKS01A2 BSP_ACCELERO_Get_Wake_Up_Detection_Status_Ext
#define BSP_ACCELERO_Set_Wake_Up_Threshold_Ext_IKS01A2 BSP_ACCELERO_Set_Wake_Up_Threshold_Ext

#define BSP_ACCELERO_Enable_Single_Tap_Detection_Ext_IKS01A2 BSP_ACCELERO_Enable_Single_Tap_Detection_Ext
#define BSP_ACCELERO_Disable_Single_Tap_Detection_Ext_IKS01A2 BSP_ACCELERO_Disable_Single_Tap_Detection_Ext
#define BSP_ACCELERO_Get_Single_Tap_Detection_Status_Ext_IKS01A2 BSP_ACCELERO_Get_Single_Tap_Detection_Status_Ext
#define BSP_ACCELERO_Enable_Double_Tap_Detection_Ext_IKS01A2 BSP_ACCELERO_Enable_Double_Tap_Detection_Ext
#define BSP_ACCELERO_Disable_Double_Tap_Detection_Ext_IKS01A2 BSP_ACCELERO_Disable_Double_Tap_Detection_Ext
#define BSP_ACCELERO_Get_Double_Tap_Detection_Status_Ext_IKS01A2 BSP_ACCELERO_Get_Double_Tap_Detection_Status_Ext
#define BSP_ACCELERO_Set_Tap_Threshold_Ext_IKS01A2 BSP_ACCELERO_Set_Tap_Threshold_Ext
#define BSP_ACCELERO_Set_Tap_Shock_Time_Ext_IKS01A2 BSP_ACCELERO_Set_Tap_Shock_Time_Ext
#define BSP_ACCELERO_Set_Tap_Quiet_Time_Ext_IKS01A2 BSP_ACCELERO_Set_Tap_Quiet_Time_Ext
#define BSP_ACCELERO_Set_Tap_Duration_Time_Ext_IKS01A2 BSP_ACCELERO_Set_Tap_Duration_Time_Ext

#define BSP_ACCELERO_Enable_6D_Orientation_Ext_IKS01A2 BSP_ACCELERO_Enable_6D_Orientation_Ext
#define BSP_ACCELERO_Disable_6D_Orientation_Ext_IKS01A2 BSP_ACCELERO_Disable_6D_Orientation_Ext
#define BSP_ACCELERO_Get_6D_Orientation_Status_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_Status_Ext
#define BSP_ACCELERO_Get_6D_Orientation_XL_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_XL_Ext
#define BSP_ACCELERO_Get_6D_Orientation_XH_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_XH_Ext
#define BSP_ACCELERO_Get_6D_Orientation_YL_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_YL_Ext
#define BSP_ACCELERO_Get_6D_Orientation_YH_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_YH_Ext
#define BSP_ACCELERO_Get_6D_Orientation_ZL_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_ZL_Ext
#define BSP_ACCELERO_Get_6D_Orientation_ZH_Ext_IKS01A2 BSP_ACCELERO_Get_6D_Orientation_ZH_Ext
#define BSP_ACCELERO_FIFO_Set_ODR_Value_Ext_IKS01A2 BSP_ACCELERO_FIFO_Set_ODR_Value_Ext
#define BSP_ACCELERO_FIFO_Get_Full_Status_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Full_Status_Ext
#define BSP_ACCELERO_FIFO_Get_Empty_Status_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Empty_Status_Ext
#define BSP_ACCELERO_FIFO_Get_Overrun_Status_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Overrun_Status_Ext
#define BSP_ACCELERO_FIFO_Get_Pattern_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Pattern_Ext
#define BSP_ACCELERO_FIFO_Get_Data_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Data_Ext
#define BSP_ACCELERO_FIFO_Get_Num_Of_Samples_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Num_Of_Samples_Ext
#define BSP_ACCELERO_FIFO_Set_Decimation_Ext_IKS01A2 BSP_ACCELERO_FIFO_Set_Decimation_Ext
#define BSP_ACCELERO_FIFO_Get_Axis_Ext_IKS01A2 BSP_ACCELERO_FIFO_Get_Axis_Ext
#define BSP_ACCELERO_FIFO_Set_Mode_Ext_IKS01A2 BSP_ACCELERO_FIFO_Set_Mode_Ext
#define BSP_ACCELERO_FIFO_Set_INT1_FIFO_Full_Ext_IKS01A2 BSP_ACCELERO_FIFO_Set_INT1_FIFO_Full_Ext
#define BSP_ACCELERO_FIFO_Set_Watermark_Level_Ext_IKS01A2 BSP_ACCELERO_FIFO_Set_Watermark_Level_Ext
#define BSP_ACCELERO_FIFO_Set_Stop_On_Fth_Ext_IKS01A2 BSP_ACCELERO_FIFO_Set_Stop_On_Fth_Ext
#define BSP_ACCELERO_Set_Interrupt_Latch_Ext_IKS01A2 BSP_ACCELERO_Set_Interrupt_Latch_Ext
#define BSP_ACCELERO_Set_SelfTest_Ext_IKS01A2 BSP_ACCELERO_Set_SelfTest_Ext

/* Temperature */
#define BSP_TEMPERATURE_Init_IKS01A2 BSP_TEMPERATURE_Init
#define BSP_TEMPERATURE_DeInit_IKS01A2 BSP_TEMPERATURE_DeInit
#define BSP_TEMPERATURE_Sensor_Enable_IKS01A2 BSP_TEMPERATURE_Sensor_Enable
#define BSP_TEMPERATURE_Sensor_Disable_IKS01A2 BSP_TEMPERATURE_Sensor_Disable
#define BSP_TEMPERATURE_IsInitialized_IKS01A2 BSP_TEMPERATURE_IsInitialized
#define BSP_TEMPERATURE_IsEnabled_IKS01A2 BSP_TEMPERATURE_IsEnabled
#define BSP_TEMPERATURE_IsCombo_IKS01A2 BSP_TEMPERATURE_IsCombo
#define BSP_TEMPERATURE_Get_Instance_IKS01A2 BSP_TEMPERATURE_Get_Instance
#define BSP_TEMPERATURE_Get_WhoAmI_IKS01A2 BSP_TEMPERATURE_Get_WhoAmI
#define BSP_TEMPERATURE_Check_WhoAmI_IKS01A2 BSP_TEMPERATURE_Check_WhoAmI
#define BSP_TEMPERATURE_Get_Temp_IKS01A2 BSP_TEMPERATURE_Get_Temp
#define BSP_TEMPERATURE_Get_ODR_IKS01A2 BSP_TEMPERATURE_Get_ODR
#define BSP_TEMPERATURE_Set_ODR_IKS01A2 BSP_TEMPERATURE_Set_ODR
#define BSP_TEMPERATURE_Set_ODR_Value_IKS01A2 BSP_TEMPERATURE_Set_ODR_Value
#define BSP_TEMPERATURE_Read_Reg_IKS01A2 BSP_TEMPERATURE_Read_Reg
#define BSP_TEMPERATURE_Write_Reg_IKS01A2 BSP_TEMPERATURE_Write_Reg
#define BSP_TEMPERATURE_Get_DRDY_Status_IKS01A2 BSP_TEMPERATURE_Get_DRDY_Status

#define BSP_TEMPERATURE_FIFO_Get_Full_Status_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Get_Full_Status_Ext
#define BSP_TEMPERATURE_FIFO_Get_Fth_Status_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Get_Fth_Status_Ext
#define BSP_TEMPERATURE_FIFO_Get_Ovr_Status_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Get_Ovr_Status_Ext
#define BSP_TEMPERATURE_FIFO_Get_Data_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Get_Data_Ext
#define BSP_TEMPERATURE_FIFO_Get_Num_Of_Samples_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Get_Num_Of_Samples_Ext
#define BSP_TEMPERATURE_FIFO_Set_Mode_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Set_Mode_Ext
#define BSP_TEMPERATURE_FIFO_Set_Interrupt_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Set_Interrupt_Ext
#define BSP_TEMPERATURE_FIFO_Reset_Interrupt_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Reset_Interrupt_Ext
#define BSP_TEMPERATURE_FIFO_Set_Watermark_Level_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Set_Watermark_Level_Ext
#define BSP_TEMPERATURE_FIFO_Stop_On_Fth_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Stop_On_Fth_Ext
#define BSP_TEMPERATURE_FIFO_Usage_Ext_IKS01A2 BSP_TEMPERATURE_FIFO_Usage_Ext

/* Pressure */
#define BSP_PRESSURE_Init_IKS01A2 BSP_PRESSURE_Init
#define BSP_PRESSURE_DeInit_IKS01A2 BSP_PRESSURE_DeInit
#define BSP_PRESSURE_Sensor_Enable_IKS01A2 BSP_PRESSURE_Sensor_Enable
#define BSP_PRESSURE_Sensor_Disable_IKS01A2 BSP_PRESSURE_Sensor_Disable
#define BSP_PRESSURE_IsInitialized_IKS01A2 BSP_PRESSURE_IsInitialized
#define BSP_PRESSURE_IsEnabled_IKS01A2 BSP_PRESSURE_IsEnabled
#define BSP_PRESSURE_IsCombo_IKS01A2 BSP_PRESSURE_IsCombo
#define BSP_PRESSURE_Get_Instance_IKS01A2 BSP_PRESSURE_Get_Instance
#define BSP_PRESSURE_Get_WhoAmI_IKS01A2 BSP_PRESSURE_Get_WhoAmI
#define BSP_PRESSURE_Check_WhoAmI_IKS01A2 BSP_PRESSURE_Check_WhoAmI
#define BSP_PRESSURE_Get_Press_IKS01A2 BSP_PRESSURE_Get_Press
#define BSP_PRESSURE_Get_ODR_IKS01A2 BSP_PRESSURE_Get_ODR
#define BSP_PRESSURE_Set_ODR_IKS01A2 BSP_PRESSURE_Set_ODR
#define BSP_PRESSURE_Set_ODR_Value_IKS01A2 BSP_PRESSURE_Set_ODR_Value
#define BSP_PRESSURE_Read_Reg_IKS01A2 BSP_PRESSURE_Read_Reg
#define BSP_PRESSURE_Write_Reg_IKS01A2 BSP_PRESSURE_Write_Reg
#define BSP_PRESSURE_Get_DRDY_Status_IKS01A2 BSP_PRESSURE_Get_DRDY_Status

#define BSP_PRESSURE_FIFO_Get_Full_Status_Ext_IKS01A2 BSP_PRESSURE_FIFO_Get_Full_Status_Ext
#define BSP_PRESSURE_FIFO_Get_Fth_Status_Ext_IKS01A2 BSP_PRESSURE_FIFO_Get_Fth_Status_Ext
#define BSP_PRESSURE_FIFO_Get_Ovr_Status_Ext_IKS01A2 BSP_PRESSURE_FIFO_Get_Ovr_Status_Ext
#define BSP_PRESSURE_FIFO_Get_Data_Ext_IKS01A2 BSP_PRESSURE_FIFO_Get_Data_Ext
#define BSP_PRESSURE_FIFO_Get_Num_Of_Samples_Ext_IKS01A2 BSP_PRESSURE_FIFO_Get_Num_Of_Samples_Ext
#define BSP_PRESSURE_FIFO_Set_Mode_Ext_IKS01A2 BSP_PRESSURE_FIFO_Set_Mode_Ext
#define BSP_PRESSURE_FIFO_Set_Interrupt_Ext_IKS01A2 BSP_PRESSURE_FIFO_Set_Interrupt_Ext
#define BSP_PRESSURE_FIFO_Reset_Interrupt_Ext_IKS01A2 BSP_PRESSURE_FIFO_Reset_Interrupt_Ext
#define BSP_PRESSURE_FIFO_Set_Watermark_Level_Ext_IKS01A2 BSP_PRESSURE_FIFO_Set_Watermark_Level_Ext
#define BSP_PRESSURE_FIFO_Stop_On_Fth_Ext_IKS01A2 BSP_PRESSURE_FIFO_Stop_On_Fth_Ext
#define BSP_PRESSURE_FIFO_Usage_Ext_IKS01A2 BSP_PRESSURE_FIFO_Usage_Ext

/* Magneto */
#define BSP_MAGNETO_Init_IKS01A2 BSP_MAGNETO_Init
#define BSP_MAGNETO_DeInit_IKS01A2 BSP_MAGNETO_DeInit
#define BSP_MAGNETO_Sensor_Enable_IKS01A2 BSP_MAGNETO_Sensor_Enable
#define BSP_MAGNETO_Sensor_Disable_IKS01A2 BSP_MAGNETO_Sensor_Disable
#define BSP_MAGNETO_IsInitialized_IKS01A2 BSP_MAGNETO_IsInitialized
#define BSP_MAGNETO_IsEnabled_IKS01A2 BSP_MAGNETO_IsEnabled
#define BSP_MAGNETO_IsCombo_IKS01A2 BSP_MAGNETO_IsCombo
#define BSP_MAGNETO_Get_Instance_IKS01A2 BSP_MAGNETO_Get_Instance
#define BSP_MAGNETO_Get_WhoAmI_IKS01A2 BSP_MAGNETO_Get_WhoAmI
#define BSP_MAGNETO_Check_WhoAmI_IKS01A2 BSP_MAGNETO_Check_WhoAmI
#define BSP_MAGNETO_Get_Axes_IKS01A2 BSP_MAGNETO_Get_Axes
#define BSP_MAGNETO_Get_AxesRaw_IKS01A2 BSP_MAGNETO_Get_AxesRaw
#define BSP_MAGNETO_Get_Sensitivity_IKS01A2 BSP_MAGNETO_Get_Sensitivity
#define BSP_MAGNETO_Get_ODR_IKS01A2 BSP_MAGNETO_Get_ODR
#define BSP_MAGNETO_Set_ODR_IKS01A2 BSP_MAGNETO_Set_ODR
#define BSP_MAGNETO_Set_ODR_Value_IKS01A2 BSP_MAGNETO_Set_ODR_Value
#define BSP_MAGNETO_Get_FS_IKS01A2 BSP_MAGNETO_Get_FS
#define BSP_MAGNETO_Set_FS_IKS01A2 BSP_MAGNETO_Set_FS
#define BSP_MAGNETO_Set_FS_Value_IKS01A2 BSP_MAGNETO_Set_FS_Value
#define BSP_MAGNETO_Read_Reg_IKS01A2 BSP_MAGNETO_Read_Reg
#define BSP_MAGNETO_Write_Reg_IKS01A2 BSP_MAGNETO_Write_Reg
#define BSP_MAGNETO_Get_DRDY_Status_IKS01A2 BSP_MAGNETO_Get_DRDY_Status

#endif /* __SENSOR_MAP_FUNC_H */

/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
