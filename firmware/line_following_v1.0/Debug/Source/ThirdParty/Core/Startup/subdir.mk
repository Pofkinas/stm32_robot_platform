################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Source/ThirdParty/Core/Startup/startup_stm32f411retx.s 

OBJS += \
./Source/ThirdParty/Core/Startup/startup_stm32f411retx.o 

S_DEPS += \
./Source/ThirdParty/Core/Startup/startup_stm32f411retx.d 


# Each subdirectory must supply rules for building sources it contributes
Source/ThirdParty/Core/Startup/%.o: ../Source/ThirdParty/Core/Startup/%.s Source/ThirdParty/Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Source-2f-ThirdParty-2f-Core-2f-Startup

clean-Source-2f-ThirdParty-2f-Core-2f-Startup:
	-$(RM) ./Source/ThirdParty/Core/Startup/startup_stm32f411retx.d ./Source/ThirdParty/Core/Startup/startup_stm32f411retx.o

.PHONY: clean-Source-2f-ThirdParty-2f-Core-2f-Startup

