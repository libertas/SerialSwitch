################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
SerialSwitch.lss \

FLASH_IMAGE += \
SerialSwitch.hex \

SIZEDUMMY += \
sizedummy \


# All Target
all: SerialSwitch.elf secondary-outputs

# Tool invocations
SerialSwitch.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,SerialSwitch.map -mmcu=atmega16 -o "SerialSwitch.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

SerialSwitch.lss: SerialSwitch.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S SerialSwitch.elf  >"SerialSwitch.lss"
	@echo 'Finished building: $@'
	@echo ' '

SerialSwitch.hex: SerialSwitch.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex SerialSwitch.elf  "SerialSwitch.hex"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: SerialSwitch.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega16 SerialSwitch.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) SerialSwitch.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
