Boot Loader for STM32F4/STM32L4
=======================================

This is a project of a simple Boot Loader that could be flashed on Nucleo (**STM32F401RE/STM32L476RG**)

It makes a check for understanding if there is one **Firmware update** available on Flash:

- If there is the Update, the Boot Loader **replaces** the current program with the Update

- If there is not the Update, the Boot Loader **jumps** to the program and executes it
