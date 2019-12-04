..
    Copyright 2019 MicroEJ Corp. All rights reserved.
    This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
    Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
..

==============================
Sony Spresense Platform v1.0.5
==============================

Build Environment
==================

The platform has been compiled and tested with arm-none-eabi-gcc v7.3.1.

Module version
==============

Architecture and pack
---------------------

- MicroEj architecture version 7.11.0
- Pack UI version 12.0.0
- Pack FS version 5.0.0
- Pack HAL version 2.0.1

Tools
-----

- MicroEJ SDK 19.05

Libraries
---------

- com.microej.audio library  v1.0.0
- com.microej.gnss librarby v1.0.0

Known issues
============

File System
-----------
You cannot use function :code:`chmod()` and :code:`utime()`, meaning you cannot modify permission or modified time.

You cannot open multiple streams on the same file.

RAM
---
Code is executed in RAM, therefore increasing the size of your application too much will cause trouble (see readme for more information).

Validation
==========
This platform has passed the following tests :

- MJVM Portage Validation

- Benchmark UI.

- UI Test Suite.

- FileSystem Test Suite.

Known Issue
=============
Using Sony Spresense SDK version superior to 1.3.1 may cause some issues with audio library and volume manipulation.

License
=======
*Copyright 2019 Sony Corp. All rights reserved.*

*This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.*

*Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.*
