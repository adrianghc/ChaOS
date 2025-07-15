# ChaOS

### Tim Scheuermann, Julian Holzwarth & Adrian Herrmann

This project was developed during Barry Linnert's operating systems course at Freie Universität
Berlin over the course of fourteen weeks in 2018-2019, plus minor adjustments in subsequent years.

It is a small operating system developed from scratch for the ARM-based *taskit Portux MiniPC* 
SoC with an *AT91RM9200* CPU, 16 MiB Flash memory and 64 MiB RAM.

## Supported features

* Serial interface (DBGU) driver via MMIO (supports read/write, interrupts)
* Dynamic kernel memory management
* Processor modes, stacks (svc, und, abt, irq, fiq)
* Interrupt handlers (Undefined Instruction, SWI, Prefetch Abort, Data Abort, IRQ, FIQ)
* System timer and scheduling
* Processes/threads, context switches, simple round-robin-based scheduling (preemptive multitasking)
* Memory protection and logical address spaces via MMU
* User/kernel interface (syscalls, utility library)

There are two example applications that demonstrate several capabilities of the kernel:

1. An application to demonstrate address space separation, context switching, system calls and
   process/thread creation:
    * An initial process waits for user input and creates a new process when a character is entered
      (with that character as the process' input).
    * The new process stores the character in its address space, initializes a counter at 0 and
      starts two new threads in its own address space.
    * All three threads enter a loop where they increase the shared counter and a private counter as
      long as the former is under a predefined limit, print a message, and sleep for a moment.
    * The printed message is formatted as follows:
      `<letter><thread number>: <global counter value> (<local counter value>)`
2. An application to demonstrate protection against various forbidden actions:
    * 0 - Accessing a NULL pointer.
    * 1 - Reading kernel data.
    * 2 - Writing into program text in memory.
    * 3 - Overflowing the stack.
    * 4 - Reading from an unmapped address.
    * 5 - Reading from an address that would normally be unmapped.

## Limitations

* No filesystem - everything happens in RAM.
* Therefore, also no dynamic loading of code - everything is statically linked into the kernel binary.

## Instructions

* Recommended: Clone, patch and build QEMU by running `make qemu`.
* Build by running `app=<num> make`, where `<num>` is the example application that should run (`1` or `2`).
* Run by running `make run` (this assumes the QEMU binary to be in `qemu/build/arm-softmmu/qemu-system-arm`).
* `make debug` starts a debuggable session (under TCP port 12345 by default) that GDB can then
  connect to (this also assumes the above location for the QEMU binary).
* Exit from QEMU by pressing Ctrl + A, then X.

### Requirements

* The GNU Arm Embedded Toolchain (`arm-none-eabi`) is required for building.
* A patched version of QEMU is required to virtualize the OS and can be built automatically by
  running `make qemu`. Alternatively, the patch and instructions for how to build and run the
  patched QEMU manually are available in the `qemu-patch` directory. Building QEMU requires that
  `pkg-config`, `glib`, `gthread` and `pixman` be installed. Refer to the
  [QEMU patch's README](qemu-patch/README.md) for troubleshooting steps.

## Design

### Directory structure

`src` contains all the source code, in particular code for the kernel entry point, two example
applications and the kernel linker script in its root.

`[src/include]/drivers` contains device drivers (e.g. for DBGU or ST) and functions that interact
with hardware directly (e.g. for writing in specific memory areas).

`[src/include]/lib` contains application libraries with utilities and basic functions that
facilitate application and driver development and do not interact with the hardware directly but via
the drivers. E.g. `math.c` provides mathematical functions and `buffer.c` a ring buffer implementation.

`[src/include]/sys` contains operating system libraries that are not for application use but only
for drivers and the system.

`doc` contains documentation, including the syscall documentation.

`qemu-patch` includes the patch necessary for QEMU to emulate the target platform.

### Memory layout

All interrupt mode stacks are placed at the end of the internal RAM.

```
FIQ: 0x002FFFF8 - 0x002FDFF9
IRQ: 0x002FDFF8 - 0x002FBFF9
SVC: 0x002FBFF8 - 0x002F9FF9
ABT: 0x002F9FF8 - 0x002F7FF9
UND: 0x002F7FF8 - ...
```

The system mode stack is placed at the end of the external RAM.

```
SYS: 0x23FFFFF8 - ...
```
