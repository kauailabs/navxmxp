arm-none-eabi-objcopy -S  -O binary  "navx-micro.elf" "navx-micro.bin" 
arm-none-eabi-objcopy -S  -O srec  "navx-micro.elf" "navx-micro.srec" 
arm-none-eabi-objcopy -S  -O ihex  "navx-micro.elf" "navx-micro.hex" 
TASKKILL /F /IM openocd.exe
set flasher="%ProgramFiles(x86)%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"
set binary=navx-micro.srec
REM Download firmware, and protect ALL flash pages except the second (which is used for parameter storage)
%flasher% -c SWD -OB WRP=0x00000002 -P %binary% -V
