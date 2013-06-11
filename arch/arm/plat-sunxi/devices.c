/*
 * arch/arm/plat-sunxi/devices.c
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Benn Huang <benn@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#ifdef CONFIG_ANDROID_RAM_CONSOLE
#include <linux/platform_data/ram_console.h>
#endif
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/pda_power.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/ahci_platform.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>
#include <asm/pmu.h>
#include <mach/hardware.h>
#include <plat/i2c.h>
#ifdef CONFIG_ANDROID_RAM_CONSOLE
#include <plat/ramconsole.h>
#endif

#include <plat/sys_config.h>
#include <plat/ahci.h>

#if 0
/* uart */
static struct plat_serial8250_port debug_uart_platform_data[] = {
	{
		.membase	= (void __iomem *)SW_VA_UART0_IO_BASE,
		.mapbase	= (resource_size_t)SW_PA_UART0_IO_BASE,
		.irq		= SW_INT_IRQNO_UART0,
		.flags		= UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_DWAPB32,
		.regshift	= 2,
		.uartclk	= 24000000,
	}, {
		.flags		= 0
	}
};

static struct platform_device debug_uart = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = debug_uart_platform_data,
	},
};
#endif

/* dma */
static struct platform_device sw_pdev_dmac = {
	.name = "sw_dmac",
};

static struct resource sw_res_nand =
{
	.start = SW_PA_NANDFLASHC_IO_BASE,
	.end = SW_PA_NANDFLASHC_IO_BASE + 0x1000,
	.flags = IORESOURCE_MEM,
};

struct platform_device sw_pdev_nand =
{
	.name = "sw_nand",
	.id = -1,
	.num_resources = 1,
	.resource = &sw_res_nand,
	.dev = {}
};

/* twi0 */
static struct sunxi_i2c_platform_data sunxi_twi0_pdata[] = {
	{
		.bus_num   = 0,
		.frequency = I2C0_TRANSFER_SPEED,
	},
};

