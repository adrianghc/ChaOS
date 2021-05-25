
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for thread management.
 */


#include "drivers/util.h"
#include "lib/inttypes.h"


#ifndef THREAD_H_
#define THREAD_H_


#define THREAD_STATUS_INACTIVE          0
#define THREAD_STATUS_READY             1
#define THREAD_STATUS_RUNNING           2
#define THREAD_STATUS_BLOCKED           3
#define THREAD_STATUS_TERMINATED        4

#define THREAD_FLAG_UNPRIVILEGED        1 << 0
#define THREAD_FLAG_PRIVILEGED          1 << 1
#define THREAD_FLAG_DRIVER              1 << 2
#define THREAD_FLAG_TASK                1 << 3

#define THREAD_PRIO_DEFAULT             1000

#define THREAD_REG_FP                   11
#define THREAD_REG_IP                   12
#define THREAD_REG_SP                   13
#define THREAD_REG_LR                   14
#define THREAD_REG_PC                   15
#define THREAD_REG_CPSR                 16

#define THREAD_CPSR_USER_MODE           0b00000000000000000000000000010000
#define THREAD_CPSR_SYSTEM_MODE         0b00000000000000000000000000011111

#define THREAD_MAX_THREADS              32
#define THREAD_ROUND_ROBIN_TIME_SLOT    3

#define THREAD_INITIAL_PAGES            6

#define THREAD_STACK_SIZE_PER_TASK      1*MB

#define THREAD_DESTROY_CODE             -1


/**
 * The struct holding a TCB entry.
 * 
 * @field id                The thread ID
 * @field parent_id         The parent thread's ID
 * @field first_sibling_id  The first child's ID
 * @field next_sibling_id   The next sibling's ID
 * @field num_task_children The number of children task threads
 * @field r                 An array containing the saved values for all 17 registers
 * @field ret               The thread's last return value
 * @field flags             The thread's flags
 * @field status            The thread's status
 * @field prio              The thread's priority
 * @field ttb               The thread's translation table base (physical address)
 */
struct thread_tcb {
    uint32_t id;
    uint32_t parent_id;
    uint32_t first_child_id;
    uint32_t next_sibling_id;
    uint32_t num_task_children;
    uint32_t r[17];
    int32_t  ret;
    uint8_t  flags;
    uint8_t  status;
    uint16_t prio;
    uint32_t* ttb;
};


/* BEGIN Idle thread */

/**
 * The main function to be executed by the idle thread.
 */
void thread_idle_text(void);

/* END Idle thread */


/* BEGIN Thread management functions */

/**
 * Initializes the thread management.
 */
void thread_init_management(void);

/**
 * Returns the TCB of the current thread.
 * 
 * @return                  A pointer to the thread's TCB
 */
struct thread_tcb* thread_get_current(void);

/**
 * Gets the fp of the last running thread.
 * Use only in the IRQ Interupt Service Routine!
 * 
 * @return                  The fp as a pointer
 */
uint32_t* thread_get_fp(void);

/**
 * Saves the context of the last running thread into its TCB.
 * Use only in the IRQ Interrupt Service Routine!
 * 
 * @param tcb               A pointer to the thread's TCB
 */
void thread_save_context(struct thread_tcb* tcb);

/**
 * Returns the context of the last running thread from its TCB.
 * Use only in the IRQ Interrupt Service Routine!
 * 
 * @param tcb               A pointer to the thread's TCB
 */
void thread_restore_context(struct thread_tcb* tcb);

/**
 * Creates a new thread, i.e. allocates a TCB entry and a stack.
 * 
 * @param text              A pointer to the new thread's text segment
 * @param par_id            The parent thread's ID
 * @param is_task           Whether the new thread is a task
 * @param is_idle           Whether we are creating the idle thread
 * 
 * @return                  A pointer to the new thread's TCB
 */
struct thread_tcb* thread_create(void* text, uint32_t par_id, int8_t is_task, uint32_t is_idle);

/**
 * Lets a thread destroy itself.
 * 
 * @param tcb               A pointer to the thread*s TCB
 * @param exit_code         The thread's exit code
 */
void thread_exit(struct thread_tcb* tcb, int32_t exit_code);

/**
 * Activates a thread, i.e. sets its status to ready.
 * 
 * @param id                The ID of the thread to be activated
 */
void thread_activate(uint32_t id);

/**
 * Deactivates a thread, i.e. sets its status to inactive.
 * 
 * @param id                The ID of the thread to be deactivated
 */
void thread_deactivate(uint32_t id);

/* END Thread management functions */


/* BEGIN Scheduling functions */

/**
 * Switches the running thread.
 * 
 * Use only in the IRQ Interrupt Service Routine!
 */
void thread_switch(void);

/**
 * Selects the next thread to run.
 * 
 * Use only in the IRQ Interrupt Service Routine!
 */
void thread_select(void);

/* END Scheduling functions */


/* BEGIN Functions to manage blocking reasons */

/**
 * Marks the current thread as blocked with the reason that it is waiting for input.
 */
void thread_block_for_input(struct thread_tcb*);

/**
 * Marks a thread as unblocked with the reason that it is not waiting for input anymore.
 * 
 * @return                  A pointer to the thread that has been unblocked
 */
struct thread_tcb* thread_unblock_for_input(void);

/**
 * Marks the current thread as blocked with the reason that it is waiting for a char.
 */
void thread_block_for_char(struct thread_tcb*);

/**
 * Marks a thread as unblocked with the reason that it is not waiting for a char anymore.
 * 
 * @return                  A pointer to the thread that has been unblocked
 */
struct thread_tcb* thread_unblock_for_char(void);

/**
 * Marks a thread as unblocked with the reason that its timer has been interrupted.
 * 
 * @param tcb               A pointer to the thread to unblock
 */
void thread_unblock_for_timer_prematurely(struct thread_tcb* tcb);


/**
 * Marks the current thread as blocked with the reason that it is waiting for a timer to finish.
 */
inline void thread_block_for_timer(struct thread_tcb* tcb);

/**
 * Marks all threads as unblocked whose timers have finished.
 */
void thread_unblock_for_timer(void);

/* END Functions to manage blocking reasons */


/* BEGIN Debugging functions */

void thread_print_info(struct thread_tcb* tcb);
void thread_print_status(uint8_t status);

/* END Debugging functions */


#endif /* THREAD_H_ */
