
# Free CANopen Stack

This project is a free implementation of the CANopen protocol according to the free specification **CiA 301**. You need to register at [Can in Automation (CiA)](https://www.can-cia.org/) to retrieve your copy of the specification.

The source code is compliant to the C99 standard and you must cross-compile the source files as part of your project with the cross-compiler of your choice.

*Note: The source code of this project is independent from the CAN controller and microcontroller hardware. The hardware specific parts are called drivers. For a full featured CANopen stack, we need the drivers for hardware timer, CAN controller and a non-volatile storage media.*

## Features

**General**

- Usable with or without a real-time operating system (RTOS)
- Software timer management

**CiA 301 - CANopen application layer and communication profile**

- Unlimited number of SDO servers which supports:
  - Expedited transfers
  - Segmented transfers
  - Block transfers
- Unlimited number of TPDO and RPDOs which supports:
  - Synchronized operation
  - Asynchronous operation
  - Manufacturer specific operation
- Unlimited number of entries in object dictionary
  - Static or dynamic object dictionary
  - Data types: signed/unsigned 8/16/32-Bit integer, string, domain and user-types
- Unlimited number of parameter groups for parameter storage
- Emergency producer which supports:
  - Manufacturer specific extensions
  - Unlimited error history
- Heartbeat producer
- Unlimited Emergency consumers
- Unlimited number of Heartbeat consumers
- Network Management consumer

**CiA 305 - Layer Setting Services (LSS)**

- Baudrate configuration
- NodeId configuration

*Note: the term 'unlimited' means, that the implementation introduces no additional limit. There are technical limits, described in the specification (for example: up to 511 possible TPDOs)*

## Usage

### Integrate sourcecode in your project

Get the project repository and add:
- `driver/source` to your project source files
- `canopen/source` to your project source files
- `driver/include` to your include search path 
- `canopen/include` to your include search path 
- `canopen/config` to your include search path

*Note: in future versions, we want to remove the pre-compiler configuration file. The corresponding configuration will be possible in a more flexible way. The main goal is to get a CANopen library for a specific microcontroller, with no application specific configuration included and therefore usable in all applications.*

### Connection to the hardware

#### Driver for CAN controller

The project contains a driver layer for the required hardware interfaces.
- `driver/can/co_can_<device-name>.h` defines the CAN controller driver interface
- `driver/can/co_can_<device-name>.c` is the implementation of the specific CAN controller

The main CAN controller driver interface is a global variable, which holds pointers to the driver functions:
- `const CO_IF_DRV_CAN <DeviceName>CanDriver;`

| device name | file names        | variable name  | comment                             |
| ----------- | ----------------- | -------------- | ----------------------------------- |
| dummy       | co_can_dummy.c/h  | DummyCanDriver | Template for CAN driver development |
| sim         | co_can_sim.c/h    | SimCanDriver   | Simulated CAN controller for tests  |

#### Connect with CAN controller

Which CAN driver is used within the project is selected by including the corresponding header file and with a simple variable:

```c
#include "co_can_<device-name>.h"

const CO_IF_DRV MyDriver = {
  <DeviceName>CanDriver
};
```

#### Connect with hardware timer

The software timer management needs one hardware timer with a configured interrupt rate.
- call `COTmrService()` in the timer interrupt service handler
- call `COTmrProcess()` where you want to process the timed functions. This execution can take place in the interrupt service handler, or within a task of your RTOS.

#### Connect your storage media

Some parameter should be stored in non-volatile memory. For this feature the application needs callback functions:
- provide `COParaSave()` to store the parameter in non-volatile memory
- provide `COParaLoad()` to load the parameter from the non-voltile memory
- provide `COParaDefault()` to return the factory default values for the parameter

### Setup your CANopen node

The CANopen node is configured with some global data structures:

#### Define CANopen Dictionary

The CANopen dictionary is an array of object entries, which we can allocate statically:

~~~c
const CO_OBJ MyDir[MY_DIR_LEN] = {
    /* setup application specific dictionary, example entry: */
    { CO_KEY(0x1000, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)(0u) },
    /* : */
};
~~~

#### Define Emergency Table

The emergency code table maps the application emergency codes to the corresponding bits in the mandatory error register.

~~~c
const CO_EMCY MyEmcyTbl[MY_EMCY_TBL_LEN] = {
    /* setup application specific error codes, example entry: */
    { MY_OVER_VOLTAGE_ERROR_CODE, CO_EMCY_REG_VOLTAGE },
    /* : */
}
~~~

#### Initialize CANopen Node

Fill the specification structure with your configuration constants, memory areas and default values and call the CANopen initialization function for setting up all the internal references and structures.

~~~c
CO_NODE myNode;

void foo(void)
{
    CO_NODE_SPEC spec;

    spec.NodeId   = 1u;
    spec.Baudrate = 250000u;
    spec.Drv      = &MyDriver;
    spec.Dir      = &MyDir;
    spec.DirLen   = MY_DIR_LEN;
    spec.EmcyCode = &MyEmcyTbl;
    spec.TmrMem   = &MyTmrMem[0];
    spec.TmrNum   = 16u;
    spec.SdoBuf   = &MySdoMem[0][0];

    CONodeInit (&myNode, &spec);
}
~~~

# History

The first release of this CANopen stack is back in 2005. It is still used in many CANopen nodes from small startup companies up to big players in the automation market. Since Embedded Office sells an OEM license to Micrium to provide the CANopen stack as a part of the uC/ product line, we maintain the CANopen stack for the Flexible Safety RTOS and bare metal usage in parallel.

Some years later, now in 2020, we think it is time for a new way of software development of components where no product specific know-how is neccessary. This project is the try with the hope, that this way of software development is good for existing customers, for Embedded Offic and for all potential new users.

## Change Log

**V4.0.3**
- add: make timer tick rate configurable
- fix: compiles warning free in IAR, GCC and Keil
- fix: minor improvements in documentation

**V4.0.2**
- add: make headers compatible with C++
- fix: NMT state machine image in docs
- fix: improve quickstart example docs

**V4.0.1**
- fix: version number in source code
- fix: prepare testsuite for 64bit machines
- add: callback after synchronized RPDO distribution into object dictionary

**V4.0.0**
- First open source release.

## Roadmap

To avoid confusion, it is the best to continue with the release version numbering with the semantic scheme 'major'.'minor'.'build'. The first stable release of the open-source variant of the CANopen Stack is: **V4.0.0**

*Ideas for further development:*

- remove all pre-compiler configuration defines to allow a single library for multiple projects with different needs
- hardware independent collection of examples for demonstration purpose (exchange driver and re-compiler should be enought for usage on real target hardware)
- improve documentation of single test cases within the testsuite
- add the SDO client (rarely used, but nice to have)
Feel free to add issues with further ideas or needs!

# License

The Apache 2.0 license is suitable for commercial usage, so we think this is the best for this free component. If you have problems or concerns with this license, just contact us at Embedded Ofice. We will help you to get the legal approvals within your company.

# Code of Conduct

As everywhere in the world (especially in the internet) and at every time, we think a respectful and open minded communication is essential for peaceful and innovative developments. Please have a look in our [Code of Conduct](.github/CODE_OF_CONDUCT.md) and think about your writing before submitting.

# Contribution

## Issues and Questions

Feel free to write bug reports, questions or and feedback as issue within this github repository.

## Development Environment

The development environment for the CANopen stack takes place on our local windows machine with the free MSVC compiler and the hardware independent test framework.

*Remember: For usage in embedded systems, the source code needs to get recompiled with the cross-compiler of your choice. Good practise is the generation, testing and release of a static library with your cross-compiler for usage in your project.*

### Required Tools

Download and install these free tools:

- [Visual Studio 2019 Build Tools](https://visualstudio.microsoft.com/de/downloads) - the free C compiler for Windows, which includes the used build tools `CMake`, `Ninja` and `Make`

- [Visual Studio Code](https://code.visualstudio.com/download) - this is my editor for coding. You can use your prefered coding editor without trouble.

Perform the configuration and setup for the environment by following the nice [Tutorial](https://code.visualstudio.com/docs/cpp/config-msvc).
