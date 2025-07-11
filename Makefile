ifndef app
app = 1
endif

CROSS = arm-none-eabi-
CC = $(CROSS)gcc
LD = $(CROSS)ld
QEMU = qemu/build/arm-softmmu/qemu-system-arm
QEMU_ARGS = -M portux920t -m 64M -nodefaults -nographic -serial mon:stdio

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=arm920t -O0 -g
LSCRIPT = $(SRCDIR)/kernel.lds
INCLUDES = -Iinclude/

OUTDIR = build
BINDIR = bin
SRCDIR = src

PORT = 12345

$(shell mkdir -p $(BINDIR))

DRIVERS = $(patsubst $(SRCDIR)/drivers/%.c, $(OUTDIR)/drivers/%.o, $(wildcard $(SRCDIR)/drivers/*.c))
LIB = $(patsubst $(SRCDIR)/lib/%.c, $(OUTDIR)/lib/%.o, $(wildcard $(SRCDIR)/lib/*.c))
SYS = $(patsubst $(SRCDIR)/sys/%.c, $(OUTDIR)/sys/%.o, $(wildcard $(SRCDIR)/sys/*.c))

.PHONY: all qemu clean-qemu run debug clean

all: kernel

qemu:
	git clone https://gitlab.com/qemu-project/qemu.git && \
	cd qemu && git checkout 6cdf8c4efa073eac7d5f9894329e2d07743c2955 && \
	git apply ../qemu-patch/portux920t.diff && \
	mkdir build && cd build && \
	../configure --disable-werror --target-list=arm-softmmu && make -j$(nproc)

clean-qemu:
	rm -rf qemu

run:
	$(QEMU) $(QEMU_ARGS) -kernel $(BINDIR)/kernel

debug:
	$(QEMU) $(QEMU_ARGS) -kernel $(BINDIR)/kernel -S -gdb tcp::$(PORT)

clean:
	rm -rf $(OUTDIR)
	rm -rf $(BINDIR)

kernel: $(DRIVERS) $(LIB) $(SYS) $(OUTDIR)/app$(app).o $(OUTDIR)/kernel.o
	$(LD) -T$(LSCRIPT) --no-warn-rwx-segments -o $(OUTDIR)/$@ $(OUTDIR)/$@.o $(OUTDIR)/app$(app).o \
		$(wildcard $(OUTDIR)/drivers/*.o) $(wildcard $(OUTDIR)/lib/*.o) $(wildcard $(OUTDIR)/sys/*.o)
	cp $(OUTDIR)/$@ $(BINDIR)/$@

$(OUTDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
