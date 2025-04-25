################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Utility/error_messages.c \
../Source/Utility/ring_buffer.c 

OBJS += \
./Source/Utility/error_messages.o \
./Source/Utility/ring_buffer.o 

C_DEPS += \
./Source/Utility/error_messages.d \
./Source/Utility/ring_buffer.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Utility/%.o Source/Utility/%.su Source/Utility/%.cyclo: ../Source/Utility/%.c Source/Utility/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32F411xE -DUSE_HAL_DRIVER -c -I../Source/ThirdParty/Core/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc -I../Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/include -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Source/ThirdParty/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Source/ThirdParty/Drivers/CMSIS/Include -I../Source/API -I../Source/APP -I../Source/Driver -I../Source/Utility -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-Utility

clean-Source-2f-Utility:
	-$(RM) ./Source/Utility/error_messages.cyclo ./Source/Utility/error_messages.d ./Source/Utility/error_messages.o ./Source/Utility/error_messages.su ./Source/Utility/ring_buffer.cyclo ./Source/Utility/ring_buffer.d ./Source/Utility/ring_buffer.o ./Source/Utility/ring_buffer.su

.PHONY: clean-Source-2f-Utility

