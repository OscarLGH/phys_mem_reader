#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

static int phys_access_proc_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t phys_access_proc_write(struct file *filp, const char __user *buffer,
			size_t count, loff_t *ppos)
{
	long phys_offset = *ppos;
	void *virt = __va(phys_offset);
	printk("*ppos = 0x%llx virt = 0x%llx, count = %d\n", *ppos, virt, count);
	if (virt == NULL)
		return -EFAULT;

	//if (count > 8)
	//	return -EFAULT;
	copy_from_user(virt, buffer, count);
	*ppos += count;
	return count;
}

static ssize_t phys_access_proc_read(struct file *filp, char __user *buffer,
			size_t count, loff_t *ppos)
{
	long phys_offset = *ppos;
	void *virt = __va(phys_offset);

	printk("*ppos = 0x%llx virt = 0x%llx, count = %d\n", *ppos, virt, count);
	if (virt == NULL)
		return -EFAULT;

	//if (count > 8)
	//	return -EFAULT;
	copy_to_user(buffer, virt, count);
	*ppos += count;
	return count;
}


static const struct file_operations phys_access_fops = {
	.open = phys_access_proc_open,
	.read = phys_access_proc_read,
	.write = phys_access_proc_write,
};


static int __init phys_access_init(void)
{
	proc_create("phys_mem", 0755, NULL, &phys_access_fops);
	return 0;
}

static void __exit phys_access_cleanup(void)
{
	remove_proc_entry("phys_mem", NULL);
}

module_init(phys_access_init);
module_exit(phys_access_cleanup);
MODULE_LICENSE("GPL");
