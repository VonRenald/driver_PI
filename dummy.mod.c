#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
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
	{ 0x826b8746, "module_layout" },
	{ 0xb7744793, "kmem_cache_alloc_trace" },
	{ 0xbe301f03, "kmalloc_caches" },
	{ 0xaa1f0d83, "gpiod_get_raw_value" },
	{ 0xc3055d20, "usleep_range_state" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xb77942ad, "gpiod_direction_output_raw" },
	{ 0x51122247, "gpiod_direction_input" },
	{ 0xf7d5fff7, "gpio_to_desc" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x5105d7f, "cdev_del" },
	{ 0x8e6ad741, "cdev_add" },
	{ 0x9d4aa33a, "cdev_init" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x2cfde9a2, "warn_slowpath_fmt" },
	{ 0x2d6fcc06, "__kmalloc" },
	{ 0x37a0cba, "kfree" },
	{ 0x92997ed8, "_printk" },
	{ 0x5f754e5a, "memset" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xff178f6, "__aeabi_idivmod" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "D448FE291F72AA69CFB6F56");