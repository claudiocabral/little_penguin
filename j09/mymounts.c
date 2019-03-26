// SPDX-License-Identifier: GPL-2.0+
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mount.h>
#include <linux/fs_struct.h>
#include <linux/list.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Claudio Cabral <cl@udio.co>");
MODULE_DESCRIPTION("This module talks to Morty");

static struct proc_dir_entry *g_file;
static char g_buffer[PAGE_SIZE];
static char g_current_dir[PAGE_SIZE];
static loff_t g_buffer_size;

static void append_to_buffer(const char *entry, const char *parent, loff_t *pos)
{
	strlcat(g_buffer, entry, PAGE_SIZE);
	strlcat(g_buffer, "\t", PAGE_SIZE);
	if (*parent) {
		strlcat(g_buffer, parent, PAGE_SIZE);
		strlcat(g_buffer, "/", PAGE_SIZE);
	}
	strlcat(g_buffer, entry, PAGE_SIZE);
	strlcat(g_buffer, "\n", PAGE_SIZE);
}

static void recursive_append(struct dentry *entry, char *parent)
{
	struct dentry *current_entry;
	size_t len;

	if (!entry)
		return ;
	append_to_buffer(entry->d_name.name, parent, &g_buffer_size);
	strlcat(parent, entry->d_name.name, PAGE_SIZE);
	len = strlen(parent);
	list_for_each_entry(current_entry,
			&current->fs->root.mnt->mnt_root->d_subdirs, d_child) {
		if (current_entry->d_flags & DCACHE_MOUNTED) {
			parent[len] = 0;
			//recursive_append(current_entry, parent);
		}
	}
}

static void update_mounts(void)
{
	*g_buffer = 0;
	*g_current_dir = 0;
	recursive_append(current->fs->root.mnt->mnt_root, g_current_dir);

}
static ssize_t mymounts_open(struct file *filp, char __user *buf, size_t count)
{
	return 0;
}

static ssize_t mymounts_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	if (*f_pos == 0)
		update_mounts();
	return simple_read_from_buffer(buf, count, f_pos, g_buffer, strlen(g_buffer));
}

static ssize_t mymounts_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	return -EINVAL;
}


static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = mymounts_open,
	.read = seq_read,
	.write = seq_write
};

static __init int init_debug(void)
{

	g_file = proc_create("mymount", 0444, NULL, &fops);
	if (!g_file)
		return -ENODEV;
	return 0;
}

static __exit void exit_debug(void)
{
	proc_remove(g_file);
}

module_init(init_debug);
module_exit(exit_debug);
