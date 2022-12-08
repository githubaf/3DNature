################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include vgl/subdir.mk
-include subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := WCS
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory pre-build && $(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: WCS

# Tool invocations
WCS: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	i386-aros-gcc  -o "WCS" $(OBJS) $(USER_OBJS) $(LIBS) -noixemul -m68020 -m68881 -fomit-frame-pointer -fbaserel -lm -lmui -DSTATIC_FCN=static -DSTATIC_VAR=static -mregparm -Winline -DSWMEM_FAST_INLINE -g -ldebug  -Wl,-Map=WCS_i386-aros.map -flto && ../check_wcs.sh WCS
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) WCS
	-@echo ' '

pre-build:
	-realpath $$(which m68k-amigaos-gcc) &&   rm -fv Version.o; rm -fv WCS_i386-aros*
	-@echo ' '

post-build:
	-cp WCS WCS_i386-aros && cp WCS_i386-aros WCS_i386-aros.unstripped && m68k-amigaos-strip WCS_i386-aros && ls -la WCS_i386-aros && m68k-amigaos-size WCS_i386-aros && cp  ../WCS.info WCS_i386-aros.info
	-@echo ' '

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
