#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x97f71b36, "regcache_sync" },
	{ 0x1f4eee1e, "regmap_write" },
	{ 0x13172f12, "devm_kmalloc" },
	{ 0x3e66a604, "snd_soc_component_read" },
	{ 0xea9e8254, "snd_soc_put_enum_double" },
	{ 0xe712db14, "devm_clk_get" },
	{ 0x1fe124b4, "snd_soc_component_write" },
	{ 0x55cd4bf3, "devm_snd_soc_register_component" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0x92997ed8, "_printk" },
	{ 0xd2f72016, "__devm_regmap_init_i2c" },
	{ 0xaa4c7a4c, "snd_soc_get_enum_double" },
	{ 0x440cbd26, "i2c_register_driver" },
	{ 0xc95c77e3, "snd_soc_info_volsw" },
	{ 0x9641e934, "snd_ctl_boolean_mono_info" },
	{ 0xc7250311, "snd_soc_unregister_component" },
	{ 0x52b3a0c2, "_dev_err" },
	{ 0x439dd703, "snd_soc_dapm_put_volsw" },
	{ 0x9768d427, "of_find_property" },
	{ 0x726795c7, "snd_soc_add_component_controls" },
	{ 0xfe788aa9, "snd_soc_dapm_add_routes" },
	{ 0xed2ef9f6, "snd_soc_info_enum_double" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x30a9f1c9, "snd_soc_get_volsw" },
	{ 0xe4fcb55a, "snd_soc_put_volsw" },
	{ 0x81fa5ef9, "snd_soc_component_update_bits" },
	{ 0x5f4133d3, "snd_soc_dapm_new_controls" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0x38aad985, "regmap_update_bits_base" },
	{ 0x31adc9e7, "snd_soc_dapm_get_volsw" },
	{ 0x424d82fe, "i2c_del_driver" },
	{ 0x815588a6, "clk_enable" },
	{ 0xeb711ae7, "snd_soc_params_to_bclk" },
	{ 0xf9a482f9, "msleep" },
	{ 0xe56a9336, "snd_pcm_format_width" },
	{ 0xb077e70a, "clk_unprepare" },
	{ 0x8f80e6e5, "module_layout" },
};

MODULE_INFO(depends, "snd-soc-core,regmap-i2c,snd,snd-pcm");

MODULE_ALIAS("i2c:wm8960");
MODULE_ALIAS("of:N*T*Cwlf,wm8960");
MODULE_ALIAS("of:N*T*Cwlf,wm8960C*");

MODULE_INFO(srcversion, "6BCE1F543E2B070F1F5C430");
