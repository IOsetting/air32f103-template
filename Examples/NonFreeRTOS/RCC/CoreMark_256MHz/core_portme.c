/* 
	File : core_portme.c
*/
/*
	Author : Shay Gal-On, EEMBC
	Legal : TODO!
*/ 
#include <stdio.h>
#include <stdlib.h>
#include "coremark.h"

#include "air32f10x_tim.h"

// ѭ�������Ĵ�����ֻ��Ҫ������ʱ�����10���Ӽ���
#define ITERATIONS 10000


#if VALIDATION_RUN
	volatile ee_s32 seed1_volatile=0x3415;
	volatile ee_s32 seed2_volatile=0x3415;
	volatile ee_s32 seed3_volatile=0x66;
#endif
#if PERFORMANCE_RUN
	volatile ee_s32 seed1_volatile=0x0;
	volatile ee_s32 seed2_volatile=0x0;
	volatile ee_s32 seed3_volatile=0x66;
#endif
#if PROFILE_RUN
	volatile ee_s32 seed1_volatile=0x8;
	volatile ee_s32 seed2_volatile=0x8;
	volatile ee_s32 seed3_volatile=0x8;
#endif
	volatile ee_s32 seed4_volatile=ITERATIONS;
	volatile ee_s32 seed5_volatile=0;
/* Porting : Timing functions
	How to capture time and convert to seconds must be ported to whatever is supported by the platform.
	e.g. Read value from on board RTC, read value from cpu clock cycles performance counter etc. 
	Sample implementation for standard time.h and windows.h definitions included.
*/
/* Define : TIMER_RES_DIVIDER
	Divider to trade off timer resolution and total time that can be measured.

	Use lower values to increase resolution, but make sure that overflow does not occur.
	If there are issues with the return value overflowing, increase this value.
	*/
	
__IO uint32_t Tick;


//#define NSECS_PER_SEC CLOCKS_PER_SEC
//#define CORETIMETYPE clock_t 
//#define GETMYTIME(_t) (*_t=clock())
//#define MYTIMEDIFF(fin,ini) ((fin)-(ini))
//#define TIMER_RES_DIVIDER 1
//#define SAMPLE_TIME_IMPLEMENTATION 1
#define EE_TICKS_PER_SEC 1000

/** Define Host specific (POSIX), or target specific global time variables. */
//static CORETIMETYPE start_time_val, stop_time_val;

void start_time(void) {
	Tick = 0;
	TIM_Cmd(TIM2,ENABLE);
}

void stop_time(void) {
	/* Stop the Timer and get the encoding 
time */ 
TIM_Cmd(TIM2,DISABLE);
}

CORE_TICKS get_time(void) {
	CORE_TICKS elapsed = 
(CORE_TICKS)Tick;
return elapsed;

}
/* Function : time_in_secs
	Convert the value returned by get_time to seconds.

	The <secs_ret> type is used to accomodate systems with no support for floating point.
	Default implementation implemented by the EE_TICKS_PER_SEC macro above.
*/
secs_ret time_in_secs(CORE_TICKS ticks) {
	secs_ret retval=((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
	return retval;
}

ee_u32 default_num_contexts=1;

/* Function : portable_init
	Target specific initialization code 
	Test for some common mistakes.
*/
void portable_init(core_portable *p, int *argc, char *argv[])
{
	(void)argc;
	(void)argv;
	extern void main_original(void);
	main_original();
	
	
	if (sizeof(ee_ptr_int) != sizeof(ee_u8 *)) {
		ee_printf("ERROR! Please define ee_ptr_int to a type that holds a pointer!\n");
	}
	if (sizeof(ee_u32) != 4) {
		ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type!\n");
	}
	p->portable_id=1;
}
/* Function : portable_fini
	Target specific final code 
*/
void portable_fini(core_portable *p)
{
	p->portable_id=0;
}


