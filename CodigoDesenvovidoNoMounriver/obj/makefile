################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
Embarcados1-1.hex \

SECONDARY_LIST += \
Embarcados1-1.lst \

SECONDARY_SIZE += \
Embarcados1-1.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: Embarcados1-1.elf secondary-outputs

# Tool invocations
Embarcados1-1.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "c:\Users\marti\OneDrive\mounriver\Embarcados1-1\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"Embarcados1-1.map" --specs=nano.specs --specs=nosys.specs -o "Embarcados1-1.elf" $(OBJS) $(LIBS) -lprintf
Embarcados1-1.hex: Embarcados1-1.elf
	@	riscv-none-embed-objcopy -O ihex "Embarcados1-1.elf" "Embarcados1-1.hex"
Embarcados1-1.lst: Embarcados1-1.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "Embarcados1-1.elf" > "Embarcados1-1.lst"
Embarcados1-1.siz: Embarcados1-1.elf
	riscv-none-embed-size --format=berkeley "Embarcados1-1.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) Embarcados1-1.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets