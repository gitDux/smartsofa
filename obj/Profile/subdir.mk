################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Profile/devinfoservice.c \
../Profile/gattprofile.c 

C_DEPS += \
./Profile/devinfoservice.d \
./Profile/gattprofile.d 

OBJS += \
./Profile/devinfoservice.o \
./Profile/gattprofile.o 

DIR_OBJS += \
./Profile/*.o \

DIR_DEPS += \
./Profile/*.d \

DIR_EXPANDS += \
./Profile/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Profile/%.o: ../Profile/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

