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
../sasc_functions.c \
../test_main.c 

O_SRCS += \
../AGUI.o \
../BigEndianReadWrite.o \
../BitMaps.o \
../Cloud.o \
../CloudGUI.o \
../ColorBlends.o \
../Commands.o \
../DEM.o \
../DEMGUI.o \
../DEMObject.o \
../DLG.o \
../DataBase.o \
../DataOps.o \
../DataOpsGUI.o \
../DefaultParams.o \
../DiagnosticGUI.o \
../DispatchGUI.o \
../EdDBaseGUI.o \
../EdEcoGUI.o \
../EdMoGUI.o \
../EdPar.o \
../EdSetExtrasGUI.o \
../EdSetGUI.o \
../EditGui.o \
../EvenMoreGUI.o \
../Foliage.o \
../FoliageGUI.o \
../Fractal.o \
../GenericParams.o \
../GenericTLGUI.o \
../GlobeMap.o \
../GlobeMapSupport.o \
../GrammarTable.o \
../HyperKhorner4M-1.o \
../Images.o \
../InteractiveDraw.o \
../InteractiveUtils.o \
../InteractiveView.o \
../LWSupport.o \
../LineSupport.o \
../MakeFaces.o \
../Map.o \
../MapExtra.o \
../MapGUI.o \
../MapLineObject.o \
../MapSupport.o \
../MapTopo.o \
../MapTopoObject.o \
../MapUtil.o \
../Memory.o \
../MoreGUI.o \
../Params.o \
../ParamsGUI.o \
../PlotGUI.o \
../RequesterGUI.o \
../RexxSupport.o \
../ScratchPad.o \
../ScreenModeGUI.o \
../Support.o \
../TLSupportGUI.o \
../TimeLinesGUI.o \
../Tree.o \
../Version.o \
../VocabTable.o \
../WCS.o \
../Wave.o \
../WaveGUI.o \
../nncrunch.o \
../nngridr.o \
../sasc_functions.o 

C_DEPS += \
./BigEndianReadWrite.d \
./BitMaps.d \
./DEM.d \
./DataOps.d \
./MapSupport.d \
./Memory.d \
./sasc_functions.d \
./test_main.d 

OBJS += \
./BigEndianReadWrite.o \
./BitMaps.o \
./DEM.o \
./DataOps.o \
./MapSupport.o \
./Memory.o \
./sasc_functions.o \
./test_main.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i386-aros-gcc -DFORCE_MUIMASTER_VMIN=19 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'$(shell m68k-amigaos-toolchain_hashes.sh | tr '!-~' 'P-~!-O' | sed 's/\\/\\\\/g' )'\" -DUSHORT=UWORD -DSHORT=int16_t -D__stdargs= -D__chip= -D__far= -D__saveds= -DSTACKED= -DMUI_OBSOLETE= -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O0 -g3 -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<" -DBUILDID=\"g/'$(shell git describe --always --dirty --exclude "*")'\"  -fomit-frame-pointer  -DSTATIC_FCN=static -DSTATIC_VAR=static  -Winline -DSWMEM_FAST_INLINE -g
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./BigEndianReadWrite.d ./BigEndianReadWrite.o ./BitMaps.d ./BitMaps.o ./DEM.d ./DEM.o ./DataOps.d ./DataOps.o ./MapSupport.d ./MapSupport.o ./Memory.d ./Memory.o ./sasc_functions.d ./sasc_functions.o ./test_main.d ./test_main.o

.PHONY: clean--2e-

