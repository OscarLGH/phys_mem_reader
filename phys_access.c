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
	
	if (virt == NULL)
		return -EFAULT;

	copy_from_user(virt, buffer, count);
	*ppos += count;
	return count;
}

static ssize_t phys_access_proc_read(struct file *filp, char __user *buffer,
			size_t count, loff_t *ppos)
{
	long phys_offset = *ppos;
	void *virt = __va(phys_offset);

	if (virt == NULL)
		return -EFAULT;

	copy_to_user(buffer, virt, count);
	*ppos += count;
	return count;
}

static int phys_access_mmap(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;

	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;	
}

static loff_t phys_access_lseek(struct file *file, loff_t offset, int orig)
{
	loff_t ret;
	inode_lock(file_inode(file));
	switch (orig) {
	case SEEK_CUR:
		offset += file->f_pos;
		file->f_pos = offset;
		ret = offset;
		break;
	case SEEK_SET:
		file->f_pos = offset;
		ret = file->f_pos;
		break;
	default:
		ret = -EINVAL;
	}
	inode_unlock(file_inode(file));
	return ret;
}

static const struct file_operations phys_access_fops = {
	.open = phys_access_proc_open,
	.read = phys_access_proc_read,
	.write = phys_access_proc_write,
	.mmap = phys_access_mmap,
	.llseek = phys_access_lseek,
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
