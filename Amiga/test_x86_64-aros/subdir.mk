################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BigEndianReadWrite.c \
../BitMaps.c \
../DEM.c \
../DataOps.c \
../MapSupport.c \
../Memory.c \
../WCS_locale.c \
../sasc_functions.c \
../test_main.c 

O_SRCS += \
../AGUI.o \
../BitMaps.o \
../Cloud.o \
../CloudGUI.o \
../WCS.o 

C_DEPS += \
./BigEndianReadWrite.d \
./BitMaps.d \
./DEM.d \
./DataOps.d \
./MapSupport.d \
./Memory.d \
./WCS_locale.d \
./sasc_functions.d \
./test_main.d 

OBJS += \
./BigEndianReadWrite.o \
./BitMaps.o \
./DEM.o \
./DataOps.o \
./MapSupport.o \
./Memory.o \
./WCS_locale.o \
./sasc_functions.o \
./test_main.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-aros-gcc -DFORCE_MUIMASTER_VMIN=19 -D__far= -DAMIGA_GUI -DTOOLCHAIN_VER=\"'$(shell ../aros_deadw00d_toolchain_hashes.sh master | tr '!-~' 'P-~!-O' | sed 's/\\/\\\\/g' )'\" -DUSHORT=UWORD -DSHORT=int16_t -D__stdargs= -D__chip= -D__saveds= -DSTACKED= -DMUI_OBSOLETE -I".." -O0 -g3 -Wall -c -fmessage-length=0 -funsigned-char -fno-common -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<" -DBUILDID=\"g/'$(shell git describe --always --dirty --exclude "*")'\"  -fomit-frame-pointer  -DSTATIC_FCN=static -DSTATIC_VAR=static  -Winline -DSWMEM_FAST_INLINE -g
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./BigEndianReadWrite.d ./BigEndianReadWrite.o ./BitMaps.d ./BitMaps.o ./DEM.d ./DEM.o ./DataOps.d ./DataOps.o ./MapSupport.d ./MapSupport.o ./Memory.d ./Memory.o ./WCS_locale.d ./WCS_locale.o ./sasc_functions.d ./sasc_functions.o ./test_main.d ./test_main.o

.PHONY: clean--2e-

