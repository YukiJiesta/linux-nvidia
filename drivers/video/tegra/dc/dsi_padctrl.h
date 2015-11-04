/*
 * drivers/video/tegra/dc/dsi_padctrl.h
 * 
 * Copyright (c) 2015, NVIDIA CORPORATION, All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/of.h>
#include <mach/dc.h>

struct tegra_dsi_padctrl {
	struct reset_control *reset;
	struct resource *base_res;
	void __iomem *base_addr;
};

/* Defined in dsi_padctrl.c and used in dsi.c */
struct tegra_dsi_padctrl *tegra_dsi_padctrl_init(struct tegra_dc *dc);
/* Defined in dsi_padctrl.c and used in dsi.c */
void tegra_dsi_padctrl_shutdown(struct tegra_dc *dc);
/* Defined in dsi_padctrl.c and used in dsi.c */
void tegra_dsi_padctrl_enable(struct tegra_dsi_padctrl *dsi_padctrl);
