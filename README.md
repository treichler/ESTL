ESTL - Embedded Systems Tiny Library
========================================

ESTL is a collection of helpful functionality related to micro controller based embedded
systems.
It is a result of several projects running on different micro controller units (MCU).
All these projects have in common the necessity of parameterization respectively system
settings, non-volatile storage, access to variables respectively registers during runtime,
data acquisition and some kind of user interface like a terminal.

Regarding the implementation of ESTL its main focus is on:

  * Pure C-code implementation
  * Small code size
  * Avoid dependencies to other libraries
  * Avoid MCU dependencies
  * Easily portable to almost any micro controller

ESTL is currently running on projects powered with these MCUs respectively MCU-cores:

  * ARM Cortex M0+, M3 and M4
  * LPC11C24, STM32G031, STM32F103, STM32F303, STM32L432


External dependencies / requirements
====================================

There are some functions in dedicated files which customize ESTL respectively create
the connection between the library and the MCUs periphery, so they have to be provided
from application side.

Hardware dependencies
---------------------
Hardware dependent functions are expected to be declared in a file called `Target.h`.
The relevant functions are all prefixed with `Target_` as an example can be seen below:

```
error_code_t Target_I2cWrite(uint8_t addr, uint8_t *data, uint16_t len);
error_code_t Target_I2cRead(uint8_t addr, uint8_t *data, uint16_t len);
```

Application specific stuff
--------------------------
Some application specific stuff is bound very close to ESTL, like library configuration
or the parameter table.
For those particular functionality there are example files provided which follow the
naming convention `*.example`.
Copy these files to your application's implementation, remove the `.example` extension
and modify them according to your requirements.

