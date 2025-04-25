################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Driver/exti_driver.c \
../Source/Driver/gpio_driver.c \
../Source/Driver/timer_driver.c \
../Source/Driver/uart_driver.c 

OBJS += \
./Source/Driver/exti_driver.o \
./Source/Driver/gpio_driver.o \
./Source/Driver/timer_driver.o \
./Source/Driver/uart_driver.o 

C_DEPS += \
./Source/Driver/exti_driver.d \
./Source/Driver/gpio_driver.d \
./Source/Driver/timer_driver.d \
./Source/Driver/uart_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Driver/%.o Source/Driver/%.su Source/Driver/%.cyclo: ../Source/Driver/%.c Source/Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32F411xE -DUSE_HAL_DRIVER -c -I../Source/ThirdParty/Core/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/include -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Source/ThirdParty/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Source/ThirdParty/Drivers/CMSIS/Include -I../Source/API -I../Source/APP -I../Source/Driver -I../Source/Utility -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-Driver

clean-Source-2f-Driver:
	-$(RM) ./Source/Driver/exti_driver.cyclo ./Source/Driver/exti_driver.d ./Source/Driver/exti_driver.o ./Source/Driver/exti_driver.su ./Source/Driver/gpio_driver.cyclo ./Source/Driver/gpio_driver.d ./Source/Driver/gpio_driver.o ./Source/Driver/gpio_driver.su ./Source/Driver/timer_driver.cyclo ./Source/Driver/timer_driver.d ./Source/Driver/timer_driver.o ./Source/Driver/timer_driver.su ./Source/Driver/uart_driver.cyclo ./Source/Driver/uart_driver.d ./Source/Driver/uart_driver.o ./Source/Driver/uart_driver.su

.PHONY: clean-Source-2f-Driver

