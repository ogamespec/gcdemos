#include <dolphin.h>
#include "video.h"

/* Show information for MSR register. */
static void describe_msr(u32 msr_val)
{
	static char *fpmod[4] = 
	{
		"exceptions disabled",
		"imprecise nonrecoverable",
		"imprecise recoverable",
		"precise mode",                        
	};
	int f;

	my_printf("MSR: 0x%08X\n", msr_val);
	
	if(msr_val & MSR_POW) my_printf("MSR[POW]: 1, power management enabled\n");
	else my_printf("MSR[POW]: 0, power management disabled\n");
	if(msr_val & MSR_ILE) my_printf("MSR[ILE]: 1\n");
	else my_printf("MSR[ILE]: 0\n");
	if(msr_val & MSR_EE) my_printf("MSR[EE] : 1, external interrupts and decrementer exception are enabled\n");
	else my_printf("MSR[EE] : 0, external interrupts and decrementer exception are disabled\n");
	if(msr_val & MSR_PR) my_printf("MSR[PR] : 1, processor execute in user mode (UISA)\n");
	else my_printf("MSR[PR] : 0, processor execute in supervisor mode (OEA)\n");
	if(msr_val & MSR_FP) my_printf("MSR[FP] : 1, floating-point is available\n");
	else my_printf("MSR[FP] : 0, floating-point unavailable\n");
	if(msr_val & MSR_ME) my_printf("MSR[ME] : 1, machine check exceptions are enabled\n");
	else my_printf("MSR[ME] : 0, machine check exceptions are disabled\n");
	
	f = (((msr_val >> MSR_FE0_BIT) & 1) << 1) | ((msr_val >> MSR_FE1_BIT) & 1);
	my_printf("MSR[FE] : %i, floating-point %s\n", f, fpmod[f]);
	
	if(msr_val & MSR_SE) my_printf("MSR[SE] : 1, single-step tracing is enabled\n");
	else my_printf("MSR[SE] : 0, single-step tracing is disabled\n");
	if(msr_val & MSR_BE) my_printf("MSR[BE] : 1, branch tracing is enabled\n");
	else my_printf("MSR[BE] : 0, branch tracing is disabled\n");
	if(msr_val & MSR_IP) my_printf("MSR[IP] : 1, exception prefix to physical address is 0xFFFn_nnnn\n");
	else my_printf("MSR[IP] : 0, exception prefix to physical address is 0x000n_nnnn\n");
	if(msr_val & MSR_IR) my_printf("MSR[IR] : 1, instruction address translation is enabled\n");
	else my_printf("MSR[IR] : 0, instruction address translation is disabled\n");
	if(msr_val & MSR_DR) my_printf("MSR[DR] : 1, data address translation is enabled\n");
	else my_printf("MSR[DR] : 0, data address translation is disabled\n");
	if(msr_val & MSR_PM) my_printf("MSR[PM] : 1, performance monitoring is enabled for this thread\n");
	else my_printf("MSR[PM] : 0, performance monitoring is disabled for this thread\n");
	if(msr_val & MSR_RI) my_printf("MSR[RI] : 1\n");
	else my_printf("MSR[RI] : 0\n");
	if(msr_val & MSR_LE) my_printf("MSR[LE] : 1, processor runs in little-endian mode\n");
	else my_printf("MSR[LE] : 0, processor runs in big-endian mode\n");
}

/* Actual Gekko testing. */
void *test_gekko(void *param)
{
	u32 val;
	
	/* Show MSR */
	val = PPCMfmsr();
	describe_msr(val);


	/* Halt thread. */
	while(1);
}
