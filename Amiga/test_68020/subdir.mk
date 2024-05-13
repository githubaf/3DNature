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
	m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DSTATIC_FCN=static -DAMIGA_GUI -DTOOLCHAIN_VER=\"'$(shell m68k-amigaos-toolchain_hashes.sh | tr '!-~' 'P-~!-O' | sed 's/\\/\\\\/g' )'\" -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O0 -g3 -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<" -DBUILDID=\"g/'$(shell git describe --always --dirty --exclude "*")'\" -noixemul -fprofile-dir=test_68020 -fprofile-arcs -ftest-coverage  -m68020 -m68881 -DSTATIC_FCN=static -DSTATIC_VAR=static  -Winline -DSWMEM_FAST_INLINE -g
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./BigEndianReadWrite.d ./BigEndianReadWrite.o ./BitMaps.d ./BitMaps.o ./DEM.d ./DEM.o ./DataOps.d ./DataOps.o ./MapSupport.d ./MapSupport.o ./Memory.d ./Memory.o ./WCS_locale.d ./WCS_locale.o ./sasc_functions.d ./sasc_functions.o ./test_main.d ./test_main.o

.PHONY: clean--2e-

