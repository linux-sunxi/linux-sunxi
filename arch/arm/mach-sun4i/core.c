#include <linux/init.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/amba/bus.h>
#include <linux/amba/clcd.h>
#include <linux/amba/pl061.h>
#include <linux/amba/mmci.h>
#include <linux/amba/pl022.h>
#include <linux/io.h>
#include <linux/gfp.h>
#include <linux/clockchips.h>
#include <linux/memblock.h>
#include <linux/bootmem.h>

#include <asm/clkdev.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/leds.h>
#include <asm/hardware/arm_timer.h>
#include <asm/hardware/icst.h>
#include <asm/hardware/vic.h>
#include <asm/mach-types.h>
#include <asm/setup.h>

#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <mach/clkdev.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#include <mach/system.h>
#include <mach/script_i.h>
#include <mach/gpio_v2.h>
#include <mach/script_v2.h>


#include "core.h"

#define SYS_TIMER_SCAL      (16)            /* timer clock source pre-divsion   */
#define SYS_TIMER_CLKSRC    (24000000)      /* timer clock source               */
#define TMR_INTER_VAL       (SYS_TIMER_CLKSRC/(SYS_TIMER_SCAL*HZ))


/**
 * Global vars definitions
 *
 */
static void timer_set_mode(enum clock_event_mode mode, struct clock_event_device *clk)
{
    volatile u32 ctrl;

    switch (mode) {
    case CLOCK_EVT_MODE_PERIODIC:
        printk("timer_set_mode: periodic\n");
        writel(TMR_INTER_VAL, SW_TIMER0_INTVAL_REG); /* interval (999+1) */
        ctrl = readl(SW_TIMER0_CTL_REG);
        ctrl &= ~(1<<7);    //continuous mode
        ctrl |= 1;  //enable
        break;

    case CLOCK_EVT_MODE_ONESHOT:
        printk("timer_set_mode: oneshot\n");
        ctrl = readl(SW_TIMER0_CTL_REG);
        ctrl |= (1<<7);     //single mode
        break;
    case CLOCK_EVT_MODE_UNUSED:
    case CLOCK_EVT_MODE_SHUTDOWN:
    default:
        ctrl = readl(SW_TIMER0_CTL_REG);
        ctrl &= ~(1<<0);    //disable timer 0
        break;
    }

    writel(ctrl, SW_TIMER0_CTL_REG);
}

static int timer_set_next_event(unsigned long evt, struct clock_event_device *unused)
{
    volatile u32 ctrl;

    /* clear any pending before continue */
    ctrl = readl(SW_TIMER0_CTL_REG);
    writel(evt, SW_TIMER0_CNTVAL_REG);
    ctrl |= (1<<1);
    writel(ctrl, SW_TIMER0_CTL_REG);
    writel(ctrl | 0x1, SW_TIMER0_CTL_REG);

    return 0;
}

static struct clock_event_device timer0_clockevent = {
    .name = "timer0",
    .shift = 32,
    .features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
    .set_mode = timer_set_mode,
    .set_next_event = timer_set_next_event,
};


static irqreturn_t sw_timer_interrupt(int irq, void *dev_id)
{
    struct clock_event_device *evt = &timer0_clockevent;

    writel(0x1, SW_TIMER_INT_STA_REG);

    evt->event_handler(evt);

    return IRQ_HANDLED;
}

static struct irqaction sw_timer_irq = {
    .name = "Softwinner Timer Tick",
    .flags = IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
    .handler = sw_timer_interrupt,
};

