## QEMU Patch

The patch is available as a diff or as a formatted patch to be applied via `git apply portux920t.diff` or `git am portux920t.patch`, respecitvely.

It must be applied on top of `stable-4.2` (`6cdf8c4efa073eac7d5f9894329e2d07743c2955`). The QEMU source code is available on https://gitlab.com/qemu-project/qemu.git

It is required that `pkg-config`, `glib`, `gthread` and `pixman` be installed. Refer to the [troubleshooting section](#troubleshooting) below for possible solutions.

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

### Troubleshooting

You may encounter the following errors when building the project. Below are possible solutions that
you can apply if you are running Debian, Ubuntu, or a similar derivative with the APT package manager.

> ERROR: pkg-config binary 'pkg-config' not found

Solution: Execute `sudo apt install pkgconf`

> ERROR: glib-2.48 gthread-2.0 is required to compile QEMU

Solution: Execute `sudo apt install libglib2.0-dev`

> ERROR: pixman >= 0.21.8 not present. Please install the pixman devel package.

Solution: Execute `sudo apt install libpixman-1-dev`


### Running

If the build is successful, there will be a binary called `qemu-system-arm` in `build/arm-softmmu`.
This binary needs to be called as follows:

```bash
qemu-system-arm -M portux920t \
                -m 64M \
                -nodefaults \
                -nographic \
                -serial mon:stdio \
                -kernel path/to/kernel.elf
```

This will launch QEMU without a window, multiplexing serial and the QEMU monitor into stdio.
Alternatively, QEMU can be launched with a graphical window:

```bash
qemu-system-arm -M portux920t \
                -m 64M \
                -nodefaults \
                -serial vc \
                -kernel path/to/kernel.elf
```
