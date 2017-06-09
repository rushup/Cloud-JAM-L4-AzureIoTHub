#!/bin/bash


######## Modify this Section:
# 1) Set the Installation path for OpenOCD
# example:
OpenOCD_DIR="/Applications/Ac6/SystemWorkbench.app//Contents/Eclipse/plugins/fr.ac6.mcu.externaltools.openocd.macos64_1.11.0.201610101357/tools/openocd/"
#OpenOCD_DIR=""

# 2) Set the installation path for stm32 OpenOCD scritps
# example:
OpenOCD_CFC="/Applications/Ac6/SystemWorkbench.app//Contents/Eclipse/plugins/fr.ac6.mcu.debug_1.11.0.201610101240/resources/openocd/scripts"
#OpenOCD_CFC=""

# 3) Add openocd library path to _LIBRARY_PATH:
export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${OpenOCD_DIR}"lib/"


######## Don't change the following part

## Control Section

if [[ ! $OpenOCD_DIR ]];  then
	echo "Please add the rigth path to OpenOCD_DIR Variable"
	exit
fi

if [[ ! $OpenOCD_CFC ]];  then
	echo "Please add the rigth path to OpenOCD_CFC Variable"
	exit
fi


## Run section

# Board type
BOARDNAME="nucleo_l476rg"

# OpenOCD command
OpenOCD_CMD="${OpenOCD_DIR}/bin/openocd -s ${OpenOCD_CFC} -f st_board/${BOARDNAME}.cfg"


echo "/******************************************/"
echo "           Clean Azure1"
echo "/******************************************/"
echo "           Erase License Manager"
echo "/******************************************/"
${OpenOCD_CMD} -c "init" -c "reset halt" -c "flash erase_sector 0 510 511" -c "shutdown"
echo "/******************************************/"
echo "              Install BootLoader"
echo "/******************************************/"
${OpenOCD_CMD} -c "program ../../../../../../Utilities/BootLoader/STM32L476RG/BootLoaderL4.bin 0x08000000 exit" -c "shutdown"
echo "/******************************************/"
echo "          Install Azure1"
echo "/******************************************/"
${OpenOCD_CMD} -c "program ./STM32L4xx-Nucleo/mbedTLS/Azure_Sns_DM.bin 0x08004000 exit" -c "shutdown"
echo "/******************************************/"
echo "    Dump Azure1+ BootLoader"
echo "/******************************************/"

# 16384==0x400
SizeBinBL=`ls -l ./STM32L4xx-Nucleo/mbedTLS/Azure_Sns_DM.bin | awk '{print $5+16384};'`
${OpenOCD_CMD} -c "init" \
			   -c "reset halt" \
			   -c "sleep 100" \
			   -c "wait_halt 2" \
			   -c "dump_image ./STM32L4xx-Nucleo/mbedTLS/Azure_Sns_DM_BL.bin 0x08000000 ${SizeBinBL}" \
			   -c "reset halt" \
			   -c "shutdown"