void sw_irq_ack(struct irq_data *d)
{
    unsigned int irq = d->irq;
    if (irq < 32){
        writel(readl(SW_INT_ENABLE_REG0) & ~(1<<irq), SW_INT_ENABLE_REG0);
        writel(readl(SW_INT_MASK_REG0) | (1 << irq), SW_INT_MASK_REG0);
        writel(readl(SW_INT_IRQ_PENDING_REG0) | (1<<irq), SW_INT_IRQ_PENDING_REG0);
    }else if(irq < 64){
        irq -= 32;
        writel(readl(SW_INT_ENABLE_REG1) & ~(1<<irq), SW_INT_ENABLE_REG1);
        writel(readl(SW_INT_MASK_REG1) | (1 << irq), SW_INT_MASK_REG1);
        writel(readl(SW_INT_IRQ_PENDING_REG1) | (1<<irq), SW_INT_IRQ_PENDING_REG1);
    }else if(irq < 96){
        irq -= 64;
        writel(readl(SW_INT_ENABLE_REG2) & ~(1<<irq), SW_INT_ENABLE_REG2);
        writel(readl(SW_INT_MASK_REG2) | (1 << irq), SW_INT_MASK_REG2);
        writel(readl(SW_INT_IRQ_PENDING_REG2) | (1<<irq), SW_INT_IRQ_PENDING_REG2);
    }
}

/* Disable irq */
static void sw_irq_mask(struct irq_data *d)
{
    unsigned int irq = d->irq;
    if(irq < 32){
        writel(readl(SW_INT_ENABLE_REG0) & ~(1<<irq), SW_INT_ENABLE_REG0);
        writel(readl(SW_INT_MASK_REG0) | (1 << irq), SW_INT_MASK_REG0);
    }else if(irq < 64){
        irq -= 32;
        writel(readl(SW_INT_ENABLE_REG1) & ~(1<<irq), SW_INT_ENABLE_REG1);
        writel(readl(SW_INT_MASK_REG1) | (1 << irq), SW_INT_MASK_REG1);
    }else if(irq < 96){
        irq -= 64;
        writel(readl(SW_INT_ENABLE_REG2) & ~(1<<irq), SW_INT_ENABLE_REG2);
        writel(readl(SW_INT_MASK_REG2) | (1 << irq), SW_INT_MASK_REG2);
    }
}

/* Enable irq */
static void sw_irq_unmask(struct irq_data *d)
{
    unsigned int irq = d->irq;
    if(irq < 32){
        writel(readl(SW_INT_ENABLE_REG0) | (1<<irq), SW_INT_ENABLE_REG0);
        writel(readl(SW_INT_MASK_REG0) & ~(1 << irq), SW_INT_MASK_REG0);
        if(irq == SW_INT_IRQNO_ENMI) /* must clear pending bit when enabled */
            writel((1 << SW_INT_IRQNO_ENMI), SW_INT_IRQ_PENDING_REG0);
    }else if(irq < 64){
        irq -= 32;
        writel(readl(SW_INT_ENABLE_REG1) | (1<<irq), SW_INT_ENABLE_REG1);
        writel(readl(SW_INT_MASK_REG1) & ~(1 << irq), SW_INT_MASK_REG1);
    }else if(irq < 96){
        irq -= 64;
        writel(readl(SW_INT_ENABLE_REG2) | (1<<irq), SW_INT_ENABLE_REG2);
        writel(readl(SW_INT_MASK_REG2) & ~(1 << irq), SW_INT_MASK_REG2);
    }
}

static struct irq_chip sw_f23_sic_chip = {
    .name   = "SW_F23_SIC",
    .irq_ack    = sw_irq_ack,
    .irq_mask   = sw_irq_mask,
    .irq_unmask = sw_irq_unmask,
};

