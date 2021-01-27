Collect Debug Information {#collect_debug_info}
===============================================

## Brief Description

The script `collect_debug_info.py` provides a relatively easy way to dump memory contents and the trace stream of the executed instructions as well.
The script can be used either in an "online" or "offline" mode depending on whether the memory contents come from a live capture or not.
The script runs both on Windows and Linux provided that a 32bit version of 2.7 Python is installed.
The script can be used either standalone or via the corresponding launcher in SmartSnippets Studio.

![Caption text](../../utilities/python_scripts/collect_debug_info/doc/launchers.png "Launcher triggering the script")

## Prerequisites

An arm-none-eabi-gdb version with Python support is required.

### A. Default way

Dialog's Development Environment provides it. It is called `arm-none-eabi-gdb-py`
(or `arm-none-eabi-gdb-py.exe` in Windows). In order to use it, the following are needed:

1.   __32bit version of 2.7 Python__  [sources](https://www.python.org/downloads).
2.   In Windows the following additional steps should be made:
     *  In the installer choose the option "Add python.exe to Path"

     ![Caption text](../..//utilities/python_scripts/collect_debug_info/doc/python_default_path.png "Choose the option 'Add python.exe to Path'")
     \n
     * Add a new environmental variable `PYTHONPATH` in Start Menu --> Computer icon --> Properties --> Advanced system settings --> Advanced tab --> Environment Variables
     \n
     \n
     ![Caption text](../../utilities/python_scripts/collect_debug_info/doc/python_env_var.png "Add New variable PYTHONPATH")
     \n
     * Just for convenience of running the script in Windows it is advised to install [git-bash](https://git-for-windows.github.io)
     (part of git for Windows).

### B. Alternative way

1.  Recompile the provided arm-none-eabi-gdb as it's not compiled with python support.
    To do so the following are needed:
    *  [GDB sources](http://ftp.gnu.org/gnu/gdb)
    *  Python headers
    *  [The GNU Documentation System](https://www.gnu.org/software/texinfo)

    E.g. For Ubuntu python-dev and texinfo need to be installed:
    *  `apt-get install python-dev`\n
    *  `apt-get install texinfo`\n

2.  Place `collect_debug_info.py` and `gdb_custom_cmds.py` on the same path.

#### Compiling GDB

1. Create a folder e.g /opt/gdb_build. This will be the location of the compiled gdb.\n
   `mkdir /opt/gdb_build`
2. Extract gdb.\n
   `tar xvfz gdb-7.11.tar.gz`
3. Change directory to gdb-7.11\n
   `cd gdb-7.11`

4. Execute:
   *  `./configure --target=arm-none-eabi --prefix=/opt/gdb_build -disable-interwork --enable-multilib --disable-werror --with-python`
   *  `make`
   *  `make install`


## Running the script

        usage: ./collect_debug_info.py -f <elf_file> -b <mem_snapshot_file> - <toolchain_path> -x <registers_xml_file>, --sram_start <> --sram_stop <>

        e.g 1 ./collect_debug_info.py -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f application.elf -x SVD.xml
        Dumps RAM contents, symbols are provided by the elf file, peripheral registers by the -x argument.
        Toolchain path is provided by the -t argument.

        e.g 2 ./collect_debug_info.py -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f application.elf -b dump_memory.ihex --sram_start 0xXXXXXXXX --sram_stop 0xYYYYYYYY
        Dumps RAM contents, symbols are provided by the elf file, memory contents will be read using the provided ihex file and the sram memory area.
        Toolchain path is provided by the -t argument.

        e.g 3 ./collect_debug_info.py -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f application.elf -b dump_memory.bin --sram_start 0xXXXXXXXX --sram_stop 0xYYYYYYYY
        Dumps RAM contents, symbols are provided by the elf file, memory contents will be read using the provided raw bin file restored at address given with --sram_start
        Toolchain path is provided by the -t argument.

The script creates an artifact-folder with a timestamp e.g `dump_20160328-1459165500` for each execution-run.
The folder contains the following:

        1. symbols_20160328-1459165500.list                [ The list of symbols ]
        2. gdb_cmds_20160328-1459165500.log                [ Our dump ]
        3. gdb_cmds_20160328-1459165500.gdb                [ The executed gdb commands ]
        4. online_memory_snapshot_20160328-1459165500.ihex [ The memory snapshot during the online run ]
        5. pxp_reporter.elf                                [ The elf file provided to the script ]
        6. pxp_reporter.map                                [ The generated map file ]

All dumps are stored in a folder called `debug_dumps` which is created on the same path collect_debug_info.py
is executed.

## Using the launcher from SmartSnippets Studio

1. Start the debugging procedure from SmartSnippets Studio. Then pause it at the moment of interest.
2. Launch collect_debug_info.
> Note: The launcher opens a file selection window-prompt for entering the corresponding xml
> register file found in `config/embsys/Dialog_Semiconductor/` path.
3. Artifacts are stored in `utilities/python_scripts/collect_debug_info/debug_dumps`

> Note:
> Windows launcher expects Python interpreter in `C:\Python27\python.exe`


## Using the script in online mode

1. Start the debugging procedure from SmartSnippets Studio. Then pause it at the moment of interest.
2. Run e.g.
   * `./collect_debug_info.py -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f my_application.elf -x SVD.xml` [on Linux or git-bash on Windows]
   * `python collect_debug_info.py -t /c/DiaSemi/SmartSnippetsStudio/GCC/gcc-arm-none-eabi-7-2017-q4-major/bin -f my_application.elf -x SVD.xml` [cmd on Windows]
3. The results are found in the `debug_dumps/dump_XXXXXXXX-XXXXXXXXXX/gdb_cmds_XXXXXXXX-XXXXXXXXXX.log` file.

## Using the script in offline mode

Given a memory snapshot in ihex format and the corresponding elf file e.g dump_memory.ihex and my_application.elf,
respectively:
1. Close SmartSnippets Studio if open.
2. Start JLink GDB Server manually:
   * On Windows launch the J-Link GDB Server application. [ All Programs->SEGGER->J-Link VX.XXx->J-Link GDB Server]
   * On Linux type: `/opt/SEGGER/JLink/JLinkGDBServer -noir -vd -device Cortex-M?? -endian little -if SWD -speed 0 -localhostonly`
3. Run e.g.
   * `./collect_debug_info.py -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin  -f my_application.elf -b dump_memory.ihex  --sram_start 0xXXXXXXXX --sram_stop 0xYYYYYYYY` [on Linux or git-bash on Windows]
   * `python collect_debug_info.py -t /c/DiaSemi/SmartSnippetsStudio/GCC/gcc-arm-none-eabi-7-2017-q4-major/bin -f my_application.elf -b dump_memory.ihex  --sram_start 0xXXXXXXXX --sram_stop 0xYYYYYYYY` [cmd on Windows]
4. The results are found in the `debug_dumps/dump_XXXXXXXX-XXXXXXXXXX/gdb_cmds_XXXXXXXX-XXXXXXXXXX.log` file.

## Generating a memory snapshot in ihex format

A. Linux

  1. Start the debugging procedure from SmartSnippets Studio. Then pause it at the moment of interest.
> Note: An alternative way is to exit SmartSnippets Studio and manually start JLink GDB Server as shown above.
  2. Run `arm-none-eabi-gdb`.
  3. On the gdb prompt type the following commands:\n

     `(gdb) target remote :2331`\n
     `(gdb) dump ihex memory dump_memory.ihex 0xXXXXXXXX 0xYYYYYYYY`\n
     `(gdb) quit`\n

  4. dump_memory.ihex is created on the same path arm-none-eabi-gdb was executed.

B. Windows

  1. Start the debugging procedure from SmartSnippets Studio. Then pause it at the moment of interest.
> Note: An alternative way is to exit SmartSnippets Studio and manually start JLink GDB Server as shown above.
  2. Run `arm-none-eabi-gdb.exe`.
     This is located in `C:\Program Files (x86)\Dialog Semiconductor\SmartSnippetsXXX\...\arm-none-eabi\bin`
  3. On the gdb prompt type the following commands:\n

     `(gdb) target remote :2331`\n
     `(gdb) dump ihex memory dump_memory.ihex 0xXXXXXXXX 0xYYYYYYYY`\n
     `(gdb) quit`\n

  4. dump_memory.ihex is created on the same path arm-none-eabi-gdb.exe was executed.

## Sections of the dump

### ARM REGISTERS


This section starts from
__ARM_REGISTERS_START__ marker until
__ARM_REGISTERS_STOP__.

E.g

        __ARM_REGISTERS_START__
        r0             0xffffffff	4294967295
        r1             0xffffffff	4294967295
        r2             0xffffffff	4294967295
        r3             0xffffffff	4294967295
        r4             0xffffffff	4294967295
        r5             0xffffffff	4294967295
        r6             0xffffffff	4294967295
        r7             0xffffffff	4294967295
        r8             0xffffffff	4294967295
        r9             0xffffffff	4294967295
        r10            0xffffffff	4294967295
        r11            0xffffffff	4294967295
        r12            0xffffffff	4294967295
        sp             0x7fc8000	0x7fc8000
        lr             0xffffffff	4294967295
        pc             0x80002a4	0x80002a4 <Reset_Handler>
        xpsr           0xc1000000	3238002688
        MSP            0x7fc8000	133988352
        PSP            0xfffffffc	4294967292
        PRIMASK        0x0	0
        BASEPRI        0x0	0
        FAULTMASK      0x0	0
        CONTROL        0x0	0
        __ARM_REGISTERS_END__


### CURRENT_TASK_BACKTRACE


This section starts from
__CURRENT_TASK_BACKTRACE_START__ marker until
__CURRENT_TASK_BACKTRACE_STOP__.

It shows the backtrace of the current task. For example:

        __CURRENT_TASK_BACKTRACE_START__
        #0  0x0800e532 in __WFI () at /home/user/work/SDK/sdk/bsp/include/core_cmInstr.h:342
        #1  prvSystemSleep (xExpectedIdleTime=12) at /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c:406
        #2  0x0800fa2a in prvIdleTask (pvParameters=<optimized out>) at /home/user/work/SDK/sdk/free_rtos/tasks.c:2792
        #3  0x0800e374 in vPortStartFirstTask () at /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c:187
        __CURRENT_TASK_BACKTRACE_STOP__


### ALL_TASKS_BACKTRACE_START


This section starts from
__ALL_TASKS_BACKTRACE_START__ marker until
__ALL_TASKS_BACKTRACE_STOP__.


It shows the backtrace of all tasks. For example:

        __ALL_TASKS_BACKTRACE_START__
        [New Thread 536900576]
        [New Thread 536885000]
        [New Thread 536905488]
        [New Thread 536912872]
        [New Thread 536911992]
        ...
        Thread 4 (Thread 536905488):
        #0  0x000117a4 in vPortExitCritical () at /home/user/work/SDK/sdk/free_rtos/portable/GCC/DA1469x/../../../../free_rtos/include/../../free_rtos/portable/GCC/DA1469x/portmacro.h:276
        #1  0x000135f2 in xTaskNotifyWait (ulBitsToClearOnEntry=ulBitsToClearOnEntry@entry=0, ulBitsToClearOnExit=ulBitsToClearOnExit@entry=4294967295, pulNotificationValue=pulNotificationValue@entry=0x200086dc <ucHeap+6600>, xTicksToWait=xTicksToWait@entry=4294967295) at /home/user/work/SDK/sdk/free_rtos/tasks.c:4561
        #2  0x0000ea58 in ad_ble_task (pvParameters=<optimized out>) at /home/user/work/SDK/sdk/interfaces/ble/adapter/src/ad_ble.c:591
        #3  0x0001163c in vPortFree (pv=0x200036d8 <xSuspendedTaskList>) at /home/user/work/SDK/sdk/free_rtos/portable/MemMang/heap_4.c:296
        #4  0x80000070 in ?? ()
        Backtrace stopped: previous frame identical to this frame (corrupt stack?)

        Thread 3 (Thread 536885000):
        #0  execute_active_wfi () at /home/user/work/SDK/sdk/bsp/system/sys_man/sys_power_mgr_da1469x.c:744
        #1  0x2000144c in pm_sleep_enter (low_power_periods=<optimized out>) at /home/user/work/SDK/sdk/bsp/system/sys_man/sys_power_mgr_da1469x.c:1519
        #2  0x00011954 in prvSystemSleep (xExpectedIdleTime=<optimized out>) at /home/user/work/SDK/sdk/free_rtos/portable/GCC/DA1469x/port.c:614
        #3  0x00012f58 in prvIdleTask (pvParameters=<optimized out>) at /home/user/work/SDK/sdk/free_rtos/tasks.c:3341
        #4  0x0001163c in vPortFree (pv=0x0 <snc_set_field_ucode_get_context>) at /home/user/work/SDK/sdk/free_rtos/portable/MemMang/heap_4.c:296
        #5  0x80000070 in ?? ()
        Backtrace stopped: previous frame identical to this frame (corrupt stack?)

        Thread 2 (Thread 536900576):
        #0  execute_active_wfi () at /home/user/work/SDK/sdk/bsp/system/sys_man/sys_power_mgr_da1469x.c:744
        #1  0x2000144c in pm_sleep_enter (low_power_periods=<optimized out>) at /home/user/work/SDK/sdk/bsp/system/sys_man/sys_power_mgr_da1469x.c:1519
        #2  0x00011954 in prvSystemSleep (xExpectedIdleTime=<optimized out>) at /home/user/work/SDK/sdk/free_rtos/portable/GCC/DA1469x/port.c:614
        #3  0x00012f58 in prvIdleTask (pvParameters=<optimized out>) at /home/user/work/SDK/sdk/free_rtos/tasks.c:3341
        #4  0x0001163c in vPortFree (pv=0x0 <snc_set_field_ucode_get_context>) at /home/user/work/SDK/sdk/free_rtos/portable/MemMang/heap_4.c:296
        #5  0x80000070 in ?? ()
        Backtrace stopped: previous frame identical to this frame (corrupt stack?)
        __ALL_TASKS_BACKTRACE_STOP__


> Note: The above trace is applicable as long as the logs are collected using the SmartSnippets Studio
        equipped with the FreeRTOS task trace capability.

### SYMBOLS


This section starts from
__SYMBOLS_START__ marker until
__SYMBOL_STOP__.

These are memory contents available in the current execution context, in a readable format.
For example the symbol called `ad_msg_wqueue` is located at 0x07fd1668 and has a size of 64 bytes.

        __SYMBOL__ 0x07fd1668 64 ad_msg_wqueue
        $103 = {
          queue = {{
              rsp_op = AD_BLE_OP_CMP_EVT,
              cmd_op = AD_BLE_OP_CMP_EVT,
              cb = 0x80065fd <ble_adapter_cmp_evt_init>,
              param = 0x7fd36a8 <ucHeap+7420>
            }, {
              rsp_op = AD_BLE_OP_CMP_EVT,
              cmd_op = AD_BLE_OP_CMP_EVT,
              cb = 0x0,
              param = 0x0
            }, {
              rsp_op = AD_BLE_OP_CMP_EVT,
              cmd_op = AD_BLE_OP_CMP_EVT,
              cb = 0x0,
              param = 0x0
            }, {
              rsp_op = AD_BLE_OP_CMP_EVT,
              cmd_op = AD_BLE_OP_CMP_EVT,
              cb = 0x0,
              param = 0x0
            }, {
              rsp_op = AD_BLE_OP_CMP_EVT,
              cmd_op = AD_BLE_OP_CMP_EVT,
              cb = 0x0,
              param = 0x0
            }},
          len = 0 '\000'
        }


### PERIPHERAL_REGISTERS

This section starts from
__PERIPHERAL_REGISTERS_START__ marker until
__PERIPHERAL_REGISTERS_STOP__.

It contains all the peripheral register values in groups.
The register names are read from the corresponding xml file, for example DA14681-01.xml.

E.g

        __PERIPHERAL_REGISTERS_START__
        __PERIPHERAL__: NVIC
        __REG__: ISER Reading from address 0xE000E100 (Data = 0x42080003)
        __REG__: ICER Reading from address 0xE000E180 (Data = 0x42080003)
        __REG__: ISPR Reading from address 0xE000E200 (Data = 0x00000000)
        __REG__: ICPR Reading from address 0xE000E280 (Data = 0x00000000)
        __REG__: IPR0 Reading from address 0xE000E400 (Data = 0x40404040)
        __REG__: IPR1 Reading from address 0xE000E404 (Data = 0x00000000)
        __REG__: IPR2 Reading from address 0xE000E408 (Data = 0x0000C0C0)
        __REG__: IPR3 Reading from address 0xE000E40C (Data = 0x00000000)
        __REG__: IPR4 Reading from address 0xE000E410 (Data = 0xC0000000)
        __REG__: IPR5 Reading from address 0xE000E414 (Data = 0x80000000)
        __REG__: IPR6 Reading from address 0xE000E418 (Data = 0x0000C080)
        __REG__: IPR7 Reading from address 0xE000E41C (Data = 0x00000000)
        __PERIPHERAL__: SCB
        __REG__: CPUID Reading from address 0xE000ED00 (Data = 0x410CC200)
        __REG__: ICSR Reading from address 0xE000ED04 (Data = 0x00000000)
        __REG__: AIRCR Reading from address 0xE000ED0C (Data = 0xFA050000)
        __REG__: SCR Reading from address 0xE000ED10 (Data = 0x00000004)
        __REG__: CCR Reading from address 0xE000ED14 (Data = 0x00000208)
        __REG__: SHPR2 Reading from address 0xE000ED1C (Data = 0x00000000)
        __REG__: SHPR3 Reading from address 0xE000ED20 (Data = 0xC0C00000)
        __PERIPHERAL__: SysTick
        __REG__: CTRL Reading from address 0xE000E010 (Data = 0x00000000)
        __REG__: LOAD Reading from address 0xE000E014 (Data = 0x00FFFFFF)
        __REG__: VAL Reading from address 0xE000E018 (Data = 0x00FFFFFF)
        __REG__: CALIB Reading from address 0xE000E01C (Data = 0x0000270F)
        ...
        __PERIPHERAL_REGISTERS_STOP__


### CURRENT_FREE_RTOS_TASK


This section starts from
__CURRENT_FREE_RTOS_TASK_START__ marker until
__CURRENT_FREE_RTOS_TASK_STOP__.

It contains the name of the current FreeRTOS task. For example:

        __CURRENT_FREE_RTOS_TASK_START__
        $222 = "IDLE"
        __CURRENT_FREE_RTOS_TASK_STOP__


### FREE_RTOS_LISTS

This section starts from
__EXPAND_FREE_RTOS_LIST_STARTS__ marker until
__EXPAND_FREE_RTOS_LIST_STOP__.

The following FreeRTOS lists are expanded:

        pxReadyTasksLists
        xDelayedTaskList1
        xDelayedTaskList2
        pxDelayedTaskList
        pxOverflowDelayedTaskList
        xPendingReadyList
        xSuspendedTaskList


In addition, a sanity check takes place so that the number of elements matches uxNumberOfItems.
If it doesn't, a corruption is detected and logged. So it's a good practice to search for the "List corruption" keyword in the dump.

Non corrupted example: Expanding xSuspendedTaskList list which contains 5 elements (tasks).


        __LIST_SYMBOL__ xSuspendedTaskList
        node@0x7fd25cc <ucHeap+3104>: {
          xItemValue = 0,
          pxNext = 0x7fd27c4 <ucHeap+3608>,
          pxPrevious = 0x7fd1800 <xSuspendedTaskList+8>,
          pvOwner = 0x7fd25c8 <ucHeap+3100>,
          pvContainer = 0x7fd17f8 <xSuspendedTaskList>
        } #1 -----------------------------------------------------------------------> First element of the list
        pvOwner@0x7fd25c8 <ucHeap+3100>: { -----------------------------------------> Corresponding owner casted as (TCB_t *)
          pxTopOfStack = 0x7fd2548 <ucHeap+2972>,
          xGenericListItem = {
            xItemValue = 0,
            pxNext = 0x7fd27c4 <ucHeap+3608>,
            pxPrevious = 0x7fd1800 <xSuspendedTaskList+8>,
            pvOwner = 0x7fd25c8 <ucHeap+3100>,
            pvContainer = 0x7fd17f8 <xSuspendedTaskList>
          },
          xEventListItem = {
            xItemValue = 3,
            pxNext = 0x0,
            pxPrevious = 0x0,
            pvOwner = 0x7fd25c8 <ucHeap+3100>,
            pvContainer = 0x0
          },
          uxPriority = 4,
          pxStack = 0x7fd2430 <ucHeap+2692>,
          pcTaskName = "USBC", ------------------------------------------------------> Corresponding task name
          uxTCBNumber = 4,
          uxTaskNumber = 0,
          uxBasePriority = 4,
          uxMutexesHeld = 0,
          ulNotifiedValue = 0,
          eNotifyState = eWaitingNotification
        }
        PC @ 0800cc1a ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c, line 273
        LR @ 0800e3c3 ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/tasks.c, line 4074
        --------------------------------------------------------------------
        node@0x7fd27c4 <ucHeap+3608>: {
          xItemValue = 0,
          pxNext = 0x7fd36dc <ucHeap+7472>,
          pxPrevious = 0x7fd25cc <ucHeap+3104>,
          pvOwner = 0x7fd27c0 <ucHeap+3604>,
          pvContainer = 0x7fd17f8 <xSuspendedTaskList>
        } #2
        pvOwner@0x7fd27c0 <ucHeap+3604>: {
          pxTopOfStack = 0x7fd2730 <ucHeap+3460>,
          xGenericListItem = {
            xItemValue = 0,
            pxNext = 0x7fd36dc <ucHeap+7472>,
            pxPrevious = 0x7fd25cc <ucHeap+3104>,
            pvOwner = 0x7fd27c0 <ucHeap+3604>,
            pvContainer = 0x7fd17f8 <xSuspendedTaskList>
          },
          xEventListItem = {
            xItemValue = 3,
            pxNext = 0x0,
            pxPrevious = 0x0,
            pvOwner = 0x7fd27c0 <ucHeap+3604>,
            pvContainer = 0x0
          },
          uxPriority = 4,
          pxStack = 0x7fd2628 <ucHeap+3196>,
          pcTaskName = "USBF",
          uxTCBNumber = 5,
          uxTaskNumber = 0,
          uxBasePriority = 4,
          uxMutexesHeld = 0,
          ulNotifiedValue = 0,
          eNotifyState = eWaitingNotification
        }
        PC @ 0800cc1a ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c, line 273
        LR @ 0800e3c3 ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/tasks.c, line 4074
        --------------------------------------------------------------------
        node@0x7fd36dc <ucHeap+7472>: {
          xItemValue = 0,
          pxNext = 0x7fd3944 <ucHeap+8088>,
          pxPrevious = 0x7fd27c4 <ucHeap+3608>,
          pvOwner = 0x7fd36d8 <ucHeap+7468>,
          pvContainer = 0x7fd17f8 <xSuspendedTaskList>
        } #3
        pvOwner@0x7fd36d8 <ucHeap+7468>: {
          pxTopOfStack = 0x7fd3648 <ucHeap+7324>,
          xGenericListItem = {
            xItemValue = 0,
            pxNext = 0x7fd3944 <ucHeap+8088>,
            pxPrevious = 0x7fd27c4 <ucHeap+3608>,
            pvOwner = 0x7fd36d8 <ucHeap+7468>,
            pvContainer = 0x7fd17f8 <xSuspendedTaskList>
          },
          xEventListItem = {
            xItemValue = 5,
            pxNext = 0x0,
            pxPrevious = 0x0,
            pvOwner = 0x7fd36d8 <ucHeap+7468>,
            pvContainer = 0x0
          },
          uxPriority = 2,
          pxStack = 0x7fd34d0 <ucHeap+6948>,
          pcTaskName = "bleM",
          uxTCBNumber = 7,
          uxTaskNumber = 0,
          uxBasePriority = 2,
          uxMutexesHeld = 0,
          ulNotifiedValue = 0,
          eNotifyState = eWaitingNotification
        }
        PC @ 0800cc1a ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c, line 273
        LR @ 0800e3c3 ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/tasks.c, line 4074
        --------------------------------------------------------------------
        node@0x7fd3944 <ucHeap+8088>: {
          xItemValue = 0,
          pxNext = 0x7fd328c <ucHeap+6368>,
          pxPrevious = 0x7fd36dc <ucHeap+7472>,
          pvOwner = 0x7fd3940 <ucHeap+8084>,
          pvContainer = 0x7fd17f8 <xSuspendedTaskList>
        } #4
        pvOwner@0x7fd3940 <ucHeap+8084>: {
          pxTopOfStack = 0x7fd3860 <ucHeap+7860>,
          xGenericListItem = {
            xItemValue = 0,
            pxNext = 0x7fd328c <ucHeap+6368>,
            pxPrevious = 0x7fd36dc <ucHeap+7472>,
            pvOwner = 0x7fd3940 <ucHeap+8084>,
            pvContainer = 0x7fd17f8 <xSuspendedTaskList>
          },
          xEventListItem = {
            xItemValue = 6,
            pxNext = 0x0,
            pxPrevious = 0x0,
            pvOwner = 0x7fd3940 <ucHeap+8084>,
            pvContainer = 0x0
          },
          uxPriority = 1,
          pxStack = 0x7fd3738 <ucHeap+7564>,
          pcTaskName = "PXP ",
          uxTCBNumber = 8,
          uxTaskNumber = 0,
          uxBasePriority = 1,
          uxMutexesHeld = 0,
          ulNotifiedValue = 0,
          eNotifyState = eWaitingNotification
        }
        PC @ 0800cc1a ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c, line 273
        LR @ 0800e3c3 ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/tasks.c, line 4074
        --------------------------------------------------------------------
        node@0x7fd328c <ucHeap+6368>: {
          xItemValue = 0,
          pxNext = 0x7fd1800 <xSuspendedTaskList+8>,
          pxPrevious = 0x7fd3944 <ucHeap+8088>,
          pvOwner = 0x7fd3288 <ucHeap+6364>,
          pvContainer = 0x7fd17f8 <xSuspendedTaskList>
        } #5
        pvOwner@0x7fd3288 <ucHeap+6364>: {
          pxTopOfStack = 0x7fd3200 <ucHeap+6228>,
          xGenericListItem = {
            xItemValue = 0,
            pxNext = 0x7fd1800 <xSuspendedTaskList+8>,
            pxPrevious = 0x7fd3944 <ucHeap+8088>,
            pvOwner = 0x7fd3288 <ucHeap+6364>,
            pvContainer = 0x7fd17f8 <xSuspendedTaskList>
          },
          xEventListItem = {
            xItemValue = 4,
            pxNext = 0x7fd18b8 <xPendingReadyList+8>,
            pxPrevious = 0x7fd18b8 <xPendingReadyList+8>,
            pvOwner = 0x7fd3288 <ucHeap+6364>,
            pvContainer = 0x0
          },
          uxPriority = 3,
          pxStack = 0x7fd2e80 <ucHeap+5332>,
          pcTaskName = "bleA",
          uxTCBNumber = 6,
          uxTaskNumber = 0,
          uxBasePriority = 3,
          uxMutexesHeld = 0,
          ulNotifiedValue = 0,
          eNotifyState = eWaitingNotification
        }
        PC @ 0800cc1a ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/portable/GCC/ARM_CM0/port.c, line 273
        LR @ 0800e3c3 ---> symbol and line for /home/user/work/SDK/sdk/free_rtos/tasks.c, line 4074
        --------------------------------------------------------------------


### FREE_RTOS_HEAP

This section starts from
__EXPAND_FREE_RTOS_HEAP_START__ marker until
__EXPAND_FREE_RTOS_HEAP_STOP__.

It contains all the allocated memory blocks given by the FreeRTOS memory management system.
In addition, it provides statistics regarding the usage of the heap and its fragmentation level.

E.g


        __EXPAND_FREE_RTOS_HEAP_START__
                  ...
        memblock@0x7fd1e18 <ucHeap+1132> size 408 { -----------> memblock starting @ 0x7fd1e18, the actual pointer given by pvPortMalloc() is memblock + 8 bytes, block size 408 bytes.
          pxNextFreeBlock = 0x0,                    -----------> All the allocated memblocks have their pxNextFreeBlock == 0x0.
          xBlockSize = 0x80000198                   -----------> (memblock size || xBlockAllocatedBit), xBlockAllocatedBit == 0x80000000. The size includes the memblock header which is 8 bytes.
        }
        memblock@0x7fd1fb0 <ucHeap+1540> size 96 {
          pxNextFreeBlock = 0x0,
          xBlockSize = 0x80000060
        }
        memblock@0x7fd2010 <ucHeap+1636> size 200 {
          pxNextFreeBlock = 0x0,
          xBlockSize = 0x800000c8
        }
                  ...
        Heap size 21584: bytes                      ------------> Heap size based on configTOTAL_HEAP_SIZE
        Sum of allocated heap blocks: 15744 bytes   ------------> Consumed heap blocks
        Max size of a free block: 5048 bytes        ------------> Maximum size out of all free blocks
        Sum of free blocks 5824: bytes              ------------> Freed heap blocks
        Heap fragmentation percentage: 13.32%       ------------> Fragmentation level (the less the better)
        __EXPAND_FREE_RTOS_HEAP_STOP__


### FREE_RTOS_TASK_STATUS

This section starts from
__FREE_RTOS_TASK_STATUS_START__ marker until
__FREE_RTOS_TASK_STATUS_STOP__

This is a pretty-print of the pxTaskStatusArray variable. The project must have its vApplicationIdleHook()
modified accordingly in order to collect task information. Then dg_configTRACK_OS_HEAP must be set
to 1 in the custom header file.


E.g


        __FREE_RTOS_TASK_STATUS_START__
       Name   xTaskNumber    xHandle    eCurrentState  uxCurrentPriority uxBasePriority ulRunTimeCounter usStackHighWaterMark(words)  usStackHighWaterMark(bytes)

       "IDLE"     2         0x7fd2468         eReady          0               0               0                   60                            240
       "temp"    10         0x7fd4260       eBlocked          1               1               0                  122                            488
       "gpad"     9         0x7fd3d88       eBlocked          1               1               0                  120                            480
       "Tmr "     3         0x7fd2728       eBlocked          6               6               0                   69                            276
       "uart"     7         0x7fd3940       eBlocked          1               1               0                   38                            152
       "uart"     8         0x7fd3ad8       eBlocked          1               1               0                   38                            152
       "uart"     6         0x7fd37a8       eBlocked          1               1               0                   26                            104
       "cons"    11         0x7fd46b8     eSuspended          1               1               0                   55                            220
       "main"     4         0x7fd3280       eBlocked          1               1               0                   52                            208

                                                                                                         +                            +
                                                                                                         ---------------------------  ---------------------------

                                                                                                                  580                           2320

        __FREE_RTOS_TASK_STATUS_STOP__

### MEMORY HEX DUMP


This section starts from
__HEX_DUMP_START__ marker until
__HEX_DUMP_STOP__

E.g


        __HEX_DUMP_START__
        0x7fc0000:	0x07fc8000	0x080002a5	0x07fd0001	0x07fd0019
        0x7fc0010:	0x00000000	0x00000000	0x00000000	0x00000000
        0x7fc0020:	0x00000000	0x00000000	0x00000000	0x08000335
        ...
        __HEX_DUMP_STOP__

### MICRO TRACE BUFFER DUMP

This section starts from
__MTB_START__ marker until
__MTB_END__

This is a pretty-print of complete trace stream of the instructions that have been executed and will fit into the RAM area.
The project must have enabled the Micro Trace Buffer. To do so dg_configENABLE_MTB must be set
to 1 in the custom header file.
> Note: MTB is available only for 69x targets.

E.g

		__MTB_START__
		MTB_BASE        hex: 0x2007e000  bin: 00100000000001111110000000000000
		MTB_SECURE      hex: 0x00000000  bin: 00000000000000000000000000000000
		MTB_MASTER      hex: 0x80000009  bin: 10000000000000000000000000001001
		MTB_FLOW        hex: 0x00000000  bin: 00000000000000000000000000000000
		MTB_POSITION    hex: 0x00001e2c  bin: 00000000000000000001111000101100
		MTB_TSTART      hex: 0x00000000  bin: 00000000000000000000000000000000
		MTB_TSTOP       hex: 0x00000000  bin: 00000000000000000000000000000000
		***********************************0x2007fe20***********************************
		SOURCE ADDRESS = 0x00009c7e A_BIT = 0x0
			/home/user/work/SDK/sdk/free_rtos/task.c:1294
				0x9c7e <vTaskDelayUntil+142>:	pop	{r3, r4, r5, pc}
		DEST ADDRESS   = 0x0000a81a S_BIT = 0x0
			../main.c:150
				0xa81a <prvTemplateTask+18>:	ldr	r3, [pc, #40]	; (0xa844 <prvTemplateTask+60>)

		***********************************0x2007fe18***********************************
		SOURCE ADDRESS = 0x00009c7a A_BIT = 0x0
			/home/user/work/SDK/sdk/free_rtos/task.c:1288
				0x9c7a <vTaskDelayUntil+138>:	isb	sy
		DEST ADDRESS   = 0x00009c7e S_BIT = 0x0
			/home/user/work/SDK/sdk/free_rtos/task.c:1294
			   0x9c7e <vTaskDelayUntil+142>:	pop	{r3, r4, r5, pc}

		***********************************0x2007fe10***********************************
		SOURCE ADDRESS = 0xffffffbc A_BIT = 0x1
			Failed to find pc_line
		DEST ADDRESS   = 0x00009c7a S_BIT = 0x0
			/home/user/work/SDK/sdk/free_rtos/task.c:1288
			   0x9c7a <vTaskDelayUntil+138>:	isb	sy

		...
		__MTB_END__


		___ ___                              ________        ___.                       .__              ._._._.
         /   |   \_____  ______ ______ ___.__. \______ \   ____\_ |__  __ __  ____   ____ |__| ____    ____| | | |
        /    ~    \__  \ \____ \\____ <   |  |  |    |  \_/ __ \| __ \|  |  \/ ___\ / ___\|  |/    \  / ___\ | | |
        \    Y    // __ \|  |_> >  |_> >___  |  |    `   \  ___/| \_\ \  |  / /_/  > /_/  >  |   |  \/ /_/  >|\|\|
         \___|_  /(____  /   __/|   __// ____| /_______  /\___  >___  /____/\___  /\___  /|__|___|  /\___  /______
               \/      \/|__|   |__|   \/              \/     \/    \/     /_____//_____/         \//_____/ \/\/\/
