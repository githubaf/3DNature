################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vgl/color.c \
../vgl/dumb.c \
../vgl/dumbpoly.c \
../vgl/pixmap.c \
../vgl/wuline.c 

O_SRCS += \
../vgl/clib.o \
../vgl/color.o \
../vgl/defpal.o \
../vgl/dumb.o \
../vgl/dumbbitblt.o \
../vgl/dumbpoly.o \
../vgl/dumbtext.o \
../vgl/fontsmall.o \
../vgl/pixmap.o \
../vgl/wuline.o 

C_DEPS += \
./vgl/color.d \
./vgl/dumb.d \
./vgl/dumbpoly.d \
./vgl/pixmap.d \
./vgl/wuline.d 

OBJS += \
./vgl/color.o \
./vgl/dumb.o \
./vgl/dumbpoly.o \
./vgl/pixmap.o \
./vgl/wuline.o 


# Each subdirectory must supply rules for building sources it contributes
vgl/%.o: ../vgl/%.c vgl/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DTOOLCHAIN_VER=\"'$(shell m68k-amigaos-toolchain_hashes.sh | tr '!-~' 'P-~!-O' | sed 's/\\/\\\\/g' )'\" -DAMIGA_GUI -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O2 -g -pg -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<" -DBUILDID=\"g/'$(shell git describe --always --dirty)'\" -noixemul -fprofile-dir=/WCS_204 -m68040 -DSTATIC_FCN=static -DSTATIC_VAR=static -mregparm -Winline -DSWMEM_FAST_INLINE -g
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-vgl

clean-vgl:
	-$(RM) ./vgl/color.d ./vgl/color.o ./vgl/dumb.d ./vgl/dumb.o ./vgl/dumbpoly.d ./vgl/dumbpoly.o ./vgl/pixmap.d ./vgl/pixmap.o ./vgl/wuline.d ./vgl/wuline.o

.PHONY: clean-vgl

