
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for thread management.
 */


#include "drivers/cp15.h"
#include "drivers/util.h"
#include "lib/inttypes.h"
#include "lib/math.h"


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

extern struct thread_tcb thread_tcb_list[THREAD_MAX_THREADS];
extern uint8_t thread_switch_counter;
extern uint32_t thread_sched_cur_idx;


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
 * Use only in the IRQ Interrupt Service Routine!
 * 
 * @return                  The fp as a pointer
 */
__attribute__((always_inline))
inline uint32_t* thread_get_fp(void) {
    uint32_t* fp;
    asm volatile (
        "mov %[fp], fp \n\t"
        : [fp] "=r" (fp)
    );
    return fp;
}

/**
 * Saves the context of the last running thread into its TCB.
 * Use only in the IRQ Interrupt Service Routine!
 * 
 * @param tcb               A pointer to the thread's TCB
 */
__attribute__((always_inline))
inline void thread_save_context(struct thread_tcb* tcb) {

    uint32_t* ptr;
    uint8_t i;
    uint32_t* b;
    uint32_t s;

    ptr = thread_get_fp() - 6;
    for (i = 0; i < 4; i++) { // r0-r3
        tcb->r[i] = ptr[i];
    }

    tcb->r[11] = ptr[4]; // fp r11
    tcb->r[12] = ptr[5]; // ip r12
    tcb->r[15] = ptr[6]; // pc r15

    b = &tcb->r[4];
    asm volatile ( // r4-r10
        "stm %[rs], {r4-r10} \n\t"
        : [rs] "=r" (b)
    );

    b = &tcb->r[13];
    asm volatile ( // r13-r14
        "stm %[rs], {r13-r14}^ \n\t"
        : [rs] "=r" (b)
    );

    s = tcb->r[THREAD_REG_CPSR];
    asm volatile ( // cpsr
        "mrs %[rs], SPSR \n\t"
        : [rs] "=r" (s)
    );

}

/**
 * Returns the context of the last running thread from its TCB.
 * Use only in the IRQ Interrupt Service Routine!
 * 
 * @param tcb               A pointer to the thread's TCB
 */
__attribute__((always_inline))
inline void thread_restore_context(struct thread_tcb* tcb) {

    uint32_t* ptr;
    uint8_t i;
    uint32_t* b;
    uint32_t s;

    // set the translation table base so the thread only sees it's own memory space
    cp15_write_translation_table_base(tcb->ttb);
    cp15_mmu_enable();

    ptr = thread_get_fp() - 6;
    for (i = 0; i < 4; i++) { // r0-r3
        ptr[i] = tcb->r[i];
    }

    ptr[4] = tcb->r[11]; // fp r11
    ptr[5] = tcb->r[12]; // ip r12
    ptr[6] = tcb->r[15]; // pc r15

    b = &tcb->r[4];
    asm volatile ( // r4-r10
        "ldm %[rs], {r4-r10} \n\t"
        : [rs] "=r" (b)
    );

    b = &tcb->r[13];
    asm volatile ( // r13-r14
        "ldm %[rs], {r13-r14}^ \n\t"
        : [rs] "=r" (b)
    );

    s = tcb->r[THREAD_REG_CPSR];

    // invalidate caches and TLB
    cp15_invalidate_caches();
    cp15_invalidate_tlb();

    asm volatile ( // cpsr
        "msr SPSR, %[rs] \n\t"
        : [rs] "=r" (s)
    );

}

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
 * Selects the next thread to run.
 * 
 * Use only in the IRQ Interrupt Service Routine!
 */
__attribute__((always_inline))
inline void thread_select(void) {

    uint8_t i;
    uint8_t j;
    struct thread_tcb* tcb;

    thread_switch_counter = 0;

    // Find out which is the next thread
    for (i = 1; i <= THREAD_MAX_THREADS; i++) {
        j = math_mod(thread_sched_cur_idx + i, THREAD_MAX_THREADS);
        if (!j) {
            continue;
        }
        tcb = &thread_tcb_list[j];

        if (tcb->id && tcb->status == THREAD_STATUS_READY) {
            // Set the next thread context
            thread_sched_cur_idx = tcb->id - 1;
            return;
        }
    }
    thread_sched_cur_idx = 0;

}

/**
 * Switches the running thread.
 * 
 * Use only in the IRQ Interrupt Service Routine!
 */
__attribute__((always_inline))
inline void thread_switch(void) {

    // Check that there is a thread currently running
    if (thread_tcb_list[thread_sched_cur_idx].status == THREAD_STATUS_RUNNING) {
        // Do not switch if the thread has not worked through its time slot yet
        if (thread_switch_counter++ < THREAD_ROUND_ROBIN_TIME_SLOT) {
            return;
        }
        thread_switch_counter = 0;

        // Save the current thread's context and set its status
        thread_save_context(&thread_tcb_list[thread_sched_cur_idx]);
        thread_tcb_list[thread_sched_cur_idx].status = THREAD_STATUS_READY;
    }

    thread_select();

    thread_restore_context(&thread_tcb_list[thread_sched_cur_idx]);
    thread_tcb_list[thread_sched_cur_idx].status = THREAD_STATUS_RUNNING;

}

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
void thread_block_for_timer(struct thread_tcb* tcb);

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
