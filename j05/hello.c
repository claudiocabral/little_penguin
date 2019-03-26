// SPDX-License-Identifier: GPL-2.0+
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Claudio Cabral <cl@udio.co>");
MODULE_DESCRIPTION("This module talks to Morty");

static const char *name = "ccabral";

#define NAME_SIZE (sizeof(name) - 1)
//#define NAME_SIZE (strlen(name))

ssize_t hello_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	ssize_t size = NAME_SIZE - *f_pos;

	count = count < size
		? count
		: size;
	if (copy_to_user(buf, name + *f_pos, count) != 0)
		return -EINVAL;
	*f_pos += count;
	return count;
}

ssize_t hello_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	char internal_buffer[NAME_SIZE + 1];

	if (count > NAME_SIZE)
		return -EINVAL;
	memset(internal_buffer, 0, NAME_SIZE + 1);
	if (copy_from_user(internal_buffer, buf, count)!= 0)
		return -EINVAL;
	return strncmp(internal_buffer, name, NAME_SIZE) == 0
		? NAME_SIZE
		: -EINVAL;
}

static const struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write
};

static struct miscdevice fortytwo = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fortytwo",
	.fops = &hello_fops
};

static __init int init_hello(void)
{
	pr_info("Hello World !\n");
	return misc_register(&fortytwo);
}

static __exit void exit_hello(void)
{
	pr_info("Cleaning up module.\n");
	misc_deregister(&fortytwo);
}

module_init(init_hello);
module_exit(exit_hello);
