// SPDX-License-Identifier: GPL-2.0+
#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/rwlock_types.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Claudio Cabral <cl@udio.co>");
MODULE_DESCRIPTION("This module talks to Morty");

#define FOLDER 0
#define ID 1
#define JIFFIES 2
#define FOO 3
#define FS_SIZE 4

DEFINE_RWLOCK(foo_lock);

static const char *name = "ccabral";
static char g_buffer[PAGE_SIZE];
static ssize_t g_buffer_size;

#define NAME_SIZE (sizeof(name) - 1)

ssize_t id_read(struct file *filp, char __user *buf, size_t count,
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

ssize_t id_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	char internal_buffer[NAME_SIZE];

	if (count > NAME_SIZE - *f_pos)
		return -EINVAL;
	if (copy_from_user(internal_buffer, buf, count) != 0)
		return -EINVAL;
	if (strncmp(internal_buffer, name + *f_pos, count) == 0) {
		*f_pos += count;
		return count;
	}
	return -EINVAL;
}

ssize_t foo_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	ssize_t ret;

	read_lock(&foo_lock);
	ret = simple_read_from_buffer(buf, count, f_pos,
			g_buffer, sizeof(g_buffer));
	read_unlock(&foo_lock);
	return ret;
}

ssize_t foo_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	ssize_t i;
	ssize_t tmp;

	write_lock(&foo_lock);
	i = 0;
	while (count > PAGE_SIZE) {
		tmp = simple_write_to_buffer(g_buffer, PAGE_SIZE, f_pos,
					     buf, PAGE_SIZE);
		if (tmp <= 0)
			goto ret;
		i += tmp;
		*f_pos = 0;
		count -= PAGE_SIZE;
	}
	tmp = simple_write_to_buffer(g_buffer, PAGE_SIZE, f_pos, buf, count);
	if (tmp <= 0)
		goto ret;
	g_buffer_size = tmp;
	tmp += i;
ret:
	write_unlock(&foo_lock);
	return tmp;
}

static struct dentry *fs[4];
static const struct file_operations fops[4] = {
	[ID] = {
		.owner = THIS_MODULE,
		.read = id_read,
		.write = id_write
	},
	[FOO] = {
		.owner = THIS_MODULE,
		.read = foo_read,
		.write = foo_write
	}
};

static __init int init_debug(void)
{
	int i;

	memset(g_buffer, 0, sizeof(g_buffer));
	fs[FOLDER] = debugfs_create_dir("fortytwo", NULL);
	if (fs[FOLDER] == NULL || fs[FOLDER] == ERR_PTR(-ENODEV))
		return -ENODEV;
	fs[ID] = debugfs_create_file("id",
			0666, fs[FOLDER], NULL, &fops[ID]);
	fs[JIFFIES] = debugfs_create_size_t("jiffies", 0444,
			fs[FOLDER], (size_t *)&jiffies);
	fs[FOO] = debugfs_create_file_size("foo",
			0644, fs[FOLDER], NULL, &fops[FOO], PAGE_SIZE);
	for (i = 0; i < FS_SIZE; ++i) {
		if (fs[i] == ERR_PTR(-ENODEV) || fs[i] == NULL) {
			debugfs_remove_recursive(fs[FOLDER]);
			for (i = 0; i < FS_SIZE; ++i)
				fs[i] = NULL;
			return -ENODEV;
		}
	}
	return 0;
}

static __exit void exit_debug(void)
{
	pr_info("Cleaning up module.");
	debugfs_remove_recursive(fs[FOLDER]);
}

module_init(init_debug);
module_exit(exit_debug);
