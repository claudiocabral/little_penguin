// SPDX-License-Identifier: GPL-2.0+
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL v2");

static __init int do_work(int my_int)
{
	int x;
	int z;

	for (x = 0; x < my_int; ++x)
		udelay(10);
	if (my_int > 10)
		pr_info("We slept a long time!");
	z = x * my_int;
	return z;
}

static __init int my_init(void)
{
	do_work(10);
	return 0;
}

static __exit void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);
