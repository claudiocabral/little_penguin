// SPDX-License-Identifier: GPL-2.0+

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/dcache.h>
#include <linux/uaccess.h>
#include <linux/mount.h>
#include <linux/fs_struct.h>
#include <linux/list.h>
#include <linux/seq_file.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <../fs/mount.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Claudio Cabral <cl@udio.co>");
MODULE_DESCRIPTION("This module talks to Morty");

static struct proc_dir_entry *g_file;
static char g_buffer[PAGE_SIZE];
static char path[PATH_MAX];

static void *seq_start(struct seq_file *m, loff_t *pos);
static void seq_stop(struct seq_file *m, void *v);
static void *seq_next(struct seq_file *m, void *v, loff_t *pos);
static int seq_show(struct seq_file *m, void *v);

static const struct seq_operations seqops = {
	.start = seq_start,
	.next = seq_next,
	.stop = seq_stop,
	.show = seq_show
};

static void *seq_start(struct seq_file *m, loff_t *pos)
{
	struct mount *mnt;

	mnt = real_mount(current->fs->root.mnt);
	m->private = &mnt->mnt_mounts;
	return seq_list_start(m->private, *pos);
}

static void seq_stop(struct seq_file *m, void *v)
{
}

static void *seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	return seq_list_next(v, m->private, pos);
}

static int seq_show(struct seq_file *m, void *v)
{
	struct mount *mnt;

	mnt = list_entry(v, struct mount, mnt_child);

	seq_printf(m, "%-16s\n", mnt->mnt_devname);
	seq_dentry(m, mnt->mnt_mountpoint, "");
	return 0;
}

static void append_to_buffer(struct mount *entry)
{
	char *real_path;

	real_path = dentry_path_raw(entry->mnt_mountpoint, path, PATH_MAX);
	if (IS_ROOT(entry->mnt_mountpoint))
		strlcat(g_buffer, "root", PAGE_SIZE);
	else
		strlcat(g_buffer, entry->mnt_devname, PAGE_SIZE);
	strlcat(g_buffer, " ", PAGE_SIZE);
	strlcat(g_buffer, real_path, PAGE_SIZE);
	strlcat(g_buffer, "\n", PAGE_SIZE);
}

static void recursive_append(struct mount *mnt)
{
	struct mount *current_entry;

	append_to_buffer(mnt);
	list_for_each_entry(current_entry,
			&mnt->mnt_mounts, mnt_child) {
		append_to_buffer(current_entry);
	}
}

static void update_mounts(void)
{
	*g_buffer = 0;
	recursive_append(real_mount(current->fs->root.mnt));
}

static int mymounts_open(struct inode *node, struct file *filp)
{
	return seq_open(filp, &seqops);
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = mymounts_open,
	// .read = mymounts_read,
	// .write = mymounts_write
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};


static __init int init_debug(void)
{

	g_file = proc_create("mymounts", 0444, NULL, &fops);
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
