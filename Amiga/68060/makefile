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
	m68k-amigaos-gcc  -o "WCS" $(OBJS) $(USER_OBJS) $(LIBS) -noixemul -m68060 -fomit-frame-pointer -fbaserel -lm -lmui -DSTATIC_FCN=static -DSTATIC_VAR=static -mregparm -Winline -DSWMEM_FAST_INLINE -g -ldebug  -Wl,-Map=WCS_68060.map -flto  && ../check_wcs_amigaos_m68k.sh WCS
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) WCS
	-@echo ' '

pre-build:
	-realpath $$(which m68k-amigaos-gcc) &&   rm -fv Version.o; rm -fv WCS_68060*
	-@echo ' '

post-build:
	-cp WCS WCS_68060 && cp WCS_68060 WCS_68060.unstripped && m68k-amigaos-strip WCS_68060 && ls -la WCS_68060 && m68k-amigaos-size WCS_68060 && cp  ../WCS.info WCS_68060.info
	-@echo ' '

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
