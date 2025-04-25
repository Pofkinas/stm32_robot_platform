################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/API/button_api.c \
../Source/API/cmd_api.c \
../Source/API/cmd_api_helper.c \
../Source/API/debug_api.c \
../Source/API/heap_api.c \
../Source/API/led_api.c \
../Source/API/motor_api.c \
../Source/API/tcrt5000_api.c \
../Source/API/uart_api.c 

OBJS += \
./Source/API/button_api.o \
./Source/API/cmd_api.o \
./Source/API/cmd_api_helper.o \
./Source/API/debug_api.o \
./Source/API/heap_api.o \
./Source/API/led_api.o \
./Source/API/motor_api.o \
./Source/API/tcrt5000_api.o \
./Source/API/uart_api.o 

C_DEPS += \
./Source/API/button_api.d \
./Source/API/cmd_api.d \
./Source/API/cmd_api_helper.d \
./Source/API/debug_api.d \
./Source/API/heap_api.d \
./Source/API/led_api.d \
./Source/API/motor_api.d \
./Source/API/tcrt5000_api.d \
./Source/API/uart_api.d 


# Each subdirectory must supply rules for building sources it contributes
Source/API/%.o Source/API/%.su Source/API/%.cyclo: ../Source/API/%.c Source/API/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32F411xE -DUSE_HAL_DRIVER -c -I../Source/ThirdParty/Core/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/include -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Source/ThirdParty/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Source/ThirdParty/Drivers/CMSIS/Include -I../Source/API -I../Source/APP -I../Source/Driver -I../Source/Utility -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-API

clean-Source-2f-API:
	-$(RM) ./Source/API/button_api.cyclo ./Source/API/button_api.d ./Source/API/button_api.o ./Source/API/button_api.su ./Source/API/cmd_api.cyclo ./Source/API/cmd_api.d ./Source/API/cmd_api.o ./Source/API/cmd_api.su ./Source/API/cmd_api_helper.cyclo ./Source/API/cmd_api_helper.d ./Source/API/cmd_api_helper.o ./Source/API/cmd_api_helper.su ./Source/API/debug_api.cyclo ./Source/API/debug_api.d ./Source/API/debug_api.o ./Source/API/debug_api.su ./Source/API/heap_api.cyclo ./Source/API/heap_api.d ./Source/API/heap_api.o ./Source/API/heap_api.su ./Source/API/led_api.cyclo ./Source/API/led_api.d ./Source/API/led_api.o ./Source/API/led_api.su ./Source/API/motor_api.cyclo ./Source/API/motor_api.d ./Source/API/motor_api.o ./Source/API/motor_api.su ./Source/API/tcrt5000_api.cyclo ./Source/API/tcrt5000_api.d ./Source/API/tcrt5000_api.o ./Source/API/tcrt5000_api.su ./Source/API/uart_api.cyclo ./Source/API/uart_api.d ./Source/API/uart_api.o ./Source/API/uart_api.su

.PHONY: clean-Source-2f-API