void __init sw_init_irq(void)
{
    u32 i = 0;

    /* Disable & clear all interrupts */
    writel(0, SW_INT_ENABLE_REG0);
    writel(0, SW_INT_ENABLE_REG1);
    writel(0, SW_INT_ENABLE_REG2);

    writel(0xffffffff, SW_INT_MASK_REG0);
    writel(0xffffffff, SW_INT_MASK_REG1);
    writel(0xffffffff, SW_INT_MASK_REG2);

    writel(0xffffffff, SW_INT_IRQ_PENDING_REG0);
    writel(0xffffffff, SW_INT_IRQ_PENDING_REG1);
    writel(0xffffffff, SW_INT_IRQ_PENDING_REG2);
    writel(0xffffffff, SW_INT_FIQ_PENDING_REG0);
    writel(0xffffffff, SW_INT_FIQ_PENDING_REG1);
    writel(0xffffffff, SW_INT_FIQ_PENDING_REG2);

    /*enable protection mode*/
    writel(0x01, SW_INT_PROTECTION_REG);
    /*config the external interrupt source type*/
    writel(0x00, SW_INT_NMI_CTRL_REG);

    for (i = SW_INT_START; i < SW_INT_END; i++) {
        irq_set_chip_and_handler(i, &sw_f23_sic_chip, handle_level_irq);
        set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
    }
}

