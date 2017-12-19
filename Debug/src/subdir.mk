################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/accelerometer.c \
../src/alarms.c \
../src/cr_startup_lpc13xx.c \
../src/crc3bit.c \
../src/dipswitches.c \
../src/eeprom.c \
../src/encoder.c \
../src/i2c.c \
../src/led.c \
../src/main.c \
../src/relais.c \
../src/sysinit.c \
../src/system_tick.c \
../src/table.c \
../src/uart.c 

OBJS += \
./src/accelerometer.o \
./src/alarms.o \
./src/cr_startup_lpc13xx.o \
./src/crc3bit.o \
./src/dipswitches.o \
./src/eeprom.o \
./src/encoder.o \
./src/i2c.o \
./src/led.o \
./src/main.o \
./src/relais.o \
./src/sysinit.o \
./src/system_tick.o \
./src/table.o \
./src/uart.o 

C_DEPS += \
./src/accelerometer.d \
./src/alarms.d \
./src/cr_startup_lpc13xx.d \
./src/crc3bit.d \
./src/dipswitches.d \
./src/eeprom.d \
./src/encoder.d \
./src/i2c.d \
./src/led.d \
./src/main.d \
./src/relais.d \
./src/sysinit.d \
./src/system_tick.d \
./src/table.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -I"/home/michele/LPCXpresso/workspace/lpc_chip_13xx/inc" -I"/home/michele/LPCXpresso/workspace/IM8_1496_accelerometro/inc" -Os -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


