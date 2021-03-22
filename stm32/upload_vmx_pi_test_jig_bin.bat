arm-none-eabi-objcopy -S  -O binary  "vmx_pi_test_jig.elf" "vmx_pi_test_jig.bin" 
arm-none-eabi-objcopy -S  -O srec  "vmx_pi_test_jig.elf" "vmx_pi_test_jig.srec" 
arm-none-eabi-objcopy -S  -O ihex  "vmx_pi_test_jig.elf" "vmx_pi_test_jig.hex" 
TASKKILL /F /IM openocd.exe
set flasher="%ProgramFiles(x86)%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"
set binary=vmx_pi_test_jig.srec
REM Download firmware, and protect ALL flash pages except the second (which is used for parameter storage)
%flasher% -c SWD SWCLK=9 -OB WRP=0x00000002 -P %binary% -V
