/*
 * t19x-nvlink-endpt-debugfs.c:
 * This file adds various debugfs nodes for the Tegra NVLINK controller.
 *
 * Copyright (c) 2017-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "t19x-nvlink-endpt.h"
#include "nvlink-hw.h"
#include <linux/uaccess.h>

static int nvlink_refclk_rate_file_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;

	return 0;
}

static ssize_t nvlink_refclk_rate_file_read(struct file *file,
				char __user *ubuf,
				size_t count, loff_t *offp)
{
	struct nvlink_device *nvlink = file->private_data;
	char buf[5];
	int str_len;

	switch (nvlink->refclk) {
	case NVLINK_REFCLK_150:
		strcpy(buf, "150");
		break;
	case NVLINK_REFCLK_156:
	default:
		strcpy(buf, "156");
		break;
	}
	strcat(buf, "\n");
	str_len = strlen(buf);
	return simple_read_from_buffer(ubuf, count, offp, buf, str_len);
}

static ssize_t nvlink_refclk_rate_file_write(struct file *file,
				const char __user *ubuf,
				size_t count, loff_t *offp)
{
	struct nvlink_device *nvlink = file->private_data;
	char tmp[3];
	int ret;
	enum init_state state = NVLINK_DEV_OFF;

	ret = nvlink_get_init_state(nvlink, &state);
	if (ret < 0) {
		nvlink_err("Error retriving the init state!");
		return ret;
	}
	if (NVLINK_DEV_OFF != state)
		return -EINVAL;

	ret = copy_from_user(tmp, ubuf, count);

	if (!strncmp(tmp, "150", 3))
		nvlink->refclk = NVLINK_REFCLK_150;
	else if (!strncmp(tmp, "156", 3))
                nvlink->refclk = NVLINK_REFCLK_156;
	else
		return -EINVAL;
	return count;
}

static const struct file_operations  nvlink_refclk_rate_fops = {
	.open	= nvlink_refclk_rate_file_open,
	.read	= nvlink_refclk_rate_file_read,
	.write	= nvlink_refclk_rate_file_write,
	.owner	= THIS_MODULE,
};

static int nvlink_speedcontrol_file_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;

	return 0;
}

static ssize_t nvlink_speedcontrol_file_read(struct file *file,
					     char __user *ubuf,
					     size_t count,
					     loff_t *offp)
{
	struct nvlink_device *nvlink = file->private_data;
	char buf[4];
	int str_len;

	switch (nvlink->speed) {
	case NVLINK_SPEED_20:
		strcpy(buf, "20");
		break;
	case NVLINK_SPEED_25:
	default:
		strcpy(buf, "25");
		break;
	}
	strcat(buf, "\n");
	str_len = strlen(buf);
	return simple_read_from_buffer(ubuf, count, offp, buf, str_len);
}

static ssize_t nvlink_speedcontrol_file_write(struct file *file,
						const char __user *ubuf,
						size_t count, loff_t *offp)
{
	struct nvlink_device *nvlink = file->private_data;
	char tmp[2];
	int ret;
	enum init_state state = NVLINK_DEV_OFF;

	ret = nvlink_get_init_state(nvlink, &state);
	if (ret < 0) {
		nvlink_err("Error retriving the device state!");
		return ret;
	}
	if (NVLINK_DEV_OFF != state)
		return -EINVAL;

	ret = copy_from_user(tmp, ubuf, count);

	if (!strncmp(tmp, "20", 2))
		nvlink->speed = NVLINK_SPEED_20;
	else if (!strncmp(tmp, "25", 2))
		nvlink->speed = NVLINK_SPEED_25;
	else
		return -EINVAL;
	return count;
}

static const struct file_operations  nvlink_speedcontrol_fops = {
	.open	= nvlink_speedcontrol_file_open,
	.read	= nvlink_speedcontrol_file_read,
	.write	= nvlink_speedcontrol_file_write,
	.owner	= THIS_MODULE,
};

static int nvlink_single_lane_debugfs_init(struct nvlink_device *ndev)
{
	struct tegra_nvlink_device *tdev =
				(struct tegra_nvlink_device *)ndev->priv;
	struct dentry *tegra_sl_debugfs;
	int ret = 0;

	tegra_sl_debugfs = debugfs_create_dir("single_lane_params",
						tdev->tegra_debugfs);
	if (!tegra_sl_debugfs) {
		nvlink_err("Failed to create Tegra NVLINK endpoint driver's"
		" single lane debugfs directory");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_bool("enabled", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.enabled)) {
		nvlink_err("Unable to create debugfs node for sl_enabled");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u16("fb_ic_inc", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.fb_ic_inc)) {
		nvlink_err("Unable to create debugfs node for fb_ic_inc");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u16("lp_ic_inc", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.lp_ic_inc)) {
		nvlink_err("Unable to create debugfs node for lp_ic_inc");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u16("fb_ic_dec", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.fb_ic_dec)) {
		nvlink_err("Unable to create debugfs node for fb_ic_dec");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u16("lp_ic_dec", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.lp_ic_dec)) {
		nvlink_err("Unable to create debugfs node for lp_ic_dec");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u32("enter_thresh", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.enter_thresh)) {
		nvlink_err("Unable to create debugfs node for enter_thresh");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u32("exit_thresh", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.exit_thresh)) {
		nvlink_err("Unable to create debugfs node for exit_thresh");
		ret = -1;
		goto fail;
	}

	if (!debugfs_create_u32("ic_limit", (S_IWUSR | S_IRUGO),
					tegra_sl_debugfs,
					&ndev->link.sl_params.ic_limit)) {
		nvlink_err("Unable to create debugfs node for ic_limit");
		ret = -1;
		goto fail;
	}

fail:
	return ret;
}

void t19x_nvlink_endpt_debugfs_init(struct nvlink_device *ndev)
{
	struct tegra_nvlink_device *tdev =
				(struct tegra_nvlink_device *)ndev->priv;

	if (!nvlink_debugfs) {
		nvlink_err("Root NVLINK debugfs directory doesn't exist");
		goto fail;
	}

	tdev->tegra_debugfs = debugfs_create_dir(NVLINK_DRV_NAME,
						nvlink_debugfs);
	if (!tdev->tegra_debugfs) {
		nvlink_err("Failed to create Tegra NVLINK endpoint driver's"
			" debugfs directory");
		goto fail;
	}

	/* nvlink_rate_config: to switch and set different NVLINK RefCLK rate */
	tdev->tegra_debugfs_file = debugfs_create_file("refclk_rate",
				(S_IWUSR | S_IRUGO), tdev->tegra_debugfs,
				ndev, &nvlink_refclk_rate_fops);
	if (IS_ERR_OR_NULL(tdev->tegra_debugfs_file)) {
		tdev->tegra_debugfs_file = NULL;
		nvlink_dbg("debugfs_create_file() for nvlink_refclk_rate failed");
		goto fail;
	}
	/* nvlink_rate_config: to switch and set different NVLINK Speed Control */
	tdev->tegra_debugfs_file = debugfs_create_file("speed_control",
				(S_IWUSR | S_IRUGO), tdev->tegra_debugfs,
				ndev, &nvlink_speedcontrol_fops);
	if (IS_ERR_OR_NULL(tdev->tegra_debugfs_file)) {
		tdev->tegra_debugfs_file = NULL;
		nvlink_dbg("debugfs_create_file() for nvlink_rate_config failed");
		goto fail;
	}

	if (nvlink_single_lane_debugfs_init(ndev) < 0)
		goto fail;

	return;

fail:
	nvlink_err("Failed to create debugfs nodes");
	debugfs_remove_recursive(tdev->tegra_debugfs);
	tdev->tegra_debugfs = NULL;
}

void t19x_nvlink_endpt_debugfs_deinit(struct nvlink_device *ndev)
{
	struct tegra_nvlink_device *tdev =
				(struct tegra_nvlink_device *)ndev->priv;

	debugfs_remove_recursive(tdev->tegra_debugfs);
	tdev->tegra_debugfs = NULL;
}
