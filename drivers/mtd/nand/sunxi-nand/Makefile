#!makefile

KERNEL_DIR ?= ../linux-a10

.PHONY : all clean
all:
	@$(MAKE) -C $(KERNEL_DIR) M=$$PWD modules

clean:
	@$(MAKE) -C $(KERNEL_DIR) M=$$PWD clean


