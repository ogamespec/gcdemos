# Gekko_Explore Demo

Gekko exploration tests demo.

![Gekko_Explore](/imgstore/Gekko_Explore.jpg)

From the contents of the MSR you can find out in which mode the Gekko is running when executing regular GC programs:
- Power management is off. Dolphin OS does not use Gekko's low power modes ("sleep", "doze").
- The processor always runs in supervisor mode. This means that regular programs have OS rights (a Dolphin OS feature).
- The byte order Gekko uses is big-endian, while X86 family uses little-endian. This should be taken into account when working with files.
- When switching the FPU context, a clever trick called "FPU N/A" is used: FPU registers are not saved in the context every time the process switches, but only when they change. This is possible because of a special exception that occurs when trying to execute an FPU instruction if "FPU N/A" is on. This reduces the time for context switching (overloading).

As you can see even such a simple program gives a lot of information.

Note for GC emulator authors. This demo use the following things:
- Video Interface for drawing (PAL or NTSC interlaced mode).
- Hardware reset ("hot reset") by Reset Button
- OS thread scheduling.
