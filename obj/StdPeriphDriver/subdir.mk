################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_adc.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_clk.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_flash.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_gpio.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_pwr.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_sys.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_timer0.c \
c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_uart1.c 

C_DEPS += \
./StdPeriphDriver/CH57x_adc.d \
./StdPeriphDriver/CH57x_clk.d \
./StdPeriphDriver/CH57x_flash.d \
./StdPeriphDriver/CH57x_gpio.d \
./StdPeriphDriver/CH57x_pwr.d \
./StdPeriphDriver/CH57x_sys.d \
./StdPeriphDriver/CH57x_timer0.d \
./StdPeriphDriver/CH57x_uart1.d 

OBJS += \
./StdPeriphDriver/CH57x_adc.o \
./StdPeriphDriver/CH57x_clk.o \
./StdPeriphDriver/CH57x_flash.o \
./StdPeriphDriver/CH57x_gpio.o \
./StdPeriphDriver/CH57x_pwr.o \
./StdPeriphDriver/CH57x_sys.o \
./StdPeriphDriver/CH57x_timer0.o \
./StdPeriphDriver/CH57x_uart1.o 

DIR_OBJS += \
./StdPeriphDriver/*.o \

DIR_DEPS += \
./StdPeriphDriver/*.d \

DIR_EXPANDS += \
./StdPeriphDriver/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
StdPeriphDriver/CH57x_adc.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_adc.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_clk.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_clk.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_flash.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_flash.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_gpio.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_gpio.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_pwr.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_pwr.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_sys.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_sys.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_timer0.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_timer0.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH57x_uart1.o: c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/CH57x_uart1.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Startup" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/APP/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/BLE_MOTOR/Profile/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/HAL/include" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/Ld" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/BLE/LIB" -I"c:/Users/dueasy/Desktop/智能轮子/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

