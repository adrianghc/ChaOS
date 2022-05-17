## QEMU Patch

The patch is available as a diff or as a formatted patch to be applied via `git apply portux920t.diff` or `git am portux920t.patch`, respecitvely.

It must be applied on top of `stable-4.2` (`6cdf8c4efa073eac7d5f9894329e2d07743c2955`). The QEMU source code is available on https://gitlab.com/qemu-project/qemu.git

### Build instructions

```bash
git clone https://gitlab.com/qemu-project/qemu.git
cd qemu
git checkout 6cdf8c4efa073eac7d5f9894329e2d07743c2955
<Apply patch>
mkdir build && cd build
../configure --disable-werror --target-list=arm-softmmu
make -j$(nproc)
```

### Running

If the build is successful, there will be a binary called `qemu-system-arm` in `build/arm-softmmu`. This binary needs to be called as follows:

```bash
qemu-system-arm -M portux920t \
                -m 64M \
                -nodefaults \
                -nographic \
                -serial mon:stdio \
                -kernel path/to/kernel.elf
```

This will launch QEMU without a window, multiplexing serial and the QEMU monitor into stdio. Alternatively, QEMU can be launched with a graphical window:

```bash
qemu-system-arm -M portux920t \
                -m 64M \
                -nodefaults \
                -serial vc \
                -kernel path/to/kernel.elf
```
