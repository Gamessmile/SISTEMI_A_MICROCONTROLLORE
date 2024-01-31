################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c 

OBJS += \
./src/main.o 

C_DEPS += \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32F4 -DSTM32F401RETx -DNUCLEO_F401RE -DDEBUG -DSTM32F401xE -I"C:/Users/Games/Desktop/Laboratorio di sistemi a microcontrollore/file per stm/STM32CubeF4-master/Drivers/CMSIS/Include" -I"C:/Users/Games/Desktop/Laboratorio di sistemi a microcontrollore/file per stm/STM32CubeF4-master/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/Games/Desktop/Laboratorio di sistemi a microcontrollore/file per stm/stm32_unict_lib/inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


