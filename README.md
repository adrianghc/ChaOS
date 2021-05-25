# ChaOS

### Tim Scheuermann, Julian Holzwarth & Adrian Herrmann

This project was developed during Barry Linnert's operating systems course at Freie Universität Berlin over the course of fourteen weeks in 2018-2019.

It is a small microkernel operating system developed from scratch for the ARM-based *taskit Portux MiniPC* SoC with an *AT91RM9200* CPU, 16 MiB Flash memory and 64 MiB RAM.

## Supported features

* Serial interface support
* Processor modes, stack, dynamic kernel memory management, exception handling
* Interrupts and timers
* Context switches, simple round-robin-based thread scheduling (preemptive multitasking)
* Syscalls, device drivers
* Memory virtualization with MMU, logical address spaces, processes, threads

There are two example applications that demonstrate several capabilities of the kernel:

1. An application to demonstrate address space separation, context switching, system calls and process/thread creation.
2. An application to demonstrate protection against various forbidden actions.

## Instructions

* Build by running *app=\<num> make*, where *\<num>* is the example application that should run (*1* or *2*).
* Run by running *make run*.
* *make debug* starts a debuggable session (under TCP port 12345 by default) that GDB can then connect to.

### Requirements

* SageMath is required for building. Version 7.4 has been tested.
* The GNU Arm Embedded Toolchain is required for building.
* A patched version of QEMU is required to run the OS. The patch (by Leonard König) is available in the *qemu-patch* directory.

## Design

### Directory structure

*src* contains all the source code, in particular code for the kernel entry point, two example applications and the kernel linker script in its root.

[*src*/*include*]/*drivers* contains device drivers (e.g. for DBGU or ST) and functions that interact with hardware directly (e.g. for writing in specific memory areas).

[*src*/*include*]/*lib* contains application libraries with utilities and basic functions that facilitate application and driver development and do not interact with the hardware directly but via the drivers. E.g. *math.c* provides mathematical functions and *buffer.c* a ring buffer implementation.

[*src*/*include*]/*sys* contains operating system libraries that are not for application use but only for drivers and the system.

*doc* contains documentation, including the syscall documentation.

*qemu-patch* includes the patch necessary for QEMU to emulate the target platform.

### Memory organization

All interrupt mode stacks are placed at the end of the internal RAM.

FIQ: 0x002FFFF8 - 0x002FDFF9\
IRQ: 0x002FDFF8 - 0x002FBFF9\
SVC: 0x002FBFF8 - 0x002F9FF9\
ABT: 0x002F9FF8 - 0x002F7FF9\
UND: 0x002F7FF8 - ...

The system mode stack is placed at the end of the external RAM.

SYS: 0x23FFFFF8 - ...
