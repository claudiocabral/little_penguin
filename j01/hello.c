// SPDX-License-Identifier: GPL-2.0+
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Claudio Cabral <cl@udio.co>");
MODULE_DESCRIPTION("This module talks to Morty");

static __init int init_hello(void)
{
	pr_info("Hellow World !");
	return 0;
}

static __exit void exit_hello(void)
{
	pr_info("Cleaning up module.");
}

module_init(init_hello);
module_exit(exit_hello);
