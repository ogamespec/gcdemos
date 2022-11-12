/*
 * Gekko exploration tests demo.
*/

#include <dolphin.h>
#include "video.h"
#include "gekko_test.h"
 
static OSThread thread_ctx;
static u8 thread_stack[8192];

static void reset(void)
{
	OSResetSystem(OS_RESET_HOTRESET, 0, FALSE);
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
	
	/*
	 * CPU is running in privileged mode with enabled interrupts from now.
	 * Dolphin OS has installed following exception handlers after OSInit:
	 *   0x0500 - External interrupt
	 *   0x0800 - Floating-point unavailable
	 *   0x0900 - Decrementer (for OSAlarm object processing)
	 *   0x0C00 - System call (do something bogus?)
	 * Other exceptions are handled by 'default' exception handler, which ends
	 * by PPCHalt and GameCube hung.
	 *
	 * For our dirty purposes we will use Program exception handler (0x0700)  >:)
	*/
	
	OSCreateThread(&thread_ctx, test_gekko, 0, thread_stack + sizeof(thread_stack), sizeof(thread_stack), 31, OS_THREAD_ATTR_DETACH);
	OSResumeThread(&thread_ctx);

	my_printf("Welcome to Gamecube exploration demo.\n");
	
	while(!OSIsThreadTerminated(&thread_ctx))
	{
		put_logo();
		update_message_buf();
		vi_swap_chain ();
	}
	
	OSReport("GameCube exploration demo complete. Executed %llu frames.\n", vi_get_frame_count());
	PPCHalt();
}
