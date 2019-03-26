// SPDX-License-Identifier: GPL-2.0+
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/rwlock_types.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Reverses input written to itself");

static ssize_t reverse_read(struct file *fp, char __user *user,
		size_t size, loff_t *offs);

static ssize_t reverse_write(struct file *fp, const char __user *user,
		size_t size, loff_t *offs);

static const struct file_operations reverse_fops = {
	.owner = THIS_MODULE,
	.read = &reverse_read,
	.write = &reverse_write
};

static struct miscdevice reverse_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &reverse_fops
};

DEFINE_RWLOCK(reverse_lock);
static char str[PAGE_SIZE];
static char rev[PAGE_SIZE];
static size_t str_size;

static int __init reverse_init(void)
{
	return misc_register(&reverse_device);
}

static void __exit reverse_cleanup(void)
{
	misc_deregister(&reverse_device);
}

static ssize_t reverse_read(struct file *fp, char __user *user, size_t size,
		loff_t *offs)
{
	size_t tmp;
	size_t i = 0;
	ssize_t ret;

	read_lock(&reverse_lock);
	size = size < str_size
		? size
		: str_size;
	tmp = size - 1;
	while (i < size)
		rev[tmp--] = str[i++];

	ret = simple_read_from_buffer(user, size, offs, rev, str_size);
	read_unlock(&reverse_lock);
	return ret;
}

static ssize_t reverse_write(struct file *fp, const char __user *user,
		size_t size, loff_t *offs)
{
	write_lock(&reverse_lock);
	str_size = simple_write_to_buffer(str, PAGE_SIZE, offs, user, size);
	write_unlock(&reverse_lock);
	return str_size;
}

module_init(reverse_init);
module_exit(reverse_cleanup);
