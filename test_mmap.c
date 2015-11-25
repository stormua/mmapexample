#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stddef.h>
#include <sys/mman.h>
 
#include "device_ioctl.h"

#define PAGE_SIZE     4096
 
int main ( int argc, char **argv )
{
    int configfd;
    char * address = NULL;
    memory_area test_dma;
    int err,i;
 

    memset((void *)&test_dma,0,sizeof(memory_area)); 
    
    configfd = open("/sys/kernel/debug/mmap_example", O_RDWR);
    if(configfd < 0)
      {
        perror("Open call failed");
        return -1;
    }
     
       test_dma.area_order=2;
   
   printf("Ask for area %d pages\n",2<<test_dma.area_order);
   
   err=ioctl(configfd,DEVICE_IOC_GETDMA, &test_dma);
   if(err!=0){
     printf("Get error %d\n",err);
   }

   printf("Get area %d bytes \n",(int)test_dma.area_size);
   printf("Get from kernel size %d bytes log addr 0x%lx  phys addr == %lx\n", (int)test_dma.area_size, test_dma.area,  test_dma.ph_area);


    address = mmap(NULL,test_dma.area_size, PROT_READ|PROT_WRITE, MAP_SHARED, configfd, 0);
    if (address == MAP_FAILED)
    {
        perror("mmap operation failed");
        return -1;
    }
 
    err=ioctl(configfd,DEVICE_TEST_DATA, &test_dma);
     if(err!=0){
       printf("Get error %d\n",err);
     }

   err=ioctl(configfd,DEVICE_IOC_MAPAREA, &test_dma);
     if(err!=0){
       printf("Get error %d\n",err);
     }

     for (i=0;i<10;i++){
       printf("content: 0x%x %c\n",*(address+i),(char)(*(address+i))  );
     }

     memcpy(address, "ByeByeBye\0", 10);
     for (i=0;i<10;i++){
       printf("content: 0x%x %c\n",*(address+i),(char)(*(address+i))  );
     }


     err=ioctl(configfd,DEVICE_IOC_MAPAREA, &test_dma);
     if(err!=0){
       printf("Get error %d\n",err);
     }



    printf("Initial message: %s\n", address);
    memcpy(address + 11 , "*user*", 6);
    printf("Changed message: %s\n", address);
    err=ioctl(configfd,DEVICE_IOC_CLEARDMA, &test_dma);
    close(configfd);    
    return 0;
}
