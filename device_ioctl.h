#ifndef _DEVICE_IOCTH_H
#define _DEVICE_IOCTH_H

/*******************************************************************/
//#ifndef  
//#error Not ready!!!!!!!!!!!
//#endif 



#define IOCTL_FUNCTION  device_ioctl


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


/* #define DEVICE_IOCSQUANTUM _IOW(DEVICE_IOC_MAGIC,  1, int) */
/* #define DEVICE_IOCSQSET    _IOW(DEVICE_IOC_MAGIC,  2, unsigned long int) */
/* #define DEVICE_IOCTQUANTUM _IO(DEVICE_IOC_MAGIC,   3) */
/* #define DEVICE_IOCTQSET    _IO(DEVICE_IOC_MAGIC,   4) */
/* #define DEVICE_IOCGQUANTUM _IOR(DEVICE_IOC_MAGIC,  5, unsigned long int) */
/* #define DEVICE_IOCGQSET    _IOR(DEVICE_IOC_MAGIC,  6, int) */
/* #define DEVICE_IOCQQUANTUM _IO(DEVICE_IOC_MAGIC,   7) */
/* #define DEVICE_IOCQQSET    _IO(DEVICE_IOC_MAGIC,   8) */
/* #define DEVICE_IOCXQUANTUM _IOWR(DEVICE_IOC_MAGIC, 9, int) */
/* #define DEVICE_IOCXQSET    _IOWR(DEVICE_IOC_MAGIC,10, int) */
/* #define DEVICE_IOCHQUANTUM _IO(DEVICE_IOC_MAGIC,  11) */
/* #define DEVICE_IOCHQSET    _IO(DEVICE_IOC_MAGIC,  12) */



#ifdef _KERNEL_MODULE
long device_ioctl(struct file* filp,unsigned int cmd, unsigned long arg);

int op_mmap(struct file *filp, struct vm_area_struct *vma);
#endif


#endif
