#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/smp.h>
#include <asm/smp.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h> /* kmalloc() */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include "cyclecounter.h"
#include <linux/version.h>
#include <asm/outercache.h>

#define PERF_DEF_OPTS (1 | 16)
#define PERF_OPT_RESET_CYCLES (2 | 4)
#define procfs_name "cyclecounter"
#define FLUSHALL 3

#define ARMV7_CNT0		0	/* First event counter */
#define ARMV7_CCNT      31  /* Cycle counter */
#define ENABLE_CNTR (1 << ARMV7_CCNT)
#define ARMV7_PMNC_E        (1 << 0) /* Enable all counters */
#define ARMV7_CNTENS_C (1 << ARMV7_CCNT)



#if !defined(__arm__)
#error Module can only be compiled on ARM machines.
#endif

enum armv7_counters {
    ARMV7_CYCLE_COUNTER     = 1,    /* Cycle counter */
    ARMV7_COUNTER0          = 2,    /* First event counter */
};
/* Perf Event to low level counters mapping */
#define ARMV7_EVENT_CNT_TO_CNTx	(ARMV7_COUNTER0 - ARMV7_CNT0)
#define	ARMV7_SELECT_MASK	0x1f		/* Mask for writable bits */

#define ARMV7_CNTENC_P(idx) (1 << (idx - ARMV7_EVENT_CNT_TO_CNTx))
#define ARMV7_CNTENC_C      (1 << ARMV7_CCNT)

#define ARMV7_CNTENS_P(idx) (1 << (idx - ARMV7_EVENT_CNT_TO_CNTx))
#define ARMV7_CNTENS_C      (1 << ARMV7_CCNT)


/*
 *  * EVTSEL: Event selection reg
 *   */
#define ARMV7_EVTSEL_MASK   0xff        /* Mask for writable bits */

#define ARMV7_INTENS_P(idx)    (1 << (idx - ARMV7_EVENT_CNT_TO_CNTx))
#define ARMV7_INTENS_C      (1 << ARMV7_CCNT)



extern long simple_strtol(const char *,char **,unsigned int);
void memory_exit(void);
int memory_init(void);
int memory_open(struct inode *inode, struct file *filp); 
int memory_release(struct inode *inode, struct file *filp); 
ssize_t memory_read(struct file *filp, char *buf, 
                    size_t count, loff_t *f_pos);
ssize_t memory_write( struct file *filp, char *buf,
                      size_t count, loff_t *f_pos);


/* Structure that declares the usual file */
/* access functions */
struct file_operations memory_fops = {
  read: memory_read,
  write: memory_write,
  open: memory_open,
  release: memory_release
};

/* Global variables of the driver */
/* Major number */
int memory_major = 69;
/* Buffer to store data */
char *memory_buffer;

int memory_open(struct inode *inode, struct file *filp) {

  /* Success */
    printk(KERN_INFO "[" DRVR_NAME "] Opening device !\n");
  return 0;
}

int memory_release(struct inode *inode, struct file *filp) {
 
  /* Success */
  return 0;
}

ssize_t memory_read(struct file *filp, char *buf, 
                    size_t count, loff_t *f_pos) { 
 
  /* Transfering data to user space */ 
  copy_to_user(buf,memory_buffer,1);

  /* Changing reading position as best suits */ 
  if (*f_pos == 0) { 
    *f_pos+=1; 
    return 1; 
  } else { 
    return 0; 
  }
}

ssize_t memory_write( struct file *filp, char *buf,
                      size_t count, loff_t *f_pos) {

  char *tmp;
  char *endptr;
  char ret;

  tmp=buf+count-1;
  copy_from_user(memory_buffer,tmp,1);
  ret = simple_strtol(memory_buffer, &endptr, 10);
  if(endptr == NULL)
  {
      printk(KERN_INFO "[" DRVR_NAME "] Failed to read an integer!\n");
  }
  else
  {
      //printk(KERN_INFO "[" DRVR_NAME "] Writing %d !\n", ret);
      if(ret == FLUSHALL)
      {
            printk(KERN_INFO "[" DRVR_NAME "] FLUSH CACHE ALL");
            flush_cache_all();
            /*outer_flush_all();*/

            /*flush_cache_louis();*/
      }
  }
  return 1;
}

static inline void enable_intens(unsigned int idx)
{
    u32 val;
    if(idx == ARMV7_CYCLE_COUNTER)
    {
        val = ARMV7_INTENS_C;
    }else
    {
        val = ARMV7_INTENS_P(idx);
    }
    asm volatile("mcr p15, 0, %0, c9, c14, 1" : : "r" (val));
}

static inline void reset_pmn()
{
    unsigned long value = 0;
    asm volatile("mrc p15, 0, %0, c9, c12, 0 " : "=r" (value)); /* Read PMNC  */
    value |= 0x02;
    asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r" (value));
}
static inline int select_counter(int idx)
{
    u32 cnt = idx & 0x1F;
    asm volatile("mcr p15, 0, %0, c9, c12, 5" : : "r" (cnt));
    return cnt;
}

static inline void write_evtsel(unsigned int idx, u32 evtCode)
{
    select_counter(idx);
    asm volatile("mcr p15, 0, %0, c9, c13, 1" : : "r" (evtCode));
}

static inline u32 read_counter(int idx)
{
    unsigned long value = 0;
    select_counter(idx);
    asm volatile("mrc p15, 0, %0, c9, c13, 2": "=r" (value));
	return value;
}

static inline void write_counter(int idx, u32 value)
{
    select_counter(idx);
    asm volatile("mcr p15, 0, %0, c9, c13, 2" :: "r" (value));
}

