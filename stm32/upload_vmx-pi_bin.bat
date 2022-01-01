arm-none-eabi-objcopy -S  -O binary  "vmx-pi.elf" "vmx-pi.bin" 
arm-none-eabi-objcopy -S  -O srec  "vmx-pi.elf" "vmx-pi.srec" 
arm-none-eabi-objcopy -S  -O ihex  "vmx-pi.elf" "vmx-pi.hex" 
TASKKILL /F /IM openocd.exe
set flasher="%ProgramFiles(x86)%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"
set binary=vmx-pi.srec
REM Download firmware, and protect ALL flash pages except the second (which is used for parameter storage)
%flasher% -c SWD SWCLK=9 -OB WRP=0xFFFFFFFF -P %binary% -V
