################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Gui/Core/oscilloscope/ChannelDLG.c \
../Gui/Core/oscilloscope/CursorDLG.c \
../Gui/Core/oscilloscope/TimeDLG.c \
../Gui/Core/oscilloscope/TriggerDLG.c \
../Gui/Core/oscilloscope/osci_win.c 

OBJS += \
./Gui/Core/oscilloscope/ChannelDLG.o \
./Gui/Core/oscilloscope/CursorDLG.o \
./Gui/Core/oscilloscope/TimeDLG.o \
./Gui/Core/oscilloscope/TriggerDLG.o \
./Gui/Core/oscilloscope/osci_win.o 

C_DEPS += \
./Gui/Core/oscilloscope/ChannelDLG.d \
./Gui/Core/oscilloscope/CursorDLG.d \
./Gui/Core/oscilloscope/TimeDLG.d \
./Gui/Core/oscilloscope/TriggerDLG.d \
./Gui/Core/oscilloscope/osci_win.d 


# Each subdirectory must supply rules for building sources it contributes
Gui/Core/oscilloscope/%.o: ../Gui/Core/oscilloscope/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_STDOUT -DSTM32F746xx -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER -DHSE_VALUE=25000000 -DUSE_STM32746G_DISCOVERY -DARM_MATH_CM7 -DUSE_SD_CARD_DISK -DUSE_USBH_MSD_DISK -I../Gui/STemWin_Addons -I../Gui/Target -I../Config -I../Core/Inc -I../Utilities/CPU -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/BSP/STM32746G-Discovery -I../Drivers/BSP/Components -I../Drivers/BSP/Components/Common -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../Middlewares/Third_Party/LwIP/src/include/ -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/LwIP/system/OS -I../Middlewares/ST/STemWin/inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/Third_Party/FatFs/src/drivers -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Modules/Common -I../Modules/audio_player -I../Modules/audio_recorder -I../Modules/videoplayer -I../Modules/vnc_server -I../Drivers/CMSIS/Include -I../mpu9250 -I../ub_lib -I../uboszi_lib -includestdint.h -includestdbool.h -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


