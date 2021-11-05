/*
 * Copyright (C) 2021 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <fwupdplugin.h>

#include "fu-mtd-recovery-device.h"

static void
fu_plugin_mtd_recovery_init(FuPlugin *plugin)
{
	FuContext *ctx = fu_plugin_get_context(plugin);
	fu_plugin_add_flag(plugin, FWUPD_PLUGIN_FLAG_REQUIRE_HWID);
	fu_context_add_quirk_key(ctx, "MtdRecoveryGpioNumber");
	fu_context_add_quirk_key(ctx, "MtdRecoveryKernelDriver");
	fu_context_add_quirk_key(ctx, "MtdRecoveryBindId");
}

static gboolean
fu_plugin_mtd_recovery_coldplug(FuPlugin *plugin, GError **error)
{
	FuContext *ctx = fu_plugin_get_context(plugin);
	const gchar *product;
	const gchar *vendor;
	g_autoptr(FuDevice) device = NULL;

	device = g_object_new(FU_TYPE_MTD_RECOVERY_DEVICE, "context", ctx, NULL);

	/* set vendor ID as the baseboard vendor */
	vendor = fu_context_get_hwid_value(ctx, FU_HWIDS_KEY_BASEBOARD_MANUFACTURER);
	if (vendor != NULL) {
		g_autofree gchar *vendor_id = g_strdup_printf("DMI:%s", vendor);
		fu_device_add_vendor_id(device, vendor_id);
	}

	/* set instance ID as the baseboard vendor and product */
	product = fu_context_get_hwid_value(ctx, FU_HWIDS_KEY_BASEBOARD_PRODUCT);
	if (vendor != NULL) {
		g_autofree gchar *instance_id0 = NULL;
		g_autofree gchar *vendor_safe = g_strdup(vendor);

		/* add more precise ID first */
		g_strdelimit(vendor_safe, " /\\\"", '-');
		if (product != NULL) {
			g_autofree gchar *instance_id1 = NULL;
			g_autofree gchar *product_safe = g_strdup(product);
			instance_id1 = g_strdup_printf("MTD\\VEN_%s&DEV_%s", vendor, product_safe);
			fu_device_add_instance_id(device, instance_id1);
		}

		/* use vendor only for quirks */
		instance_id0 = g_strdup_printf("MTD\\VEN_%s", vendor_safe);
		fu_device_add_instance_id_full(device,
					       instance_id0,
					       FU_DEVICE_INSTANCE_FLAG_ONLY_QUIRKS);
	}

	/* manually convert the IDs */
	if (!fu_device_setup(device, error))
		return FALSE;

	/* success */
	fu_plugin_device_add(plugin, device);
	return TRUE;
}

static void
fu_plugin_mtd_recovery_set_proxy(FuPlugin *plugin, FuDevice *device)
{
	GPtrArray *devices = fu_plugin_get_devices(plugin);
	for (guint i = 0; i < devices->len; i++) {
		FuDevice *device_tmp = g_ptr_array_index(devices, i);
		g_debug("using %s as proxy to %s",
			fu_device_get_id(device),
			fu_device_get_id(device_tmp));
		fu_device_set_proxy(device_tmp, device);
	}
}

/* a MTD device just showed up, probably as the result of FuMtdRecoveryDevice->detach() */
static void
fu_plugin_mtd_recovery_device_registered(FuPlugin *plugin, FuDevice *device)
{
	if (g_strcmp0(fu_device_get_plugin(device), "mtd") == 0 &&
	    fu_device_has_flag(device, FWUPD_DEVICE_FLAG_UPDATABLE)) {
		fu_plugin_mtd_recovery_set_proxy(plugin, device);
		fu_device_inhibit(device, "proxy-to-recovery", "Proxy for recovery device");
	}
}

/* a MTD device got removed, probably as the result of FuMtdRecoveryDevice->attach() */
static gboolean
fu_plugin_mtd_recovery_backend_device_removed(FuPlugin *plugin, FuDevice *device, GError **error)
{
	if (g_strcmp0(fu_device_get_plugin(device), "mtd") == 0)
		fu_plugin_mtd_recovery_set_proxy(plugin, NULL);
	return TRUE;
}

void
fu_plugin_init_vfuncs(FuPluginVfuncs *vfuncs)
{
	vfuncs->build_hash = FU_BUILD_HASH;
	vfuncs->init = fu_plugin_mtd_recovery_init;
	vfuncs->coldplug = fu_plugin_mtd_recovery_coldplug;
	vfuncs->device_registered = fu_plugin_mtd_recovery_device_registered;
	vfuncs->backend_device_removed = fu_plugin_mtd_recovery_backend_device_removed;
}
