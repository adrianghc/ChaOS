ifndef app
app = 1
endif

CROSS = arm-none-eabi-
CC = $(CROSS)gcc
LD = $(CROSS)ld
QEMU = $(BASE)qemu-bsprak

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=arm920t -O0 -g
LSCRIPT = $(SRCDIR)/kernel.lds
INCLUDES = -Iinclude/

OUTDIR = build
BINDIR = bin
SRCDIR = src

PORT = 12345


$(shell mkdir -p $(OUTDIR)/drivers)
$(shell mkdir -p $(OUTDIR)/lib)
$(shell mkdir -p $(OUTDIR)/sys)
$(shell mkdir -p $(BINDIR))

DRIVERS = $(patsubst $(SRCDIR)/drivers/%.c,$(OUTDIR)/drivers/%.o,$(wildcard $(SRCDIR)/drivers/*.c))
LIB = $(patsubst $(SRCDIR)/lib/%.c,$(OUTDIR)/lib/%.o,$(wildcard $(SRCDIR)/lib/*.c))
SYS = $(patsubst $(SRCDIR)/sys/%.c,$(OUTDIR)/sys/%.o,$(wildcard $(SRCDIR)/sys/*.c))

$(OUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@


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
	rm -rf $(OUTDIR)
	rm -rf $(BINDIR)


kernel: $(DRIVERS) $(LIB) $(SYS)
	cat $(SRCDIR)/kernel.c $(SRCDIR)/app$(app).c > $(OUTDIR)/temp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $(OUTDIR)/temp.c -o $(OUTDIR)/$@.o
	$(LD) -T$(LSCRIPT) -o $(OUTDIR)/$@ $(OUTDIR)/$@.o \
		$(wildcard $(OUTDIR)/drivers/*.o) $(wildcard $(OUTDIR)/lib/*.o) $(wildcard $(OUTDIR)/sys/*.o)
	cp $(OUTDIR)/$@ $(BINDIR)/$@