static struct map_desc sw_io_desc[] __initdata = {
    { SW_VA_SRAM_BASE,          __phys_to_pfn(SW_PA_SRAM_BASE),         SZ_32K, MT_MEMORY_ITCM  },
    { SW_VA_IO_BASE,            __phys_to_pfn(SW_PA_IO_BASE),           SZ_4M,  MT_DEVICE       },
#if 0
    { SW_VA_CCM_IO_BASE,        __phys_to_pfn(SW_PA_CCM_IO_BASE),       SZ_1K,  MT_DEVICE       },
    { SW_VA_SRAM_IO_BASE,       __phys_to_pfn(SW_PA_SRAM_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_DRAM_IO_BASE,       __phys_to_pfn(SW_PA_DRAM_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_DMAC_IO_BASE,       __phys_to_pfn(SW_PA_DMAC_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_NANDFLASHC_IO_BASE, __phys_to_pfn(SW_PA_NANDFLASHC_IO_BASE),SZ_4K,  MT_DEVICE       },
    { SW_VA_INT_IO_BASE,        __phys_to_pfn(SW_PA_INT_IO_BASE),       SZ_1K,  MT_DEVICE       },
    { SW_VA_PORTC_IO_BASE,      __phys_to_pfn(SW_PA_PORTC_IO_BASE),     SZ_1K,  MT_DEVICE       },
    { SW_VA_TIMERC_IO_BASE,     __phys_to_pfn(SW_PA_TIMERC_IO_BASE),    SZ_1K,  MT_DEVICE       },
    { SW_VA_UART0_IO_BASE,      __phys_to_pfn(SW_PA_UART0_IO_BASE),     SZ_1K,  MT_DEVICE       },
    { SW_VA_SID_IO_BASE,        __phys_to_pfn(SW_PA_SID_IO_BASE),       SZ_1K,  MT_DEVICE       },
    { SW_VA_TP_IO_BASE,         __phys_to_pfn(SW_PA_TP_IO_BASE),        SZ_1K,  MT_DEVICE       },
    { SW_VA_LRADC_IO_BASE,      __phys_to_pfn(SW_PA_LRADC_IO_BASE),     SZ_1K,  MT_DEVICE       },
    { SW_VA_IR0_IO_BASE,        __phys_to_pfn(SW_PA_IR0_IO_BASE),       SZ_1K,  MT_DEVICE       },
    { SW_VA_TWI0_IO_BASE,       __phys_to_pfn(SW_PA_TWI0_IO_BASE),      SZ_1K,  MT_DEVICE       },
    { SW_VA_TWI1_IO_BASE,       __phys_to_pfn(SW_PA_TWI1_IO_BASE),      SZ_1K,  MT_DEVICE       },
    { SW_VA_TWI2_IO_BASE,       __phys_to_pfn(SW_PA_TWI2_IO_BASE),      SZ_1K,  MT_DEVICE       },
    { SW_VA_USB0_IO_BASE,       __phys_to_pfn(SW_PA_USB0_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_USB1_IO_BASE,       __phys_to_pfn(SW_PA_USB1_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_USB2_IO_BASE,       __phys_to_pfn(SW_PA_USB2_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_GPS_IO_BASE,        __phys_to_pfn(SW_PA_GPS_IO_BASE),       SZ_64K, MT_DEVICE       },

    { SW_VA_TVE0_IO_BASE,       __phys_to_pfn(SW_PA_TVE0_IO_BASE),      SZ_4K,  MT_DEVICE       },
    { SW_VA_TVE1_IO_BASE,       __phys_to_pfn(SW_PA_TVE1_IO_BASE),      SZ_4K,  MT_DEVICE       },

    { SW_VA_TCON0_IO_BASE,      __phys_to_pfn(SW_PA_TCON0_IO_BASE),     SZ_4K,  MT_DEVICE       },
    { SW_VA_TCON1_IO_BASE,      __phys_to_pfn(SW_PA_TCON1_IO_BASE),     SZ_4K,  MT_DEVICE       },

    { SW_VA_HDMI_IO_BASE,       __phys_to_pfn(SW_PA_HDMI_IO_BASE),      SZ_4K,  MT_DEVICE       },

    { SW_VA_DEFE0_IO_BASE,      __phys_to_pfn(SW_PA_DEFE0_IO_BASE),     SZ_4K,  MT_DEVICE       },
    { SW_VA_DEFE1_IO_BASE,      __phys_to_pfn(SW_PA_DEFE1_IO_BASE),     SZ_4K,  MT_DEVICE       },
    { SW_VA_DEBE0_IO_BASE,      __phys_to_pfn(SW_PA_DEBE0_IO_BASE),     SZ_4K,  MT_DEVICE       },
    { SW_VA_DEBE1_IO_BASE,      __phys_to_pfn(SW_PA_DEBE1_IO_BASE),     SZ_4K,  MT_DEVICE       },
#endif

    { SW_VA_SRAM_BROM,          __phys_to_pfn(SW_PA_BROM_START),        SZ_32K, MT_MEMORY_ITCM  },
};

void __init sw_map_io(void)
{
    iotable_init(sw_io_desc, ARRAY_SIZE(sw_io_desc));
}

struct bus_type sw_subsys = {
    .name = "sw-core",
    .dev_name = "sw-core",
};

static struct device sw_dev = {
    .bus = &sw_subsys,
};

static u32 DRAMC_get_dram_size(void)
{
    u32 reg_val;
    u32 dram_size;
    u32 chip_den;

    reg_val = readl(SW_DRAM_SDR_DCR);
    chip_den = (reg_val>>3)&0x7;
    if(chip_den == 0)
        dram_size = 32;
    else if(chip_den == 1)
        dram_size = 64;
    else if(chip_den == 2)
        dram_size = 128;
    else if(chip_den == 3)
        dram_size = 256;
    else if(chip_den == 4)
        dram_size = 512;
    else
        dram_size = 1024;

    if( ((reg_val>>1)&0x3) == 0x1)
        dram_size<<=1;
    if( ((reg_val>>6)&0x7) == 0x3)
        dram_size<<=1;
    if( ((reg_val>>10)&0x3) == 0x1)
        dram_size<<=1;

    return dram_size;
}

extern unsigned long fb_start;
extern unsigned long fb_size;
extern unsigned long gps_start;
extern unsigned long gps_size;
extern unsigned long g2d_start;
extern unsigned long g2d_size;

static int __init sw_core_init(void)
{
    pr_info("DRAM Size: %u\n", DRAMC_get_dram_size());
    return subsys_system_register(&sw_subsys, NULL);
}
core_initcall(sw_core_init);

extern int sw_register_clocks(void);
void __init sw_init(void)
{
    device_register(&sw_dev);
}

static void __init sw_timer_init(void)
{
    volatile u32  val = 0;

    writel(TMR_INTER_VAL, SW_TIMER0_INTVAL_REG);
    /* set clock sourch to HOSC, 16 pre-division */
    val = readl(SW_TIMER0_CTL_REG);
    val &= ~(0x07<<4);
    val &= ~(0x03<<2);
    val |= (4<<4) | (1<<2);
    writel(val, SW_TIMER0_CTL_REG);
    /* set mode to auto reload */
    val = readl(SW_TIMER0_CTL_REG);
    val |= (1<<1);
    writel(val, SW_TIMER0_CTL_REG);
    setup_irq(SW_INT_IRQNO_TIMER0, &sw_timer_irq);

    /* Enable time0 interrupt */
    val = readl(SW_TIMER_INT_CTL_REG);
    val |= (1<<0);
    writel(val, SW_TIMER_INT_CTL_REG);

    timer0_clockevent.mult = div_sc(SYS_TIMER_CLKSRC/SYS_TIMER_SCAL, NSEC_PER_SEC, timer0_clockevent.shift);
    timer0_clockevent.max_delta_ns = clockevent_delta2ns(0xff, &timer0_clockevent);
    timer0_clockevent.min_delta_ns = clockevent_delta2ns(0x1, &timer0_clockevent);
    timer0_clockevent.cpumask = cpumask_of(0);
    clockevents_register_device(&timer0_clockevent);
}

extern int script_get_int(const char *script_buf, const char *main_key, const char *sub_key);
static void __init sw_reserve(void)
{
	int gps_used = 0;
	int g2d_used = 0;
	char *script_base = (char *)(PAGE_OFFSET + 0x3000000);

	gps_used = script_get_int(script_base, "gps_para", "gps_used");
	g2d_used = script_get_int(script_base, "g2d_para", "g2d_used");

	memblock_reserve(CONFIG_SW_SYSMEM_RESERVED_BASE, CONFIG_SW_SYSMEM_RESERVED_SIZE * 1024);
	memblock_reserve(fb_start, fb_size);

	pr_info("Memory Reserved:\n");
	pr_info("  VE:\t0x%08x, 0x%08x\n", (unsigned int)CONFIG_SW_SYSMEM_RESERVED_BASE,
		(unsigned int)CONFIG_SW_SYSMEM_RESERVED_SIZE * 1024);
	pr_info("  FB:\t0x%08x, 0x%08x\n", (unsigned int)fb_start, (unsigned int)fb_size);

/* reserve memory for GPS, G2D */
	if (gps_used) {
		gps_size = script_get_int(script_base, "gps_para", "gps_size");
		if (gps_size < 0 || gps_size > SW_GPS_MEM_MAX) {
			gps_size = SW_GPS_MEM_MAX;
		}
		gps_start = SW_GPS_MEM_BASE;
		gps_size = gps_size;
		pr_info("  GPS:\t0x%08x, 0x%08x\n", (unsigned int)gps_start, (unsigned int)gps_size);
		memblock_reserve(gps_start, gps_size);
	}

	if (g2d_used) {
		g2d_size = script_get_int(script_base, "g2d_para", "g2d_size");
		if (g2d_size < 0 || g2d_size > SW_G2D_MEM_MAX) {
			g2d_size = SW_G2D_MEM_MAX;
		}
		g2d_start = SW_G2D_MEM_BASE;
		g2d_size = g2d_size;
		pr_info("  G2D:\t0x%08x, 0x%08x\n", (unsigned int)g2d_start, (unsigned int)g2d_size);
		memblock_reserve(g2d_start, g2d_size);
	}
}

static void __init sw_fixup(struct tag *tags, char **cmdline,
			    struct meminfo *mi)
{
    u32 size;

    pr_info("Lichee System fixup\n");
    if (mi->nr_banks != 0) {
	    int i;
	    for (i = 0; i < mi->nr_banks; i++)
		size += mi->bank[i].size / SZ_1M;
	    pr_info("Total Configured Memory: %uMB with %d banks\n", size, mi->nr_banks);
    } else {
	    size = DRAMC_get_dram_size();

	    if (size <= 512) {
		    mi->nr_banks=1;
		    mi->bank[0].start = 0x40000000;
		    mi->bank[0].size = SZ_1M * (size - 64);
	    } else {
		    mi->nr_banks=2;
		    mi->bank[0].start = 0x40000000;
		    mi->bank[0].size = SZ_1M * (512 - 64);
		    mi->bank[1].start = 0x60000000;
		    mi->bank[1].size = SZ_1M * (size - 512);
	    }
	    pr_info("Total Detected Memory: %uMB with %d banks\n", size, mi->nr_banks);
    }
}

struct sys_timer sw_timer = {
    .init = sw_timer_init,
};

enum sw_ic_ver sw_get_ic_ver(void)
{
	volatile u32 val;

    /* write the register */
	val = readl(SW_VA_TIMERC_IO_BASE + 0x13c);
    val &= ~(0x03<<6);
    writel(val, SW_VA_TIMERC_IO_BASE + 0x13c);

    /* read the register and parse ic version */
	val = readl(SW_VA_TIMERC_IO_BASE + 0x13c);
	val = (val >> 6) & 0x3;
	if (val == 0x00) {
		return MAGIC_VER_A;
	}
	else if(val == 0x03) {
	    return MAGIC_VER_B;
	}
	return MAGIC_VER_C;
}
EXPORT_SYMBOL(sw_get_ic_ver);

static script_sub_key_t *get_sub_key(const char *script_buf, const char *main_key, const char *sub_key)
{
	script_head_t *hd = NULL;
	script_main_key_t *mk = NULL;
	script_sub_key_t *sk = NULL;
	int i, j;

	if (main_key == NULL || sub_key == NULL) {
		return NULL;
	}

	hd = (script_head_t *)script_buf;
	mk = (script_main_key_t *)(hd + 1);

	for (i = 0; i < hd->main_key_count; i++) {
		if (strcmp(main_key, mk->main_name)) {
			mk++;
			continue;
		}

		for (j = 0; j < mk->lenth; j++) {
			sk = (script_sub_key_t *)(script_buf + (mk->offset<<2) + j * sizeof(script_sub_key_t));
			if (!strcmp(sub_key, sk->sub_name)) {
				return sk;
			}
		}
	}
	return NULL;
}

/**
 * @func: script_get_int
 * @desc: get key value
 * @ret: -1 ERROR, >=0 OK
 * @arg1
 * @arg2:
 * @arg3:
 **/
int script_get_int(const char *script_buf, const char *main_key, const char *sub_key)
{
	script_sub_key_t *sk = NULL;
	char *pdata;
	int value;

	sk = get_sub_key(script_buf, main_key, sub_key);
	if (sk == NULL) {
		return -1;
	}

	if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
		pdata = (char *)(script_buf + (sk->offset<<2));
		value = *((int *)pdata);
		return value;
	}

	return -1;
}

/**
 * @func: script_get_str
 * @desc: get key value
 * @ret: NULL on ERROR
 * @arg1
 * @arg2:
 * @arg3:
 * @arg4: user require to clean the buf
 **/
char *script_get_str(const char *script_buf, const char *main_key, const char *sub_key, char *buf)
{
	script_sub_key_t *sk = NULL;
	char *pdata;

	sk = get_sub_key(script_buf, main_key, sub_key);
	if (sk == NULL) {
		return NULL;
	}

	if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_STRING) {
		pdata = (char *)(script_buf + (sk->offset<<2));
		memcpy(buf, pdata, ((sk->pattern >> 0) & 0xffff));
		return (char *)buf;
	}

	return NULL;
}

MACHINE_START(SUN4I, "sun4i")
	.map_io         = sw_map_io,
	.fixup          = sw_fixup,
	.reserve        = sw_reserve,
	.init_irq       = sw_init_irq,
	.timer          = &sw_timer,
	.init_machine   = sw_init,
	.atag_offset	= 0x100,
MACHINE_END


