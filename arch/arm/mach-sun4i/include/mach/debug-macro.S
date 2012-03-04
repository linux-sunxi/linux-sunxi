/*
 * arch/arm/mach-sun4i/include/mach/debug-macro.S
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <mach/hardware.h>
#include <mach/uart_regs.h>

	.macro	addruart, rp, rv
	ldr	\rp, =PA_UARTS_BASE
	ldr	\rv, =PV_UARTS_BASE
#if CONFIG_SW_DEBUG_UART != 0
	add	\rp, \rp, #(0x400 * CONFIG_SW_DEBUG_UART)
	add	\rv, \rv, #(0x400 * CONFIG_SW_DEBUG_UART)
#endif
	.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