static inline void enable_counter(unsigned int idx)
{
    u32 val;

    if (idx == ARMV7_CYCLE_COUNTER)
        val = ARMV7_CNTENS_C;
    else
        val = (1 << idx);

    asm volatile("mcr p15, 0, %0, c9, c12, 1" : : "r" (val));
}

static inline void disable_counter(unsigned int idx)
{
    u32 val;
    if (idx == ARMV7_CYCLE_COUNTER)
        val = ARMV7_CNTENC_C;
    else
        val = ARMV7_CNTENC_P(idx);

    asm volatile("mcr p15, 0, %0, c9, c12, 2" : : "r" (val));

}

static inline void enable_event(int idx, int evt)
{
    /*disable_counter(idx);*/
    if (idx != ARMV7_CYCLE_COUNTER)
        write_evtsel(idx, evt);

    enable_intens(idx);
}


/* Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int cyclecounter_device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "{" DRVR_NAME "] opening cyclecounter device.\n");
    return 0;
}

static void enable_cpu_counters(void* data)
{
    printk(KERN_INFO "[" DRVR_NAME "] enabling user-mode PMU access on CPU #%d", smp_processor_id());
    /* Enable user-mode access to counters. */
    asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1));
    asm volatile("mcr p15, 0, %0, c9, c14, 2" :: "r"(0x80000000));
    /* Enable all counters */
    asm volatile("mcr p15, 0, %0, c9, c12, 0" : : "r"(ARMV7_PMNC_E));
    //enable CCNT (write in CNTENS Register)
    asm volatile("mcr p15, 0, %0, c9, c12, 1" : : "r" (ARMV7_CNTENS_C));

    //DISABLE L2
    outer_disable();
}

static void disable_cpu_counters(void* data)
{
printk(KERN_INFO "[" DRVR_NAME "] disabling user-mode PMU access on CPU #%d",
smp_processor_id());
/* Program PMU and disable all counters */
    /*asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(0));*/
    /*asm volatile("mcr p15, 0, %0, c9, c12, 2" :: "r"(0x8000000f));*/
    asm volatile("mcr p15, 0, %0, c9, c12, 0" : : "r" (0));
    asm volatile("mcr p15, 0, %0, c9, c12, 2" : : "r" (ARMV7_CNTENS_C));
    /* Disable user-mode access to counters. */
    asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(0));
}


int init_module(void)
{
    printk(KERN_INFO "[" DRVR_NAME "] Module initialisation\n");
    on_each_cpu(enable_cpu_counters, NULL, 1);
    int result = register_chrdev(memory_major, "cyclecounter", &memory_fops);
    if (result < 0) {
        printk(
          "[" DRVR_NAME "]: cannot obtain major number %d\n", memory_major);
        return result;
    }

      /* Allocating memory for the buffer */
      memory_buffer = kmalloc(1, GFP_KERNEL); 
      if (!memory_buffer) { 
        result = -ENOMEM;
        goto fail; 
      } 
      memset(memory_buffer, 0, 1);

      printk("[" DRVR_NAME "] Inserting memory module\n"); 

      /*unsigned int counter1= 0;*/
      /*unsigned int event1 = 0x03;//cache_misses event*/
      /*unsigned int counter2= 1;*/
      /*unsigned int event2 = 0x04;*/
      /*printk("[" DRVR_NAME "] Enabling counter %u for the event %u on CPU%d\n\n", counter1, event1, smp_processor_id());*/
      /*reset_pmn();*/
      /*enable_event(counter1, event1);*/
      /*enable_event(counter2, event2);*/

      /*enable_counter(counter1);*/
      /*enable_counter(counter2);*/
      /*write_counter(counter1, 0);*/
      /*write_counter(counter2, 0);*/

      /*u32 cm1, cm2, ch1, ch2;*/
      /*cm1 = read_counter(counter1);  */
      /*ch1 = read_counter(counter2);*/
      /*printk("[" DRVR_NAME "] ===============+> misses %u hits %u CPU%d\n\n", cm1, ch1, smp_processor_id());*/
      /*ch2 = read_counter(counter2);*/
      /*cm2 = read_counter(counter1);*/
      /*printk("[" DRVR_NAME "] ===============+> misses %u hits %u CPU%d\n\n", (cm2 - cm1), (ch2 - ch1), smp_processor_id());*/
      /*int i;*/
      /*for(i = 0; i < 10; i++)*/
      /*{*/
          /*cm1 = read_counter(counter1);  */
          /*ch1 = read_counter(counter2);*/
          /*msleep(2000);*/
          /*cm2 = read_counter(counter1);  */
          /*ch2 = read_counter(counter2);*/
          /*printk("[" DRVR_NAME "] ===============>+ misses %u hits: %u CPU%d\n\n", (cm2 - cm1), (ch2 - ch1), smp_processor_id());*/
      /*}*/
      
      return 0;

      fail: 
        memory_exit(); 
        return result;
        return 0;
}

void memory_exit(void) {
  /* Freeing the major number */
  unregister_chrdev(memory_major, "cyclecounter");

  /* Freeing buffer memory */
  if (memory_buffer) {
    kfree(memory_buffer);
  }

  printk("[" DRVR_NAME "] Removing module\n");

}


void cleanup_module(void)
{
    printk(KERN_INFO "[" DRVR_NAME "] Module unloading\n");
    on_each_cpu(disable_cpu_counters, NULL, 1);
    memory_exit();
}

MODULE_AUTHOR("razaina.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1");
MODULE_DESCRIPTION("Enables user-mode access to ARMv7 PMU counters + additional features");
