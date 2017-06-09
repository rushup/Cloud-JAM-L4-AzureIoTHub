@echo off
set STLINK_PATH="C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\"
set NAMEMOTENV=Release\Exe\Azure2_NL4
set BOOTLOADER="..\..\..\..\..\..\Utilities\BootLoader\STM32L476RG\BootLoaderL4.bin"
color 0F
echo                /******************************************/
echo                           Clean AZURE
echo                /******************************************/
echo                              Full Chip Erase
echo                /******************************************/
%STLINK_PATH%ST-LINK_CLI.exe -ME
echo                /******************************************/
echo                              Install BootLoader
echo                /******************************************/
%STLINK_PATH%ST-LINK_CLI.exe -P %BOOTLOADER% 0x08000000 -V "after_programming"
echo                /******************************************/
echo                          Install AZURE
echo                /******************************************/
%STLINK_PATH%ST-LINK_CLI.exe -P %NAMEMOTENV%.bin 0x08004000 -V "after_programming"
echo                /******************************************/
echo                    Dump AZURE+ BootLoader
echo                /******************************************/
set offset_size=0x4000
for %%I in (%NAMEMOTENV%.bin) do set application_size=%%~zI
echo %NAMEMOTENV%.bin size is %application_size% bytes
set /a size=%offset_size%+%application_size%
echo Dumping %offset_size% + %application_size% = %size% bytes ...
echo ..........................
%STLINK_PATH%ST-LINK_CLI.exe -Dump 0x08000000 %size% %NAMEMOTENV%_BL.bin
echo                /******************************************/
echo                                 Reset STM32
echo                /******************************************/
%STLINK_PATH%ST-LINK_CLI.exe -Rst
if NOT "%1" == "SILENT" pause