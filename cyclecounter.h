#ifndef CYCLECOUNTER_H
#define CYCLECOUNTER_H
#define DRVR_NAME "cyclecounter"

#include <linux/ioctl.h>
/* 
 * This header file is used by both kernel code and user code. The
 * portion of the header used by kernel code is concealed from user
 * code by the __KERNEL__ ifdef.
 */
#ifdef __KERNEL
/*
 * Required Proc File-system Struct
 *
 * Used to map entry into proc file table upon module insertion
 */
extern struct proc_dir_entry *cyclecounter_proc_entry;

#endif /* __KERNEL__*/

/* 
 * ===============================================
 *             Public API Functions
 * ===============================================
 */
/*
 * There typically needs to be a struct definition for each flavor of
 * IOCTL call.
 */
typedef struct cyclecounter_ioctl_flushall_s {
	int placeholder;
} cyclecounter_ioctl_flushall_t;

/* 
 * This generic union allows us to make a more generic IOCTRL call
 * interface. Each per-IOCTL-flavor struct should be a member of this
 * union.
 */
typedef union cyclecounter_ioctl_param_u {
	cyclecounter_ioctl_flushall_t      set;
} cyclecounter_ioctl_param_union;


/* 
 * Used by _IOW to create the unique IOCTL call numbers. It appears
 * that this is supposed to be a single character from the examples I
 * have looked at so far. 
 */
#define CYCLECOUNTER_MAGIC 't'

/*
 * For each flavor of IOCTL call you will need to make a macro that
 * calls the _IOW() macro. This macro is just a macro that creates a
 * unique ID for each type of IOCTL call. It uses a combination of bit
 * shifting and OR-ing of each of these arguments to create the
 * (hopefully) unique constants used for IOCTL command values.
 */
#define CYCLECOUNTER_IOCTL_FLUSHALL _IOW(CYCLECOUNTER_MAGIC, 1, cyclecounter_ioctl_flushall_t)


#endif /* CYCLECOUNTER_H */
