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
	{ 0x260335b9, "regcache_cache_only" },
	{ 0x97f71b36, "regcache_sync" },
	{ 0x1f4eee1e, "regmap_write" },
	{ 0xa69f46e5, "devm_request_threaded_irq" },
	{ 0xb2052d98, "gpiod_set_value" },
	{ 0x225ee71d, "of_property_read_variable_u32_array" },
	{ 0xd57d2443, "regmap_get_max_register" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0xd29bc4ca, "i2c_match_id" },
	{ 0xe0259c51, "devm_input_allocate_device" },
	{ 0x2bf839e4, "gpiod_to_irq" },
	{ 0x829db510, "input_unregister_device" },
	{ 0x37a0cba, "kfree" },
	{ 0xc3055d20, "usleep_range_state" },
	{ 0x6b755970, "devm_gpiod_get_optional" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x55cd4bf3, "devm_snd_soc_register_component" },
	{ 0x92997ed8, "_printk" },
	{ 0xcc41c770, "input_register_device" },
	{ 0xd2f72016, "__devm_regmap_init_i2c" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0xb2fcb56d, "queue_delayed_work_on" },
	{ 0xa4252990, "devm_free_irq" },
	{ 0xffb30001, "seeed_voice_card_register_set_clock" },
	{ 0x440cbd26, "i2c_register_driver" },
	{ 0xc95c77e3, "snd_soc_info_volsw" },
	{ 0xc7250311, "snd_soc_unregister_component" },
	{ 0x52b3a0c2, "_dev_err" },
	{ 0xc62a4969, "gpiod_direction_input" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0xcf2d8d06, "input_set_capability" },
	{ 0x726795c7, "snd_soc_add_component_controls" },
	{ 0xd0f9404d, "sysfs_create_group" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0x99a2d68b, "_dev_warn" },
	{ 0xfe788aa9, "snd_soc_dapm_add_routes" },
	{ 0x7297e669, "input_event" },
	{ 0xa388e04c, "sysfs_remove_group" },
	{ 0x30a9f1c9, "snd_soc_get_volsw" },
	{ 0x2a1fc714, "regmap_read" },
	{ 0xe4fcb55a, "snd_soc_put_volsw" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x5f4133d3, "snd_soc_dapm_new_controls" },
	{ 0x3c12dfe, "cancel_work_sync" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0x5f6ad7b0, "regcache_cache_bypass" },
	{ 0x8631c9ba, "alt_cb_patch_nops" },
	{ 0x38aad985, "regmap_update_bits_base" },
	{ 0x26d9d21b, "kmalloc_trace" },
	{ 0x424d82fe, "i2c_del_driver" },
	{ 0x1322203e, "snd_soc_dai_active" },
	{ 0xf9a482f9, "msleep" },
	{ 0x670ae423, "kmalloc_caches" },
	{ 0x2d3385d3, "system_wq" },
	{ 0x8f80e6e5, "module_layout" },
};

MODULE_INFO(depends, "snd-soc-core,regmap-i2c,snd-soc-seeed-voicecard");

MODULE_ALIAS("i2c:ac108_0");
MODULE_ALIAS("i2c:ac108_1");
MODULE_ALIAS("i2c:ac108_2");
MODULE_ALIAS("i2c:ac108_3");
MODULE_ALIAS("i2c:ac101");
MODULE_ALIAS("of:N*T*Cx-power,ac108_0");
MODULE_ALIAS("of:N*T*Cx-power,ac108_0C*");
MODULE_ALIAS("of:N*T*Cx-power,ac108_1");
MODULE_ALIAS("of:N*T*Cx-power,ac108_1C*");
MODULE_ALIAS("of:N*T*Cx-power,ac108_2");
MODULE_ALIAS("of:N*T*Cx-power,ac108_2C*");
MODULE_ALIAS("of:N*T*Cx-power,ac108_3");
MODULE_ALIAS("of:N*T*Cx-power,ac108_3C*");
MODULE_ALIAS("of:N*T*Cx-power,ac101");
MODULE_ALIAS("of:N*T*Cx-power,ac101C*");

MODULE_INFO(srcversion, "62523D38084365BA974C632");
