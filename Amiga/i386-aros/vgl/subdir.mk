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

OBJS += \
./vgl/color.o \
./vgl/dumb.o \
./vgl/dumbpoly.o \
./vgl/pixmap.o \
./vgl/wuline.o 

C_DEPS += \
./vgl/color.d \
./vgl/dumb.d \
./vgl/dumbpoly.d \
./vgl/pixmap.d \
./vgl/wuline.d 


# Each subdirectory must supply rules for building sources it contributes
vgl/%.o: ../vgl/%.c vgl/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i386-aros-gcc -DFORCE_MUIMASTER_VMIN=19 -D__far= -DAMIGA_GUI -DTOOLCHAIN_VER=\"'$(shell m68k-amigaos-toolchain_hashes.sh | tr '!-~' 'P-~!-O' | sed 's/\\/\\\\/g' )'\" -DUSHORT=UWORD -DSHORT=int16_t -D__stdargs= -D__chip= -D__saveds= -DSTACKED= -DMUI_OBSOLETE -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -I/home/developer/Desktop/IcarosDesktop/Development/include/SDI -I/home/developer/Desktop/IcarosDesktop/Development/include -O0 -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<" -DBUILDID=\"g/'$(shell git describe --always --dirty --exclude "*")'\" -DSTATIC_FCN=static -DSTATIC_VAR=static -Winline -DSWMEM_FAST_INLINE -g
	@echo 'Finished building: $<'
	@echo ' '


