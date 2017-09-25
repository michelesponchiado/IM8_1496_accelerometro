################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: IM8_1496_accelerometro.axf

# Tool invocations
IM8_1496_accelerometro.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nodefaultlibs -nostdlib -L/home/michele/LPCXpresso/workspace/lpc_chip_13xx/Debug -Xlinker -Map="IM8_1496_accelerometro.map" -Xlinker --gc-sections -flto -Os -mcpu=cortex-m3 -mthumb -T "IM8_1496_accelerometro_Debug.ld" -o "IM8_1496_accelerometro.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) IM8_1496_accelerometro.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "IM8_1496_accelerometro.axf" ; arm-none-eabi-objcopy -v -O binary "IM8_1496_accelerometro.axf" "IM8_1496_accelerometro.bin" ; checksum -p LPC1313 -d "IM8_1496_accelerometro.bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
