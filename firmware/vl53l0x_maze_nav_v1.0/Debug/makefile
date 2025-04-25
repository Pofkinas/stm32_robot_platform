################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Source/Utility/subdir.mk
-include Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Source/ThirdParty/Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Source/ThirdParty/Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Source/ThirdParty/Core/Startup/subdir.mk
-include Source/ThirdParty/Core/Src/subdir.mk
-include Source/Driver/subdir.mk
-include Source/APP/subdir.mk
-include Source/API/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := vl53l0x_maze_nav
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
vl53l0x_maze_nav.elf \

MAP_FILES += \
vl53l0x_maze_nav.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
vl53l0x_maze_nav.list \


# All Target
all: main-build

# Main-build Target
main-build: vl53l0x_maze_nav.elf secondary-outputs

# Tool invocations
vl53l0x_maze_nav.elf vl53l0x_maze_nav.map: $(OBJS) $(USER_OBJS) C:\Users\povil\OneDrive\Documents\Projects\stm32_robot_platform\firmware\vl53l0x_maze_nav_v1.0\STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "vl53l0x_maze_nav.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\povil\OneDrive\Documents\Projects\stm32_robot_platform\firmware\vl53l0x_maze_nav_v1.0\STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="vl53l0x_maze_nav.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

vl53l0x_maze_nav.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "vl53l0x_maze_nav.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout vl53l0x_maze_nav.elf vl53l0x_maze_nav.list vl53l0x_maze_nav.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
