ifndef app
app = 1
endif

# BASE = /PATH/TO/GNU/ARM/TOOLCHAIN/
# SAGE = /import/sage-7.4/local/lib/
# LD_LIBRARY_PATH = /usr/local/lib:$(SAGE)

CROSS = arm-none-eabi-
CC = $(BASE)$(CROSS)gcc
LD = $(BASE)$(CROSS)ld
QEMU = $(BASE)qemu-bsprak

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=arm920t -O0 -g
LSCRIPT = $(SRCDIR)/kernel.lds
INCLUDES = -Iinclude/

BUILDDIR = build
BINDIR = bin
SRCDIR = src

PORT = 12345


DRIVERS = $(wildcard $(SRCDIR)/drivers/*.c)
LIB = $(wildcard $(SRCDIR)/lib/*.c)
SYS = $(wildcard $(SRCDIR)/sys/*.c)

$(shell mkdir -p $(BUILDDIR)/drivers)
$(shell mkdir -p $(BUILDDIR)/lib)
$(shell mkdir -p $(BUILDDIR)/sys)
$(shell mkdir -p $(BINDIR))

.PHONY: all
all: kernel

.PHONY: run
run:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(QEMU) -kernel $(BINDIR)/kernel

.PHONY: debug
debug:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(QEMU) -kernel $(BINDIR)/kernel -S -gdb tcp::$(PORT)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(BINDIR)


kernel:
	cat $(SRCDIR)/kernel.c $(SRCDIR)/app$(app).c $(DRIVERS) $(LIB) $(SYS) > $(BUILDDIR)/temp.c
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(CC) $(INCLUDES) $(CFLAGS) -o $(BUILDDIR)/$@.o -c $(BUILDDIR)/temp.c
	$(LD) -T$(LSCRIPT) -o $(BUILDDIR)/$@ $(BUILDDIR)/$@.o
	cp $(BUILDDIR)/$@ $(BINDIR)/$@
