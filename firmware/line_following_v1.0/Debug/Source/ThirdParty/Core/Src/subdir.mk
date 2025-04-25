################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/ThirdParty/Core/Src/freertos.c \
../Source/ThirdParty/Core/Src/gpio.c \
../Source/ThirdParty/Core/Src/i2c.c \
../Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.c \
../Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.c \
../Source/ThirdParty/Core/Src/stm32f4xx_it.c \
../Source/ThirdParty/Core/Src/system_stm32f4xx.c \
../Source/ThirdParty/Core/Src/tim.c \
../Source/ThirdParty/Core/Src/usart.c 

OBJS += \
./Source/ThirdParty/Core/Src/freertos.o \
./Source/ThirdParty/Core/Src/gpio.o \
./Source/ThirdParty/Core/Src/i2c.o \
./Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.o \
./Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.o \
./Source/ThirdParty/Core/Src/stm32f4xx_it.o \
./Source/ThirdParty/Core/Src/system_stm32f4xx.o \
./Source/ThirdParty/Core/Src/tim.o \
./Source/ThirdParty/Core/Src/usart.o 

C_DEPS += \
./Source/ThirdParty/Core/Src/freertos.d \
./Source/ThirdParty/Core/Src/gpio.d \
./Source/ThirdParty/Core/Src/i2c.d \
./Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.d \
./Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.d \
./Source/ThirdParty/Core/Src/stm32f4xx_it.d \
./Source/ThirdParty/Core/Src/system_stm32f4xx.d \
./Source/ThirdParty/Core/Src/tim.d \
./Source/ThirdParty/Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Source/ThirdParty/Core/Src/%.o Source/ThirdParty/Core/Src/%.su Source/ThirdParty/Core/Src/%.cyclo: ../Source/ThirdParty/Core/Src/%.c Source/ThirdParty/Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32F411xE -DUSE_HAL_DRIVER -c -I../Source/ThirdParty/Core/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/include -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Source/ThirdParty/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Source/ThirdParty/Drivers/CMSIS/Include -I../Source/API -I../Source/APP -I../Source/Driver -I../Source/Utility -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-ThirdParty-2f-Core-2f-Src

clean-Source-2f-ThirdParty-2f-Core-2f-Src:
	-$(RM) ./Source/ThirdParty/Core/Src/freertos.cyclo ./Source/ThirdParty/Core/Src/freertos.d ./Source/ThirdParty/Core/Src/freertos.o ./Source/ThirdParty/Core/Src/freertos.su ./Source/ThirdParty/Core/Src/gpio.cyclo ./Source/ThirdParty/Core/Src/gpio.d ./Source/ThirdParty/Core/Src/gpio.o ./Source/ThirdParty/Core/Src/gpio.su ./Source/ThirdParty/Core/Src/i2c.cyclo ./Source/ThirdParty/Core/Src/i2c.d ./Source/ThirdParty/Core/Src/i2c.o ./Source/ThirdParty/Core/Src/i2c.su ./Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.cyclo ./Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.d ./Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.o ./Source/ThirdParty/Core/Src/stm32f4xx_hal_msp.su ./Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.cyclo ./Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.d ./Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.o ./Source/ThirdParty/Core/Src/stm32f4xx_hal_timebase_tim.su ./Source/ThirdParty/Core/Src/stm32f4xx_it.cyclo ./Source/ThirdParty/Core/Src/stm32f4xx_it.d ./Source/ThirdParty/Core/Src/stm32f4xx_it.o ./Source/ThirdParty/Core/Src/stm32f4xx_it.su ./Source/ThirdParty/Core/Src/system_stm32f4xx.cyclo ./Source/ThirdParty/Core/Src/system_stm32f4xx.d ./Source/ThirdParty/Core/Src/system_stm32f4xx.o ./Source/ThirdParty/Core/Src/system_stm32f4xx.su ./Source/ThirdParty/Core/Src/tim.cyclo ./Source/ThirdParty/Core/Src/tim.d ./Source/ThirdParty/Core/Src/tim.o ./Source/ThirdParty/Core/Src/tim.su ./Source/ThirdParty/Core/Src/usart.cyclo ./Source/ThirdParty/Core/Src/usart.d ./Source/ThirdParty/Core/Src/usart.o ./Source/ThirdParty/Core/Src/usart.su

.PHONY: clean-Source-2f-ThirdParty-2f-Core-2f-Src

