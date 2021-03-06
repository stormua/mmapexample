#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>  
#include <linux/ioctl.h>
#include <asm/uaccess.h>


 
#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif
 

typedef struct {
  //  int is_placed;
  unsigned long area_order;  //in order
  unsigned long area_size;
  unsigned long area;
  unsigned long ph_area;

} memory_area;



/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define DEVICE_IOC_MAGIC  0x82
/* Please use a different 8-bit number in your code */

#define DEVICE_IOCRESET    _IO(DEVICE_IOC_MAGIC, 0)


#define DEVICE_IOC_GETDMA    _IOWR(DEVICE_IOC_MAGIC,  1, unsigned long int)
#define DEVICE_IOC_CLEARDMA  _IOR(DEVICE_IOC_MAGIC,   2, unsigned long int)
#define DEVICE_IOC_MAPAREA   _IOWR(DEVICE_IOC_MAGIC,  3, unsigned long int)
#define DEVICE_IOC_UNMAPAREA _IOR(DEVICE_IOC_MAGIC,   4, unsigned long int)
#define DEVICE_TEST_DATA     _IO(DEVICE_IOC_MAGIC,   5)

#define DEVICE_IOC_MAXNR 14

unsigned long kv_addr=0;
dma_addr_t dma_handle;
size_t dma_order=0;
size_t dma_size=0;



struct dentry  *file;
 
struct mmap_info
{
  char *data;
  unsigned long size;
  unsigned int order;
  int reference;      
};
 
void mmap_open(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference++;
}
 
void mmap_close(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference--;
}
 
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    struct page *page;
    struct mmap_info *info;    
     
    info = (struct mmap_info *)vma->vm_private_data;
    if (!info->data)
    {
        printk("No data\n");
        return 0;    
    }
     
    page = virt_to_page(info->data);    
     
    get_page(page);
    vmf->page = page;            
     
    return 0;
}
 
struct vm_operations_struct mmap_vm_ops =
{
    .open =     mmap_open,
    .close =    mmap_close,
    .fault =    mmap_fault,    
};
 
int op_mmap(struct file *filp, struct vm_area_struct *vma)
{
    vma->vm_ops = &mmap_vm_ops;
    vma->vm_flags |= VM_RESERVED;    
    vma->vm_private_data = filp->private_data;
    mmap_open(vma);
    return 0;
}



int mmapfop_close(struct inode *inode, struct file *filp)
{
    struct mmap_info *info = filp->private_data;

    if(info->size){
      free_pages((unsigned long)info->data, info->order);
    }
    kfree((unsigned long)info);
    filp->private_data = NULL;
    return 0;
}
 
int mmapfop_open(struct inode *inode, struct file *filp)
{
    struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);    
    //    info->data = (char *)get_zeroed_page(GFP_KERNEL);
    //memcpy(info->data, "hello from kernel this is file: ", 32);
    //memcpy(info->data + 32, filp->f_dentry->d_name.name, strlen(filp->f_dentry->d_name.name));
    /* assign this info struct to the file */
    filp->private_data = info;
    return 0;
}


long device_ioctl(struct file* filp,unsigned int cmd, unsigned long arg)
{
  int err = 0;//, tmp;
  //int retval = 0;
  //unsigned long int *addr;
  
  memory_area mem_arg;
  struct mmap_info *info;
  char str[80];
#ifdef DEBUG
  printk(KERN_INFO "Incoming to device ioctl...\n");
#endif

  /* extract the type and number bitfields, and don't decode */
  /*   wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok() */
  if (_IOC_TYPE(cmd) != DEVICE_IOC_MAGIC)
    return -ENOTTY;
  if (_IOC_NR(cmd) > DEVICE_IOC_MAXNR)
    return -ENOTTY;

  /*
   * the direction is a bitmask, and VERIFY_WRITE catches R/W
   * transfers. `Type' is user-oriented, while
   * access_ok is kernel-oriented, so the concept of "read" and
   * "write" is reversed
   */
  /* if (_IOC_DIR(cmd) & _IOC_READ) { */
  /*   err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd)); */
  /*   //printk(KERN_INFO "Warinig  ioctl can't write data to user space...\n"); */
  /* } else if (_IOC_DIR(cmd) & _IOC_WRITE) { */
  /*   err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)); */
  /*   printk(KERN_INFO "Warning  ioctl can't read data from user space...\n"); */
  /* } */
  //	if (err)
  //		return -1;


  copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));

  switch(cmd){
  case DEVICE_IOC_GETDMA:
    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    dma_order=mem_arg.area_order;
#ifdef DEBUG
    printk(KERN_INFO "Ask kernel  for  %d pages", 2<<dma_order);
#endif

    kv_addr=__get_free_pages(0,dma_order);
#ifdef DEBUG
    printk(KERN_INFO "Get from kernel at %lx", (unsigned long)kv_addr);
#endif

    if(kv_addr!=0){
      dma_size=(2<<dma_order)*PAGE_SIZE;
      info=filp->private_data;
      //      info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);    
      info->data = (char *)kv_addr;
      info->size=dma_size;
      info->order=dma_order;
      filp->private_data = info;

      //      dma_handle=dma_map_single( DEVICE_VAR ,(void *)kv_addr,dma_size,DMA_BIDIRECTIONAL);
      mem_arg.area=kv_addr;
      mem_arg.area_size=dma_size;
      mem_arg.ph_area=__phys_addr(kv_addr);


#ifdef DEBUG
      printk(KERN_INFO "ioctl asks for %d bytes  kv_addr==0x%lx, saves to 0x%lx", (unsigned int)dma_size, (unsigned long)kv_addr, __phys_addr(kv_addr));
#endif
      copy_to_user((memory_area __user*)arg,&mem_arg,sizeof(memory_area));
    }else{
      printk(KERN_INFO "get NULL from __get_free_pages(0,%d)\n",dma_order);
    }
    break;

  case DEVICE_IOC_CLEARDMA:
    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    if(mem_arg.area!=0){
#ifdef DEBUG
      printk(KERN_INFO "Try to clear %d pages\n",2<<mem_arg.area_order);
#endif
      info=filp->private_data;

      //dma_unmap_single(  ,dma_handle,mem_arg.area_size,DMA_BIDIRECTIONAL);
      free_pages(info->data, info->order);
      info->size=0;
      //      kfree(info);
      //      filp->private_data=NULL;
    }
    break;
  case  DEVICE_IOC_MAPAREA:
    //    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    info=filp->private_data;
    strncpy(str,info->data,79);
    str[79]=0;
    printk(KERN_INFO "area==_%s_\n",str);

    break;
  case  DEVICE_IOC_UNMAPAREA:
    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));

    break;
  case  DEVICE_TEST_DATA:
    info=filp->private_data;
    memcpy(info->data, "hello from kernel this is file: ", 32);
    memcpy(info->data + 32, filp->f_dentry->d_name.name, strlen(filp->f_dentry->d_name.name));
    break;
  default:
    return -1;
  }

  return 0;
}

 
static const struct file_operations mmap_fops = {
    .open = mmapfop_open,
    .release = mmapfop_close,
    .mmap = op_mmap,
    .unlocked_ioctl=device_ioctl,
};
 
static int __init mmapexample_module_init(void)
{
    file = debugfs_create_file("mmap_example", 0664, NULL, NULL, &mmap_fops);
    return 0;
}
 
static void __exit mmapexample_module_exit(void)
{
    debugfs_remove(file);
}
 
module_init(mmapexample_module_init);
module_exit(mmapexample_module_exit);
MODULE_LICENSE("GPL");

