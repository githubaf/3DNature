################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include x86_64-aros/vgl/subdir.mk
-include x86_64-aros/subdir.mk
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
	x86_64-aros-gcc  -o "WCS" $(OBJS) $(USER_OBJS) $(LIBS)  -fomit-frame-pointer  -lm -lmui -DSTATIC_FCN=static -DSTATIC_VAR=static  -Winline -DSWMEM_FAST_INLINE -g -ldebug  -Wl,-Map=WCS_test_x86_64-aros.map  && ../check_wcs_aros_i386.sh WCS
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) WCS
	-@echo ' '

pre-build:
	-realpath $$(which x86_64-aros-gcc) &&   rm -fv Version.o; rm -fv WCS_test_x86_64-aros*
	-@echo ' '

post-build:
	-cp WCS WCS_test_x86_64-aros && cp WCS_test_x86_64-aros WCS_test_x86_64-aros.unstripped && x86_64-aros-strip --strip-unneeded -R.comment  WCS_test_x86_64-aros && ls -la WCS_test_x86_64-aros && x86_64-aros-size WCS_test_x86_64-aros && cp  ../WCS.info WCS_test_x86_64-aros.info && ../check_pre-commit-hook
	-@echo ' '

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
