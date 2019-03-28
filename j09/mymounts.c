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
	if (mnt->mnt_parent)
		mnt = mnt->mnt_parent;
	m->private = &mnt->mnt_list;
	return seq_list_start(m->private, *pos);
}

static void seq_stop(struct seq_file *m, void *v)
{
}

static void *seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	return seq_list_next(v, m->private, pos);
}

static void print_recursive(struct seq_file *m, struct mount *mnt)
{
	char buf[4];
	char *dir_path;

	if (!mnt || !mnt->mnt_parent || !mnt_has_parent(mnt->mnt_parent))
		return;
	print_recursive(m, mnt->mnt_parent);
	dir_path = dentry_path_raw(mnt->mnt_mountpoint, buf, sizeof(buf));
	if (dir_path != ERR_PTR(-ENAMETOOLONG) &&
	    strncmp(dir_path, "/", 2) == 0)
		return;
	seq_dentry(m, mnt->mnt_mountpoint, "");
}

static int seq_show(struct seq_file *m, void *v)
{
	struct mount *mnt;

	mnt = list_entry(v, struct mount, mnt_list);
	if (!mnt || !mnt->mnt_mountpoint || !mnt->mnt_parent)
		return 0;
	if (strcmp(mnt->mnt_parent->mnt_devname, "rootfs") == 0) {
		seq_printf(m, "%-16s/\n", mnt->mnt_devname);
	} else if (mnt->mnt_mountpoint->d_flags & DCACHE_MOUNTED) {
		seq_printf(m, "%-16s", mnt->mnt_devname);
		print_recursive(m, mnt);
		seq_putc(m, '\n');
	}
	return 0;
}

static int mymounts_open(struct inode *node, struct file *filp)
{
	return seq_open(filp, &seqops);
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = mymounts_open,
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
