################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include vgl/subdir.mk
-include noftastmatgregparm/vgl/subdir.mk
-include nofastmathregparm/vgl/subdir.mk
-include nofastmathregparm/esp/.metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include nofastmathregparm/subdir.mk
-include no_lto/vgl/subdir.mk
-include no_lto/esp/.metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include no_lto/subdir.mk
-include esp/.metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include CONFIG_Optimization/vgl/subdir.mk
-include CONFIG_Optimization/subdir.mk
-include subdir.mk
-include objects.mk

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
all: WCS

# Tool invocations
WCS: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	m68k-amigaos-gcc  -o "WCS" $(OBJS) $(USER_OBJS) $(LIBS) -noixemul -m68040 -fomit-frame-pointer -fbaserel -lm -lmui -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE -flto
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) WCS
	-@echo ' '

post-build:
	-cp WCS_Release WCS_Release.unstripped && m68k-amigaos-strip WCS_Release && ls -la WCS_Release && m68k-amigaos-size WCS_Release && cp  ../WCS.info WCS_Release.info
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