static struct resource sunxi_twi0_resources[] = {
	{
		.start	= TWI0_BASE_ADDR_START,
		.end	= TWI0_BASE_ADDR_END,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= SW_INT_IRQNO_TWI0,
		.end	= SW_INT_IRQNO_TWI0,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device sunxi_twi0_device = {
	.name		= "sunxi-i2c",
	.id		    = 0,
	.resource	= sunxi_twi0_resources,
	.num_resources	= ARRAY_SIZE(sunxi_twi0_resources),
	.dev = {
		.platform_data = sunxi_twi0_pdata,
	},
};

/* twi1 */
static struct sunxi_i2c_platform_data sunxi_twi1_pdata[] = {
	{
		.bus_num   = 1,
    	.frequency = I2C1_TRANSFER_SPEED,
	},
};

static struct resource sunxi_twi1_resources[] = {
	{
		.start	= TWI1_BASE_ADDR_START,
		.end	= TWI1_BASE_ADDR_END,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= SW_INT_IRQNO_TWI1,
		.end	= SW_INT_IRQNO_TWI1,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device sunxi_twi1_device = {
	.name		= "sunxi-i2c",
	.id		    = 1,
	.resource	= sunxi_twi1_resources,
	.num_resources	= ARRAY_SIZE(sunxi_twi1_resources),
	.dev = {
		.platform_data = sunxi_twi1_pdata,
	},
};

/* twi2 */
static struct sunxi_i2c_platform_data sunxi_twi2_pdata[] = {
	{
		.bus_num   = 2,
    	.frequency = I2C2_TRANSFER_SPEED,
	},
};

static struct resource sunxi_twi2_resources[] = {
	{
		.start	= TWI2_BASE_ADDR_START,
		.end	= TWI2_BASE_ADDR_END,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= SW_INT_IRQNO_TWI2,
		.end	= SW_INT_IRQNO_TWI2,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device sunxi_twi2_device = {
	.name		= "sunxi-i2c",
	.id		    = 2,
	.resource	= sunxi_twi2_resources,
	.num_resources	= ARRAY_SIZE(sunxi_twi2_resources),
	.dev = {
		.platform_data = sunxi_twi2_pdata,
	},
};

static struct resource sunxi_pmu_resources[] = {
	{
		.start	= SW_INT_IRQNO_PLE_PFM,
		.end	= SW_INT_IRQNO_PLE_PFM,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device sunxi_pmu_device = {
	.name		= "arm-pmu",
	.id		= ARM_PMU_DEVICE_CPU,
	.resource	= sunxi_pmu_resources,
	.num_resources	= ARRAY_SIZE(sunxi_pmu_resources),
};


#if defined(CONFIG_MALI_DRM) || defined(CONFIG_MALI_DRM_MODULE)
static struct platform_device sunxi_device_mali_drm = {
	.name = "mali_drm",
	.id   = -1,
};
#endif

/* ram console */
#ifdef CONFIG_ANDROID_RAM_CONSOLE

static struct resource sun4i_ramconsole_resources[] = {
	{
		.flags  = IORESOURCE_MEM,
		.start  = SUNXI_RAMCONSOLE_START,
		.end    = SUNXI_RAMCONSOLE_START + SUNXI_RAMCONSOLE_SIZE - 1,
	},
};

static struct ram_console_platform_data sun4i_ramconsole_pdata;

static struct platform_device sun4i_ramconsole = {
	.name           = "ram_console",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(sun4i_ramconsole_resources),
	.resource       = sun4i_ramconsole_resources,
	.dev            = {
		.platform_data = &sun4i_ramconsole_pdata,
	},
};
#endif

/* ahci */
#ifdef CONFIG_ARCH_SUN4I
static int sunxi_ahci_phy_init(u32 base)
{
	u32 value;
	int timeout;
	
	writel(0, base + SW_AHCI_RWCR_OFFSET);
	mdelay(5);
	
	value = readl(base + SW_AHCI_PHYCS1R_OFFSET);
	value |= (0x1 <<19); 
	writel(value, base + SW_AHCI_PHYCS1R_OFFSET);
		
	value = readl(base + SW_AHCI_PHYCS0R_OFFSET);
	value |= 0x1 << 23; 
	value |= 0x1 << 18; 
	value &= ~(0x7 << 24); 
	value |= 0x5 << 24; 
	writel(value, base + SW_AHCI_PHYCS0R_OFFSET);
	
	value = readl(base + SW_AHCI_PHYCS1R_OFFSET);
	value &= ~(0x3 << 16); 
	value |= (0x2 << 16); 	
	value &= ~(0x1f << 8);
	value |= (6 << 8); 	
	value &= ~(0x3 << 6); 
	value |= (2 << 6); 	
	writel(value, base + SW_AHCI_PHYCS1R_OFFSET);
	
	value = readl(base + SW_AHCI_PHYCS1R_OFFSET);
	value |= (0x1 << 28); 
	value |= (0x1 << 15); 
	writel(value, base + SW_AHCI_PHYCS1R_OFFSET);
	
	value = readl(base + SW_AHCI_PHYCS1R_OFFSET);
	value &= ~(0x1 << 19); 
	writel(value, base + SW_AHCI_PHYCS1R_OFFSET);
		
	value = readl(base + SW_AHCI_PHYCS0R_OFFSET);
	value &= ~(0x7 << 20); 
	value |= (0x03 << 20);  
	writel(value, base + SW_AHCI_PHYCS0R_OFFSET);
		
	value = readl(base + SW_AHCI_PHYCS2R_OFFSET);
	value &= ~(0x1f << 5); 
	value |= (0x19 << 5);  
	writel(value, base + SW_AHCI_PHYCS2R_OFFSET);
		
	mdelay(5);
	
	value = readl(base + SW_AHCI_PHYCS0R_OFFSET);
	value |= (0x1 << 19); 
	writel(value, base + SW_AHCI_PHYCS0R_OFFSET);
		
	timeout = 0x100000;
	do {
		value = readl(base + SW_AHCI_PHYCS0R_OFFSET);
		timeout --;
		if(!timeout) break;
	}while((value & (0x7 << 28)) != (0x02 << 28)); 
	
	if(!timeout) {
		printk("SATA AHCI Phy Power Failed!!\n");
	}
	
	value = readl(base + SW_AHCI_PHYCS2R_OFFSET);
	value |= (0x1 << 24); 
	writel(value, base + SW_AHCI_PHYCS2R_OFFSET);
	
	timeout = 0x100000;
	do{
		value = readl(base + SW_AHCI_PHYCS2R_OFFSET);
		timeout --;
		if(!timeout) break;
	}while(value & (0x1 << 24)); 
	
	if(!timeout) {
		printk("SATA AHCI Phy Calibration Failed!!\n");
	}
	
	mdelay(15);
	writel(0x07, base + SW_AHCI_RWCR_OFFSET);
	
	return 0;		
}

static int sunxi_ahci_start(struct device *dev, void __iomem *addr)
{
	struct clk *hclk;
	struct clk *mclk;
	int ctrl = 0;
	int rc = 0;
		
	script_parser_fetch("sata_para", "sata_used", &ctrl, sizeof(int));
	if(!ctrl) {
		dev_err(dev, "AHCI is disable\n");
		rc = -EINVAL;
    	goto err2;
	}	

	/*Enable mclk and hclk for AHCI*/
	mclk = clk_get(dev, "sata");
	if (IS_ERR(mclk))
    {
    	dev_err(dev, "Error to get module clk for AHCI\n");
    	rc = -EINVAL;
    	goto err2;
    }
    
	hclk = clk_get(dev, "ahb_sata");
	if (IS_ERR(hclk))
	{
		dev_err(dev, "Error to get ahb clk for AHCI\n");
    	rc = -EINVAL;
    	goto err1;
	}
	
	/*Enable SATA Clock in SATA PLL*/
	clk_enable(mclk);
	clk_enable(hclk);
	
	sunxi_ahci_phy_init((u32)addr);
	
	clk_put(hclk);
err1:
	clk_put(mclk);	
err2:	
	return rc;
}

static void sunxi_ahci_stop(struct device *dev)
{
	struct clk *hclk;
	struct clk *mclk;
	int rc = 0;
		
	mclk = clk_get(dev, "sata");
	if (IS_ERR(mclk)) {
    	dev_err(dev, "Error to get module clk for AHCI\n");
    	rc = -EINVAL;
    	goto err2;
    }
    
	hclk = clk_get(dev, "ahb_sata");
	if (IS_ERR(hclk)) {
		dev_err(dev, "Error to get ahb clk for AHCI\n");
    	rc = -EINVAL;
    	goto err1;
	}
	
	/*Disable mclk and hclk for AHCI*/
	clk_disable(mclk);
	clk_disable(hclk);
	clk_put(hclk);
err1:
	clk_put(mclk);
err2:
	return;
}

static struct ahci_platform_data sunxi_ahci_pdata = {
	.init = sunxi_ahci_start,
	.exit = sunxi_ahci_stop,
};

static struct resource sunxi_ahci_resources[] = {
	{
		.start	= SW_PA_SATA_IO_BASE,
		.end	= SW_PA_SATA_IO_BASE + 0xfff,
		.flags	= IORESOURCE_MEM,
	},
	{
    	.start	= SW_INT_IRQNO_SATA,
    	.end	= SW_INT_IRQNO_SATA,
    	.flags	= IORESOURCE_IRQ,
    },
};

struct platform_device sunxi_ahci_device = {
	.name		= "ahci",
	.id		    = 0,
	.resource	= sunxi_ahci_resources,
	.num_resources	= ARRAY_SIZE(sunxi_ahci_resources),
	.dev = {
		.platform_data = &sunxi_ahci_pdata,
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};
#endif

static struct resource sunxi_uart0_resource[] = {
    {
        .start = SW_PA_UART0_IO_BASE,      
        .end   = SW_PA_UART0_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART0, 
        .end   = SW_INT_IRQNO_UART0,           
        .flags = IORESOURCE_IRQ
    }, 
};

static struct resource sunxi_uart1_resource[] = {
    {
        .start = SW_PA_UART1_IO_BASE,      
        .end   = SW_PA_UART1_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART1, 
        .end   = SW_INT_IRQNO_UART1,           
        .flags = IORESOURCE_IRQ
    }, 
};

static struct resource sunxi_uart2_resource[] = {
    {
        .start = SW_PA_UART2_IO_BASE,      
        .end   = SW_PA_UART2_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART2, 
        .end   = SW_INT_IRQNO_UART2,           
        .flags = IORESOURCE_IRQ
    }, 
};

static struct resource sunxi_uart3_resource[] = {
    {
        .start = SW_PA_UART3_IO_BASE,      
        .end   = SW_PA_UART3_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART3, 
        .end   = SW_INT_IRQNO_UART3,           
        .flags = IORESOURCE_IRQ
    }, 
};

#ifdef CONFIG_ARCH_SUN4I
static struct resource sunxi_uart4_resource[] = {
    {
        .start = SW_PA_UART4_IO_BASE,      
        .end   = SW_PA_UART4_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART4, 
        .end   = SW_INT_IRQNO_UART4,           
        .flags = IORESOURCE_IRQ
    }, 
};

static struct resource sunxi_uart5_resource[] = {
    {
        .start = SW_PA_UART5_IO_BASE,      
        .end   = SW_PA_UART5_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART5, 
        .end   = SW_INT_IRQNO_UART5,           
        .flags = IORESOURCE_IRQ
    }, 
};

static struct resource sunxi_uart6_resource[] = {
    {
        .start = SW_PA_UART6_IO_BASE,      
        .end   = SW_PA_UART6_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART6, 
        .end   = SW_INT_IRQNO_UART6,           
        .flags = IORESOURCE_IRQ
    }, 
};

static struct resource sunxi_uart7_resource[] = {
    {
        .start = SW_PA_UART7_IO_BASE,      
        .end   = SW_PA_UART7_IO_BASE + 0x3ff, 
        .flags = IORESOURCE_MEM
    },
    {
        .start = SW_INT_IRQNO_UART7, 
        .end   = SW_INT_IRQNO_UART7,           
        .flags = IORESOURCE_IRQ
    }, 
};
#endif

struct platform_device sunxi_uart0_device = {
    .name           = "sunxi-uart", 
    .id             = 0, 
    .num_resources  = ARRAY_SIZE(sunxi_uart0_resource), 
    .resource       = sunxi_uart0_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

struct platform_device sunxi_uart1_device = {
    .name           = "sunxi-uart", 
    .id             = 1, 
    .num_resources  = ARRAY_SIZE(sunxi_uart1_resource), 
    .resource       = sunxi_uart1_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

struct platform_device sunxi_uart2_device = {
    .name           = "sunxi-uart", 
    .id             = 2, 
    .num_resources  = ARRAY_SIZE(sunxi_uart2_resource), 
    .resource       = sunxi_uart2_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

struct platform_device sunxi_uart3_device = {
    .name           = "sunxi-uart", 
    .id             = 3, 
    .num_resources  = ARRAY_SIZE(sunxi_uart3_resource), 
    .resource       = sunxi_uart3_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

#ifdef CONFIG_ARCH_SUN4I
struct platform_device sunxi_uart4_device = {
    .name           = "sunxi-uart", 
    .id             = 4, 
    .num_resources  = ARRAY_SIZE(sunxi_uart4_resource), 
    .resource       = sunxi_uart4_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

struct platform_device sunxi_uart5_device = {
    .name           = "sunxi-uart", 
    .id             = 5, 
    .num_resources  = ARRAY_SIZE(sunxi_uart5_resource), 
    .resource       = sunxi_uart5_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

struct platform_device sunxi_uart6_device = {
    .name           = "sunxi-uart", 
    .id             = 6, 
    .num_resources  = ARRAY_SIZE(sunxi_uart6_resource), 
    .resource       = sunxi_uart6_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};

struct platform_device sunxi_uart7_device = {
    .name           = "sunxi-uart", 
    .id             = 7, 
    .num_resources  = ARRAY_SIZE(sunxi_uart7_resource), 
    .resource       = sunxi_uart7_resource,     
	.dev            = {
		.coherent_dma_mask	= DMA_32BIT_MASK,
	},
};
#endif

static struct platform_device *sw_pdevs[] __initdata = {
#if 0
	&debug_uart,
#endif
	&sw_pdev_dmac,
	&sw_pdev_nand,
	&sunxi_uart0_device,
	&sunxi_uart1_device,
	&sunxi_uart2_device,
	&sunxi_uart3_device,
#ifdef CONFIG_ARCH_SUN4I
	&sunxi_uart4_device,
	&sunxi_uart5_device,
	&sunxi_uart6_device,
	&sunxi_uart7_device,
#endif
	&sunxi_twi0_device,
	&sunxi_twi1_device,
	&sunxi_twi2_device,
	&sunxi_pmu_device,
#ifdef CONFIG_ARCH_SUN4I
	&sunxi_ahci_device,
#endif
#if defined(CONFIG_MALI_DRM) || defined(CONFIG_MALI_DRM_MODULE)
	&sunxi_device_mali_drm,
#endif
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	&sun4i_ramconsole,
#endif
};

void __init sw_pdev_init(void)
{
	platform_add_devices(sw_pdevs, ARRAY_SIZE(sw_pdevs));
}
