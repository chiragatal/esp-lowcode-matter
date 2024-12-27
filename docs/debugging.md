# Debugging

* [Debugging](#debugging)
* [Breakpoint Panic](#breakpoint-panic)
* [Illegal Instruction Panic](#illegal-instruction-panic)
* [Logging](#logging)
* [Recommended coding practices](#recommended-coding-practices)

The ESP32-C6 microcontroller features two processing cores: the High-Performance (HP) Core and the Low-Power (LP) Core. Each core serves a specific purpose in the device’s operation. The HP Core is primarily responsible for initializing and managing essential system functions, including the Wi-Fi, Bluetooth, and Matter protocol stacks. Once these systems are initialized, the HP Core also loads the firmware intended for the LP Core into memory.

The LP Core, in turn, communicates with the HP Core, enabling event-driven communication where it can both send and receive events to/from the HP Core. The firmware running on the device is specifically designed for the LP Core and is typically flashed using a low-code platform.

The initialization process begins with the HP Core, which sets up the necessary communication protocols and handles the firmware loading for the LP Core. Once the HP Core has completed its tasks, the LP Core is then initialized, ready to execute the firmware that has been loaded into memory. This sequence is crucial for the proper operation of the device and must be understood when debugging any firmware issues.

The LP Core, like any processor, can encounter errors during execution that result in panic conditions. There are two primary scenarios that can cause the LP Core to panic:

## Breakpoint Panic

A breakpoint panic occurs when the LP Core firmware attempts to access null memory. An example of the panic output text is:

```sh
I (596) sys_info: ESP-AMP shared memory: addr=0x40879580, len=0x4fe0
I (603) esp-amp-loader: Reserved dram region (0x4086d560 - 0x40879560) for subcore
I (612) esp-amp-loader: Give unused reserved dram region (0x4086ec80 - 0x40879560) back to main-core heap
LP UART initialized successfully
app_main: Starting low code
Guru Meditation Error: Subcore panic'ed Breakpoint
Core 1 register dump:
MEPC    : 0x4086e922 RA      : 0x4086e91e SP      : 0x50003870 GP      : 0x00000000
TP      : 0x00000000 T0      : 0x50000000 T1      : 0x00000000 T2      : 0x00000000
S0/FP   : 0x00000000 S1      : 0x00000000 A0      : 0x0000001c A1      : 0x00000001
A2      : 0x00000000 A3      : 0x00000080 A4      : 0x60000000 A5      : 0x00000000
A6      : 0x00000000 A7      : 0x00000000 S2      : 0x00000000 S3      : 0x00000000
S4      : 0x00000000 S5      : 0x00000000 S6      : 0x00000000 S7      : 0x00000000
S8      : 0x00000000 S9      : 0x00000000 S10     : 0x00000000 S11     : 0x00000000
T3      : 0x00000000 T4      : 0x00000000 T5      : 0x00000000 T6      : 0x00000000
MSTATUS : 0x00001800 MTVEC   : 0x50000001 MCAUSE  : 0x00000003 MTVAL   : 0x00000000
MHARTID : 0x00000001

Stack memory:
50003870: 0x00000000 0x00000000 0x00000000 0x4086de00 0x00000000 0x00000000 0x00000000 0x500000ae
50003890: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0xc0061141 0x00a000ef 0x01414082
500038b0: 0x47b78082 0xa7835000 0xc3918e47 0x80828782 0x500045b7 0x500047b7 0x8a478793 0x8ec58593
500038d0: 0x50004537 0x05138d9d 0xc3178a45 0x0067efff 0x00007ca3 0x00000000 0x00000000 0x02020000
500038f0: 0x00000202 0x0000003c 0x40829a78 0x00000014 0x00000014 0x000006dc 0x5000390c 0x28dfdfa0
50003910: 0x75db6737 0x5000390c 0x50003fd0 0x00000001 0x00000008 0x00000000 0x00000000 0x00000000
50003930: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
50003950: 0x00000000 0x5000390c 0x5000390c 0x5000390c 0x50003fd0 0x5000390c 0x5000390c 0x5000390c
50003970: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003990: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
500039b0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
500039d0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
500039f0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003a10: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003a30: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003a50: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003a70: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003a90: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003ab0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003ad0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003af0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003b10: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003b30: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003b50: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003b70: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003b90: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003bb0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003bd0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003bf0: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003c10: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003c30: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c
50003c50: 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c 0x5000390c

```

The output above is referred to as a stack dump. To identify the specific line of code that caused the panic, you can use the MEPC address from the stack dump, which in this case is 0x4086e922. To find the corresponding line of code, execute the following command in the terminal within the product's working directory:

```sh
riscv32-esp-elf-addr2line -e <path-to-elf-file> <mepc-address>
```

(The .elf file can be found in the build directory of the compiled product.)

NOTE: Not every null memory access will trigger a breakpoint panic. The compiler will insert breakpoints during optimization to handle null memory accesses only in cases where it can detect potential null memory access at compile time. However, if a null memory access occurs at runtime, a breakpoint panic may not be triggered. This distinction is crucial for understanding the conditions under which a breakpoint panic might occur.

## Illegal Instruction Panic

An illegal instruction panic is often triggered by a type of buffer overflow, particularly when the stack pointer overflows. This occurs when a buffer (such as an array or a memory region) exceeds its allocated memory space and begins to overwrite adjacent memory areas, including critical system data like the stack pointer.

```sh
Guru Meditation Error: Subcore panic'ed Unhandled interrupt/Unknown cause
Core 1 register dump:
MEPC    : 0x00000000 RA      : 0x00000000 SP      : 0x00000000 GP      : 0x00000000
TP      : 0x00000000 T0      : 0x00000000 T1      : 0x00000000 T2      : 0x00000000
S0/FP   : 0x00000000 S1      : 0x00000000 A0      : 0x00000000 A1      : 0x00000000
A2      : 0x00000000 A3      : 0x00000000 A4      : 0x00000000 A5      : 0x00000000
A6      : 0x00000000 A7      : 0x00000000 S2      : 0x00000000 S3      : 0x00000000
S4      : 0x00000000 S5      : 0x00000000 S6      : 0x00000000 S7      : 0x00000000
S8      : 0x00000000 S9      : 0x00000000 S10     : 0x00000000 S11     : 0x00000000
T3      : 0x00000000 T4      : 0x00000000 T5      : 0x00000000 T6      : 0x00000000
MSTATUS : 0x00000000 MTVEC   : 0x00000000 MCAUSE  : 0x00000000 MTVAL   : 0x00000000
MHARTID : 0x00000000
```

In this case the core dump generated may not be very useful for debugging. This is because the stack pointer or other critical memory addresses may have been corrupted, making it difficult to trace the exact point of failure from the core dump alone. As a result, the information provided in the dump may be incomplete or unreliable.

Instead, to identify the root cause of the issue, we must rely on [logging](#logging).

## Logging

* By enabling detailed logging throughout the code, especially around memory accesses and buffer operations, we can monitor the execution flow leading up to a panic. These logs provide valuable insights into the functions and operations executed prior to the overflow, helping developers identify the exact location of the buffer overflow and the affected memory addresses. This approach is crucial for diagnosing and resolving the issue, particularly when core dumps are insufficient for debugging.

* The LP core includes an implementation of the standard `printf` function, enabling log outputs to be displayed on the console.

* The LP core runs synchronously on a single thread, ensuring that all log outputs are sequential and in sync. This guarantees that log statements are printed in the exact order they are executed, making it easier to follow the execution flow and troubleshoot issues. This feature is especially useful for identifying logical errors in the firmware, as it allows developers to trace how different parts of the code interact step-by-step. By examining the log output, developers can pinpoint inconsistencies or unexpected behaviors, speeding up the process of diagnosing and resolving bugs.

## Recommended coding practices

* `Define and enforce buffer boundaries`: Always define buffer sizes explicitly, and never exceed them. Implement checks to ensure that data does not overflow past the allocated memory, especially when dealing with arrays, buffers, or memory structures.

* `Do not use memory allocation function`: using memory allocation function like malloc, calloc etc, can result in undefined behaviour, use statically allocated buffers instead. This reduces the risk of memory fragmentation and ensures that memory is used efficiently.

* `Limit pointer arithmetic`: Avoid complex pointer arithmetic that could lead to accessing unintended memory regions. Be cautious when incrementing or decrementing pointers, especially when working with buffers.

* `Avoid Deep Recursion`: Whenever possible, replace recursive functions with iterative solutions. This eliminates the risk of stack overflow, as iterative functions use a constant amount of stack space regardless of the depth of the loop.

(Note: In the case of LP firmware, illegal memory access does not always result in a panic. The firmware may not panic when reading from or writing to illegal memory. However, when reading from illegal memory, it will always return 0x0.)

## Related Documents

* [Create LowCode Product](./create_product.md)
* [Product Configuration](./product_configuration.md)
* [Programmer's Model](./programmer_model.md)
* [All Documents](./all_documents.md)
