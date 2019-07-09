===============================
Sony Spresense MicroEJ Platform
===============================
Introduction
============

This is the README file of the reference implementation of the MicroEJ platform for the Spresense board (https://developer.sony.com/develop/spresense).

The Sony Spresense SDK is highly configurable. Any change from the provided configuration must be validated.
The MicroEJ Platform for the Spresense board is delivered as a “Reference Implementation”, with all source included to rebuild the platform and generate a firmware.

The development process is to first configure Sony Spresense’s SDK enabling and/or disabling the features you need and configuring the amount of shared memory you need.
Once you have configured your SDK, you can build the platform, generate a microejapp.o containing the bytecode of your Java application. Then, you need to configure the size of Java Heap used by the application and optimize the resources used by the MicroEJ depending on the RAM available.

For more documentation on how to use MicroEJ SDK, refer to https://developer.microej.com/, in particular the following documents in the resource section:

- Device developer guide
- Application developer guide

The MicroEJ platform is configured with the following features:

- MEJ32 for ARM Cortex-M4 / GCC
- HAL.
- File System on SD Card.
- GUI on LCD Display ILI9340 connected to the SPI Pins:

   - Display SCK - Arduino Pin 13
   - Display SDO(MISO) - Arduino Pin 12
   - Display SDI(MOSI) - Arduino Pin 11
   - Display CS - Arduino Pin 10
   - Display D/C - Arduino Pin 9
   - Display RESET - Arduino Pin 8

Arborescence
============

-  README.rst: The file you are currently reading.
-  microjvm: folders containing the implementation of the low level
   API of the MicroEJ Runtime and foundations libraries. The folder is named like that because Sony Spresense's SDK has a naming convention for apps in its SDK. Since microEJ is declared as an app inside the SDK, it is necessary to follow this convention.

   - audio/: Implementation of a simple audio player application (based on the example given in the SDK) with SNI functions for an easy Java interface.
   - core/: C sources file implementation of the core of the low level APIs of MicroEJ runtime.
   - fs/: C sources implementaton of the file system low level APIs.
   - gnss/: Implementation of a simple gnss application (based on the example given in the SDK) with SNI functions for an easy Java interface.
   - microej\_async\_worker/: C source library to execute asynchronuous functions.
   - microej\_list/: C source library containing a linked list implementation.
   - osal/: C source library containing an operating system abstraction layer.
   - ui/: C sources files implementation for the Low Level APIs of the MicroUI display stack for the ILI9340 display.
   - microjvm\_main.c: C source file containing the entry point of the application.
   - inc/: Generated folder upon building your platform. Contains generated LLAPI headers.
   - lib/: Generated folder upon building your platform. Contains microej runtime library and microejapp object file containing your Java application.
   - Makefile

-  Sony-Spresense-CM4hardfp\_GCC48-configuration: Configuration files
   of the platform. Normally, you should not need to modify any of these.

   - bsp/bsp.properties: BSP property file (determine the output of the platform build).
   - display/display.properties: Display property file.
   - dropins/: Folder containing Java foundations libraries that will be integrated into the platform.
   - frontpanel/frontpanel.properties: frontpanel properties file.
   - fs/fs.properties: File system properties file.
   - microui/microui.properties: MicroUI (GUI Library) properties file.
   - mjvm/mjvm.properties: Embedded Java Virtual Machine Properties file.
   - Spresense.platform: Platform file.
   - .project: Eclipse's project file in order to easily import the platform into MicroEJ's SDK.

-  Sony-Spresense-CM4hardfp\_GCC48-fp/: Front panel folder.

   -  definitions/

      -  Sony-Spresense-CM4hardfp\_GCC48-fp.fp: The simulated display definition.
      -  widget.desc: Internal file used by the front panel. You should not touch this file.

   -  resources/: Contains the image of the front panel.

   - configs/

      - microjvm-defconfig: an importable Spresense SDK configuration file running microjvm given as example.

-  microej.mk: Specific Makefile rules to integrate MicroEJ Platform
   into Sony Spresense's SDK.
-  Application.mk: Makefile rules to declare and build a NuttX
   application as part of Spresense's SDK.
-  Makefile/: Makefile rules to declare and build a NuttX application
   as part of Spresense's SDK.
-  LibTarget.mk/: Makefile rules to declare and build a NuttX
   application as part of Spresense's SDK.
-  CHANGELOG.rst: CHANGELOG file


Setup & Versions
================

- The Sony Spresense SDK is made to work under a shell terminal; either a native Ubuntu, a Cygwin or using Windows Subsystem Linux (WSL).

- The Sony Spresense SDK version required is v1.3.0.

- For Windows 10 user or higher, we strongly suggest using WSL. How to install WSL:    https://docs.microsoft.com/en-us/windows/wsl/install-win10

- Installation gcc-arm-none-eabi v7.3.1 is required. You can find it at https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

- MicroEJ XPFP packs version is 7.11.0, Pack FS 5.0.0, Pack UI 12.0.0, Pack HAL 2.0.1.

   - Download MEJ32 architecture 7.11 https://repository.microej.com/architectures/com/microej/architecture/CM4/CM4hardfp_GCC48/flopi4G25/7.11.0/flopi4G25-7.11.0-eval.xpf

    - Download Pack FS 5.0 https://repository.microej.com/architectures/com/microej/architecture/generic/fs/fs-pack/5.0.0/fs-5.0.0.xpfp

    - Download Pack UI 12.0 https://repository.microej.com/architectures/com/microej/architecture/CM4/CM4hardfp_GCC48/flopi4G25-ui-pack/12.0.0/flopi4G25UI-12.0.0.xpfp

    - Download Pack HAL 2.0.1 https://repository.microej.com/architectures/com/microej/architecture/generic/hal/hal-pack/2.0.1/hal-2.0.1.xpfp

    - You can install them in your SDK by going into Windows -> Preferences -> MicroEJ -> Architectures- > Import...

- MicroEJ SDK Version 19.05 or higher. You can download it at https://developer.microej.com/packages/SDK/19.05/.


Step-by-steps Installation
==========================

- Step 1: Execute the following line to clone Spresense's SDK, checked out on the correct version (v1.3.0)
    - :code:`$ git clone https://github.com/sonydevworld/spresense.git`
    - :code:`$ cd spresense/`
    - :code:`$ git submodule update --init --recursive`
    - :code:`$ git checkout --recurse-submodules v1.3.0`
    - :code:`$ git submodule add https://github.com/MicroEJ/Platform-Sony-Spresense.git MicroEJ`

You can also git clone the MicroEJ folder outside and create a symlink in the spresense folder if you do not want to add a submodule
    - :code:`$ cd ..`
    - :code:`$ git clone https://github.com/MicroEJ/Platform-Sony-Spresense.git MicroEJ`
    - :code:`$ cd spresense`
    - :code:`$ ln -s ../MicroEJ`

-  Step 2 : Install MicroEJ SDK (version 19.05).

   -  Step 2.1: Download the SDK evaluation license from http://license.microej.com/ or get a production license. Verifiy that the version of each pack correspond to the versions in "Setup and Versions".
   -  Step 2.2 : Install the architecure. Go to Windows -> Preferences -> MicroEJ -> Architecture -> Import...
   -  Step 2.3: File -> Import... -> General -> Existing Projects into Workspace -> Select root directory -> Point to the MicroEJ directory.
   -  Step 2.4: Open the Spresense.platform file. You may configure what is included in the MicroEJ platform in the Content tab by selecting content (or not).
   -  Step 2.5: In the Overview tab, click on build platform.

-  Step 3: Create a Java Application.

   -  Step 3.1: In MicroEJ SDK either open an existing app or create a new
      MicroEJ Application with a simple hello world for now.
   -  Step 3.2: Select the appliaction folder. Then create a new MicroEJ Launcher.
   -  Step 3.2: In the tab Execution, check that the selected Platform is correct. Then select "Execute on Simulator". This will run the Java application in the simulator.
   -  Step 3.3: In Run -> Run Configuration... -> Tab execution,  check "Execute on Device". This will build a microejapp.o that contains your application.

-  Step 4: Make sure the configuration of your SDK and NuttX is correct. For the NuttX configuration use the release configuration and enable the option :literal:`CONFIG_SYSTEMTICK_HOOK=y`.  :literal:`CONFIG_LIBM` must be disabled in the nuttx configuration.

   - MicroEJ Platform require a LCD driver in the Spresense SDK configuration. Without a LCD Driver compiled in the BSP, symbols may be missing.

   - Configure the SDK with the functionality needed and then add / remove the following:

      - Either using Kconfig (recommended) instead of editing config file manually:
      - :code:`$ tools/config.py -k -m` then select :literal:`RTOS Features -> Clocks and Timers -> System timer hook`
      - For the SDK configuration, add :literal:`CONFIG_MICROJVM=y` and :literal:`CONFIG_LIBM_NEWLIB=y`.
      - Using Kconfig:
      - :code:`$ tools/config.py -m`then select :literal:`Library Routines -> Newlib Math library` and :literal:`Microjvm -> microjvm runtime`. It is also possible to select :literal:`MicroEJ Audio Library LLAPIs` and :literal:`MicroEJ gnss library LLAPIs`

   - Step 4.1:
       - :code:`$ cd spresense/sdk`.
       - :code:`$ tools/config.py -k -m` to configure the kernel using Kconfig
       - :code:`$ tools/config.py -k release` to configure the kernel using in release mode. If you do so you will need to manually (either via config or via editing the file in spresense/nuttx/.config) to add the property CONFIG_SYSTEMTICK_HOOK=y

   - Step 4.2 : Then configure the spresense SDK; Enable the properties you need, configure your shared memoery (if any), lcd screen (if any). Then add the property CONFIG_MICROJVM=y either manually or with Kconfig.

         - If you want to boot directly on the Java runtime change the entry point to "microjvm_main". If so, make sure the board ioctl init function :code:`boardctl(BOARDIOC_INIT, 0);` is called (it should be the case by default).

   - Step 4.3: Type make buildkernel to compile NuttX.
   - Step 4.4: Type make to compile your firmware. You can flash it on board following the instruction on Sony Spresense's website https://developer.sony.com/develop/spresense/developer-tools/get-started-using-nuttx/set-up-the-nuttx-environment . There also useful information in the ReadMe of the repository https://github.com/sonydevworld/spresense

   - Step 5: The default configuration should be to define the entry point as "microjvm_main" in the .config file. You can use NSH as entry point, in which case you need to type the command :code:`microvjm` in the NSH command line. I highly recommand using the default configuration to avoid loading the NSH library for nothing and to avoid a manual command to start your application.

Tasks running
==================================================
When running MicroEJ some tasks will be added to the RTOS.
These are the threads running on top of the existing tasks / threads from NuttX & Spresense SDK :

- UI Task: priority 100, stack size 1024 bytes.
- VM Task: priority 100, stack size 4096 bytes (can be changed using Kconfig, these are the default values).
- pthread GNSS: stack size 512 bytes.
- pthread pool for the file system implementation: 4 threads with 256 bytes stack (configurable).

Some audio tasks from the SDK:
Audio player : priority 150, stack size 3*1024 bytes.
Audio renderer : priority 200, stack size 3*1024 bytes.

Memory Map
==========

Code is stored in flash,executed in RAM.

Memory zone : ram (rwx) :
ORIGIN = 0x0d000000, LENGTH = 1536K, END\_MEMORY=0xd180000

PlatformHeap, Java application Heap, Java Immortals are allocated in the bss zone.

C-heap is the stack of the :code:`Idle` thread, defined relatively to end of the
bss zone. Increasing the size of compiled code (text zone), data (data,
rodata), size of diplay, Java heap(bss) or shared memory (defined
in .config) can break your application on board if not enough memory is available.

Several symptoms may appear, depending on which memory zone is impacted
and the memory allocation you are doing, such as :

   - A failure to create task resulting in hardfault in os_startup.
   - Code executing stopping without any uart trace.
   - Impossibility to create a pthread (or a task), with a return code of 12 corresponding to the errno ENOMEM.

If the board suddenly stops working, this is the FIRST thing you should
check.

Additional Tips
==================================================

- The first time you flash the board you will be directed to download a zip containing a firmware.
- When you flash a new board do not forget to flash the bootloade (read https://developer.sony.com/develop/spresense/developer-tools/get-started-using-nuttx/nuttx-developer-guide#_flashing_bootloader).
- The linker file given in the NuttX repository may not have the correct name. This result in the following error :literal:`arm-none-eabi-ld: cannot open linker script file /mnt/c/msys64/home/acolleux/spresense/sdk/../nuttx/configs/cxd56evb/scripts/gnu-elf.ld: No such file or directory`. To solve this you can simply create a symlink with the expected name

    :code:`$ cd spresense//nuttx/configs/cxd56evb/scripts/`

    :code:`$ ln -s ramconfig.ld gnu-elf.ld`

- There are lots of examples in the SDK! To better understand how to configure your Spresense SDK, start by using the provided example.

- The Spresense SDK is highly configurable. The configuration of the SDK will have a great impact on the amount of RAM available and the functionnalities available. We recommend first configuring your SDK and then starting the integration of your Java application on board. You will have to configure the Java heap and Image heap you use depending on the size of your application and the available ram.

- If some hardfaults occurs, the board do not start or suddenly stops (without any UART trace), asserts fails, it is often due to a lack of RAM. This probably means that your Java application uses too much resources.
  When you use microEJ you add areas in RAM with a fixed size in your Run Configuration... . A wrong size given the Run Configuration... of your Java application can cause problems on-board.
  This is why it is critical to use the simulator to find the minimum resources you can use for your application in RAM and also to correctly configure your Spresense SDK during your development process, if you encounter a bug as previously described the FIRST thing you need to check is how much RAM you use in the Java Heap, Image Heap in your MicroEJ Run Configuration... and how much memory you use in the SDK and-application code.

- When installing kconfig front end for the Spresense SDK setup you may encouter this problem :

If your system has gperf 3.0.4 or earlier, you may safely skip this chapter. gperf 3.1 (released on 5th January of 2017) changed the type used as length argument in generated functions from unsigned int to size_t. This will cause your build to fail with following error message:

:literal:`CC     libkconfig_parser_la-yconf.lo`
:literal:`In file included from yconf.c:234:0:`
:literal:`hconf.gperf:141:1: error: conflicting types for 'kconf_id_lookup'`
:literal:`hconf.gperf:12:31: note: previous declaration of 'kconf_id_lookup' was here`
:literal:`static const struct kconf_id *kconf_id_lookup(register const char *str, register unsigned int len);`

:literal:`make[3]: *** [Makefile:456: libkconfig_parser_la-yconf.lo] Error 1`
:literal:`make[2]: *** [Makefile:350: all] Error 2`
:literal:`make[1]: *** [Makefile:334: all-recursive] Error 1`
:literal:`make: *** [Makefile:385: all-recursive] Error 1`

- The procedure to fix is below:

:code:`$ curl -O https://gist.githubusercontent.com/KamilSzczygiel/d16a5d88075939578f7bd8fadd0907aa/raw/1928495cfb6a6141365d545a23d66203222d28c0/kconfig-frontends.patch`

:code:`$ patch -p1 -i kconfig-frontends.patch`

:code:`$ autoreconf -fi`

- To use the Audio and Gnss library in Java you must add to the configuration CONFIG_MICROEJ_AUDIOPLAYER and CONFIG_MICROEJ_GNSS respectively.



License
==================================================

*Copyright 2019 Sony Corp. All rights reserved.*

*This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.*

*Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.*
