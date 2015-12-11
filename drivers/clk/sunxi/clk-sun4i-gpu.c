/*
 * Copyright 2015 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/reset-controller.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define SUN4I_A10_GPU_PARENTS	5

#define SUN4I_A10_GPU_GATE_BIT	31
#define SUN4I_A10_GPU_RESET_BIT	30
#define SUN4I_A10_GPU_MUX_MASK	7
#define SUN4I_A10_GPU_MUX_SHIFT	24
#define SUN4I_A10_GPU_DIV_WIDTH	4
#define SUN4I_A10_GPU_DIV_SHIFT	0

struct reset_data {
	void __iomem			*reg;
	spinlock_t			*lock;
	struct reset_controller_dev	rcdev;
};

static DEFINE_SPINLOCK(sun4i_a10_gpu_lock);

static int sun4i_a10_gpu_assert(struct reset_controller_dev *rcdev,
				    unsigned long id)
{
	struct reset_data *data = container_of(rcdev,
					       struct reset_data,
					       rcdev);
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(data->lock, flags);

	reg = readl(data->reg);
	writel(reg & ~BIT(SUN4I_A10_GPU_RESET_BIT), data->reg);

	spin_unlock_irqrestore(data->lock, flags);

	return 0;
}

static int sun4i_a10_gpu_deassert(struct reset_controller_dev *rcdev,
				      unsigned long id)
{
	struct reset_data *data = container_of(rcdev,
						   struct reset_data,
						   rcdev);
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(data->lock, flags);

	reg = readl(data->reg);
	writel(reg | BIT(SUN4I_A10_GPU_RESET_BIT), data->reg);

	spin_unlock_irqrestore(data->lock, flags);

	return 0;
}

static int sun4i_a10_gpu_status(struct reset_controller_dev *rcdev,
				    unsigned long id)
{
	struct reset_data *data = container_of(rcdev,
					       struct reset_data,
					       rcdev);

	return !(readl(data->reg) & BIT(SUN4I_A10_GPU_RESET_BIT));
}

static struct reset_control_ops sun4i_a10_gpu_reset_ops = {
	.assert		= sun4i_a10_gpu_assert,
	.deassert	= sun4i_a10_gpu_deassert,
	.status		= sun4i_a10_gpu_status,
};

static int sun4i_a10_gpu_reset_xlate(struct reset_controller_dev *rcdev,
					 const struct of_phandle_args *spec)
{
	if (WARN_ON(spec->args_count != rcdev->of_reset_n_cells))
		return -EINVAL;

	/* We only have a single reset signal */
	return 0;
}

static void __init sun4i_a10_gpu_setup(struct device_node *node)
{
	const char *parents[SUN4I_A10_GPU_PARENTS];
	const char *clk_name = node->name;
	struct reset_data *reset_data;
	struct clk_divider *div;
	struct clk_gate *gate;
	struct clk_mux *mux;
	void __iomem *reg;
	struct clk *clk;
	int i;

	of_property_read_string(node, "clock-output-names", &clk_name);

	reg = of_io_request_and_map(node, 0, of_node_full_name(node));
	if (IS_ERR(reg)) {
		pr_err("%s: Could not map the clock registers\n", clk_name);
		return;
	}

	for (i = 0; i < SUN4I_A10_GPU_PARENTS; i++)
		parents[i] = of_clk_get_parent_name(node, i);

	mux = kzalloc(sizeof(*mux), GFP_KERNEL);
	if (!mux)
		return;

	mux->reg = reg;
	mux->shift = SUN4I_A10_GPU_MUX_SHIFT;
	mux->mask = SUN4I_A10_GPU_MUX_MASK;
	mux->lock = &sun4i_a10_gpu_lock;

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		goto free_mux;

	gate->reg = reg;
	gate->bit_idx = SUN4I_A10_GPU_GATE_BIT;
	gate->lock = &sun4i_a10_gpu_lock;

	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		goto free_gate;

	div->reg = reg;
	div->shift = SUN4I_A10_GPU_DIV_SHIFT;
	div->width = SUN4I_A10_GPU_DIV_WIDTH;
	div->lock = &sun4i_a10_gpu_lock;

	clk = clk_register_composite(NULL, clk_name,
				     parents, SUN4I_A10_GPU_PARENTS,
				     &mux->hw, &clk_mux_ops,
				     &div->hw, &clk_divider_ops,
				     &gate->hw, &clk_gate_ops,
				     0);
	if (IS_ERR(clk)) {
		pr_err("%s: Couldn't register the clock\n", clk_name);
		goto free_div;
	}

	of_clk_add_provider(node, of_clk_src_simple_get, clk);

	reset_data = kzalloc(sizeof(*reset_data), GFP_KERNEL);
	if (!reset_data)
		goto free_clk;

	reset_data->reg = reg;
	reset_data->lock = &sun4i_a10_gpu_lock;
	reset_data->rcdev.nr_resets = 1;
	reset_data->rcdev.ops = &sun4i_a10_gpu_reset_ops;
	reset_data->rcdev.of_node = node;
	reset_data->rcdev.of_reset_n_cells = 0;
	reset_data->rcdev.of_xlate = &sun4i_a10_gpu_reset_xlate;

	if (reset_controller_register(&reset_data->rcdev)) {
		pr_err("%s: Couldn't register the reset controller\n",
		       clk_name);
		goto free_reset;
	}

	return;

free_reset:
	kfree(reset_data);
free_clk:
	clk_unregister(clk);
free_div:
	kfree(div);
free_gate:
	kfree(gate);
free_mux:
	kfree(mux);
}

CLK_OF_DECLARE(sun4i_a10_gpu, "allwinner,sun4i-a10-gpu-clk",
	       sun4i_a10_gpu_setup);
