#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8001e9b1, "module_layout" },
	{ 0xc55d963b, "kmem_cache_alloc" },
	{ 0xe3259ae8, "kmalloc_caches" },
	{ 0x95d7d942, "__register_chrdev" },
	{ 0x5541ea93, "on_each_cpu" },
	{ 0x37a0cba, "kfree" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x425456c9, "cpu_cache" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x587df050, "outer_cache" },
	{ 0x27e1a049, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "832E1303EE9B59385FD0594");
