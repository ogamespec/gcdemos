/*
 * Gekko exploration tests demo.
*/

#include <dolphin.h>
#include "video.h"
#include "dhry.h"
 
static OSThread thread_ctx;
static u8 thread_stack[8192];

static OSHeapHandle MyHeap;

static void reset(void)
{
	OSResetSystem(OS_RESET_HOTRESET, 0, FALSE);
}

static void init_osarena(void)
{
    void* arenaLo;
    void* arenaHi;	
    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1);
    OSSetArenaLo(arenaLo);
    MyHeap = OSCreateHeap((void*)OSRoundUp32B(arenaLo), (void*)OSRoundDown32B(arenaHi));
    OSSetCurrentHeap(MyHeap);
    OSSetArenaLo(arenaLo = arenaHi);
}

void main(void)
{
	#ifndef __SDKVER__
	OSHalt("Please, compile with Metrowerks CodeWarrior and Nintendo Dolphin SDK.\n");
	#endif

	/*
	 * Initialized GC sub-systems: CPU, EXI and SI (both internally by OSInit), VI.
	 * Left uninitialized: PAD, CARD, AI, DSP, ARAM, DVD, whole GX.
	*/
	OSInit();
	OSSetResetCallback(reset);
	vi_initialize();
	init_osarena();
	
	OSCreateThread(&thread_ctx, Dhrystone_main, 0, thread_stack + sizeof(thread_stack), sizeof(thread_stack), 31, OS_THREAD_ATTR_DETACH);
	OSResumeThread(&thread_ctx);

	my_printf("Welcome to Gekko Dhrystone 2.1 demo.\n");
	
	while(!OSIsThreadTerminated(&thread_ctx))
	{
		update_message_buf();
		vi_swap_chain ();
	}
	
	OSReport("Demo complete. Executed %llu frames.\n", vi_get_frame_count());
	PPCHalt();
}
