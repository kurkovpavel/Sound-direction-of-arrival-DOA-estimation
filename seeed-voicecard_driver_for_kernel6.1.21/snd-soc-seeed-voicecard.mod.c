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

SYMBOL_CRC(seeed_voice_card_register_set_clock, 0xffb30001, "");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd792e0a, "snd_soc_of_parse_tdm_slot" },
	{ 0x842698ae, "asoc_simple_parse_clk" },
	{ 0x13172f12, "devm_kmalloc" },
	{ 0xaffdc854, "of_node_put" },
	{ 0x225ee71d, "of_property_read_variable_u32_array" },
	{ 0x6703e37e, "platform_driver_unregister" },
	{ 0xb22e15d6, "asoc_simple_set_dailink_name" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0xd08f572, "asoc_simple_clean_reference" },
	{ 0x3b2ef9c2, "snd_soc_of_parse_audio_routing" },
	{ 0x17000cb9, "snd_soc_dai_set_sysclk" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0x7cc2f5af, "of_get_next_child" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0x2283c191, "asoc_simple_parse_card_name" },
	{ 0x8fec33e1, "snd_soc_dai_set_bclk_ratio" },
	{ 0x1ddc2631, "of_get_child_by_name" },
	{ 0xd2467102, "asoc_simple_canonicalize_platform" },
	{ 0x52b3a0c2, "_dev_err" },
	{ 0x57bb3f4e, "__of_parse_phandle_with_args" },
	{ 0x4c37d289, "asoc_simple_parse_daifmt" },
	{ 0x9768d427, "of_find_property" },
	{ 0xa83922e7, "of_device_is_available" },
	{ 0x2b39bef1, "snd_soc_of_get_dai_name" },
	{ 0x402d7b48, "asoc_simple_canonicalize_cpu" },
	{ 0xc3c21c78, "__platform_driver_register" },
	{ 0xc0ffeac6, "snd_soc_runtime_calc_hw" },
	{ 0x3c12dfe, "cancel_work_sync" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0x49154f5a, "snd_soc_pm_ops" },
	{ 0x2917bbf6, "snd_soc_of_parse_audio_simple_widgets" },
	{ 0x815588a6, "clk_enable" },
	{ 0xa76652a5, "devm_snd_soc_register_card" },
	{ 0x2d3385d3, "system_wq" },
	{ 0xb077e70a, "clk_unprepare" },
	{ 0x8f80e6e5, "module_layout" },
};

MODULE_INFO(depends, "snd-soc-core,snd-soc-simple-card-utils");

MODULE_ALIAS("of:N*T*Cseeed-voicecard");
MODULE_ALIAS("of:N*T*Cseeed-voicecardC*");

MODULE_INFO(srcversion, "304FD3F1521E990D121703A");
