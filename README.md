# Simple AML Bytecode Interpreter
**SABI** is a simple bytecode interpreter for the ACPI Machine Language (AML), which is an essential component of the firmware (such as UEFI) on many modern machines. As such, the code allows an operating system to implement full ACPI support in the kernel.

The code is currently under active development and, while most things work, there are still many unimplemented functions and corner cases. Moreover, the code has been developed for a 64-bit architecture, and it will very like not work as intended on a 32-bit system. **Use at your own risk!**

## Host functions
The host OS must provide a set of basic functions needed for SABI to work. These functions include memory management, access to hardware registers, timing functions, and error handling. All the necessary host functions are templated in `source/host.c` which serves as a wrapper for the actual functions implemented by the underlying operating system.

Besides from the host functions, the code assumes the existence of `stdint.h` and a minimally working implementation of `string.h` containing the following C99 compliant functions.

```C
memcpy()
memset()
strcpy()
strlen()
strcmp()
strncmp()
```

## Public functions
The public functions allow the operating system to execute functions within the AML code, as well as navigating the ACPI namespace. There are also auxiliary functions that perform predefined tasks, such as parsing the PCI routing tables, parsing resource templates, handling events, and changing the system power state. The public functions can be found in the following header files.

```
include/sabi/api.h
include/sabi/events.h
include/sabi/pci.h
include/sabi/pm.h
include/sabi/resources.h
```

## Tools
The directory `tools` contains userspace programs with limited functionality. The userspace program `printns` reads an AML table (DSDT or SSDT) from disk and prints the dynamically generated namespace.
