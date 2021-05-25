
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for thread management.
 */


#include "sys/thread.h"
#include "drivers/cp15.h"
#include "drivers/util.h"
#include "lib/buffer.h"
#include "lib/inttypes.h"
#include "lib/math.h"
#include "lib/mem.h"
#include "sys/memmgmt.h"
#include "sys/sysio.h"


struct thread_tcb thread_tcb_list[THREAD_MAX_THREADS];
uint8_t thread_switch_counter;
uint32_t thread_sched_cur_idx;

// TODO Implement these with dynamic memory
struct ring_buffer threads_blocked_for_input;
uint32_t threads_blocked_for_input_raw[THREAD_MAX_THREADS];

struct ring_buffer threads_blocked_for_char;
uint32_t threads_blocked_for_char_raw[THREAD_MAX_THREADS];

int32_t threads_blocked_for_timer[THREAD_MAX_THREADS];


/* BEGIN Idle thread */

/**
 * The main function to be executed by the idle thread.
 */
__attribute__((section(".lib")))
void thread_idle_text(void) {
    // The idle thread makes a repeated SWI_THREAD_YIELD call to
    // ensure it only runs when there is no other thread to yield to.
    while(1) {
        asm volatile(
            "swi 0x20"
            ::
        );
    }
}

/* END Idle thread */


/* BEGIN Thread management functions */

/**
 * Initializes the thread management.
 */
void thread_init_management(void) {

    uint8_t i;
    struct thread_tcb* idle_tcb;

    for (i = 0; i < THREAD_MAX_THREADS; i++) {
        thread_tcb_list[i].id = 0;
        threads_blocked_for_input_raw[i] = 0;
        threads_blocked_for_char_raw[i] = 0;
        threads_blocked_for_timer[i] = -1;
    }

    thread_switch_counter = 0;

    ring_init(
        &threads_blocked_for_input,
        (int8_t*) &threads_blocked_for_input_raw,
        THREAD_MAX_THREADS * sizeof(uint32_t)
    );

    ring_init(
        &threads_blocked_for_char,
        (int8_t*) &threads_blocked_for_char_raw,
        THREAD_MAX_THREADS * sizeof(uint32_t)
    );

    idle_tcb = thread_create(&thread_idle_text, 0, 0, 1);
    thread_activate(idle_tcb->id);

}

/**
 * Returns the TCB of the current thread.
 * 
 * @return          A pointer to the thread's TCB
 */
struct thread_tcb* thread_get_current(void) {
    return &thread_tcb_list[thread_sched_cur_idx];
}

/**
 * Gets the fp of the last running thread.
 * Use only in the IRQ Interupt Service Routine!
 * 
 * @return          The fp as a pointer
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
 * @param tcb       A pointer to the thread's TCB
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
 * @param tcb       A pointer to the thread's TCB
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
struct thread_tcb* thread_create(void* text, uint32_t par_id, int8_t is_task, uint32_t is_idle) {
    uint32_t i;

    // Search for the next free TCB
    for (i = 0; i < THREAD_MAX_THREADS; i++) {
        if (!thread_tcb_list[i].id) {
            break;
        }
    }

    // Return 0 if there are no free TCBs
    if (i == THREAD_MAX_THREADS) {
        return 0;
    }

    // Return 0 if we would nest task threads
    if (is_task && thread_tcb_list[par_id-1].flags & THREAD_FLAG_TASK) {
        return 0;
    }

    // Return 0 if the kernel wants to create a task
    if (is_task && !par_id) {
        return 0;
    }

    struct thread_tcb* tcb = &thread_tcb_list[i];
    memzero((uint8_t*) tcb, sizeof(struct thread_tcb));

    if (is_idle) {
        tcb->id = 1;
    } else {
        tcb->id = i+1;
    }
    tcb->r[THREAD_REG_PC] = (uint32_t)text;
    if (is_task) {
        thread_tcb_list[par_id-1].num_task_children++;
        tcb->r[THREAD_REG_SP] = 0xF0000000 - (thread_tcb_list[par_id-1].num_task_children * THREAD_STACK_SIZE_PER_TASK);
    } else {
        tcb->r[THREAD_REG_SP] = 0xF0000000;
    }
    tcb->num_task_children = 0;
    tcb->r[THREAD_REG_CPSR] = THREAD_CPSR_USER_MODE;

    tcb->flags  = THREAD_FLAG_UNPRIVILEGED;
    if (is_task) {
        tcb->flags |= THREAD_FLAG_TASK;
    }

    tcb->prio       = THREAD_PRIO_DEFAULT;
    tcb->status     = THREAD_STATUS_INACTIVE;
    tcb->parent_id  = par_id;

    if (!is_idle) {

        struct thread_tcb* prev_sibling;
        struct thread_tcb* parent;

        if (!par_id) {
            parent = &thread_tcb_list[0];
        } else {
            parent = &thread_tcb_list[par_id-1];
        }

        if (parent->first_child_id) {
            prev_sibling = &thread_tcb_list[parent->first_child_id - 1];
            while (prev_sibling->next_sibling_id) {
                prev_sibling = &thread_tcb_list[prev_sibling->next_sibling_id - 1];
            }
            prev_sibling->next_sibling_id = tcb->id;
        } else {
            parent->first_child_id = tcb->id;
        }

    }

    tcb->next_sibling_id = 0;
    tcb->first_child_id = 0;

    if (is_task) {
        tcb->ttb = (uint32_t*) thread_tcb_list[par_id-1].ttb;
        memmgmt_map_any(tcb->ttb, tcb->r[THREAD_REG_SP] - 1*MB, 1, 1);
    } else {
        tcb->ttb = memmgmt_setup_thread(tcb->id);

        // Setting up the mapping for the OS
        for (i = 0; i < 512; i++) {
            memmgmt_map_page(tcb->ttb, i, i * MB, 0, 0);
        }

        // Map the kernel non-readable to itself
        memmgmt_map_to(tcb->ttb, 0x20000000, 0x20000000, 0, 0);
        // Map the user library and the application read-only to itself
        memmgmt_map_to(tcb->ttb, 0x20100000, 0x20100000, 1, 0);
        // Map the stack for the thread
        memmgmt_map_any(tcb->ttb, tcb->r[THREAD_REG_SP] - 1*MB, 1, 1);

        // Setting up the mapping for the OS
        for (i = MEMMGMT_TTB_ENTRIES - 256; i < MEMMGMT_TTB_ENTRIES; i++) {
            memmgmt_map_page(tcb->ttb, i, i * MB, 0, 0);
        }
    }

    return tcb;
}

/**
 * Lets a thread destroy itself.
 * 
 * @param tcb       A pointer to the thread's TCB
 * @param exit_code The thread's exit code
 */
void thread_exit(struct thread_tcb* tcb, int32_t exit_code) {

    thread_unblock_for_timer_prematurely(tcb);
    // TODO Delete from other blocking lists

    tcb->status = THREAD_STATUS_TERMINATED;
    tcb->ret = exit_code;

    if (!tcb->parent_id || !exit_code) {
        tcb->id = 0;
    }

    // Exit all children
    if (tcb->first_child_id) {
        struct thread_tcb* child = &thread_tcb_list[tcb->first_child_id -1];
        uint32_t id_next_sibling = child->next_sibling_id;
        thread_exit(child, 0);
        while (id_next_sibling) {
            child = &thread_tcb_list[id_next_sibling-1];
            id_next_sibling = child->next_sibling_id;
            thread_exit(child, 0);
        }
    }

    // Clean the memory from the thread
    if (!(tcb->flags & THREAD_FLAG_TASK)) {
        memmgmt_cleanup_thread(tcb->ttb);
    }

    // TODO Return exit code to father

}

/**
 * Activates a thread, i.e. sets its status to ready.
 * 
 * @param id        The ID of the thread to be activated
 */
void thread_activate(uint32_t id) {
    thread_tcb_list[id-1].status = THREAD_STATUS_READY;
}

/**
 * Deactivates a thread, i.e. sets its status to inactive.
 * 
 * @param id        The ID of the thread to be deactivated
 */
void thread_deactivate(uint32_t id) {
    thread_tcb_list[id-1].status = THREAD_STATUS_INACTIVE;
}

/* END Thread management functions */


/* BEGIN Scheduling functions */

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

/**
 * Selects the next thread to run.
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

/* END Scheduling functions */


/* BEGIN Functions to manage blocking reasons */

/**
 * Marks the current thread as blocked with the reason that it is waiting for input.
 */
inline void thread_block_for_input(struct thread_tcb* tcb) {

    // Set the current thread as blocked
    tcb->status = THREAD_STATUS_BLOCKED;

    // Write the thread to the blocked ring
    ring_write(&threads_blocked_for_input, (int8_t*)&thread_sched_cur_idx, sizeof(uint32_t));

}

/**
 * Marks the current thread as blocked with the reason that it is waiting for a char.
 */
inline void thread_block_for_char(struct thread_tcb* tcb) {

    // Set the current thread as blocked
    tcb->status = THREAD_STATUS_BLOCKED;

    // Write the thread to the blocked ring
    ring_write(&threads_blocked_for_char, (int8_t*)&thread_sched_cur_idx, sizeof(uint32_t));

}

/**
 * Marks a thread as unblocked with the reason that it is not waiting for input anymore.
 * 
 * @return          A pointer to the thread that has been unblocked
 */
struct thread_tcb* thread_unblock_for_input(void) {

    struct thread_tcb* tcb;
    uint32_t thread_to_activate;

    if (ring_read(&threads_blocked_for_input, (int8_t*) &thread_to_activate, sizeof(uint32_t)) != sizeof(uint32_t)) {
        return 0;
    }

    tcb = &thread_tcb_list[thread_to_activate];
    tcb->status = THREAD_STATUS_READY;

    return tcb;

}

/**
 * Marks a thread as unblocked with the reason that it is not waiting for a char anymore.
 * 
 * @return          A pointer to the thread that has been unblocked
 */
struct thread_tcb* thread_unblock_for_char(void) {

    struct thread_tcb* tcb;
    uint32_t thread_to_activate;

    if (ring_read(&threads_blocked_for_char, (int8_t*) &thread_to_activate, sizeof(uint32_t)) != sizeof(uint32_t)) {
        return 0;
    }

    tcb = &thread_tcb_list[thread_to_activate];
    tcb->status = THREAD_STATUS_READY;

    return tcb;

}


/**
 * Marks the current thread as blocked with the reason that it is waiting for a timer to finish.
 */
inline void thread_block_for_timer(struct thread_tcb* tcb) {

    // Set the current thread as blocked
    tcb->status = THREAD_STATUS_BLOCKED;

    // Write the thread to the blocked array
    threads_blocked_for_timer[tcb->id - 1] = (uint32_t) tcb->r[7];

}

/**
 * Marks all threads as unblocked whose timers have finished.
 */
void thread_unblock_for_timer(void) {

    uint32_t i;

    for (i = 0; i < THREAD_MAX_THREADS; i++) {
        if (threads_blocked_for_timer[i] == -1) {
            continue;
        }
        if (!threads_blocked_for_timer[i]--) {
            thread_tcb_list[i].status = THREAD_STATUS_READY;
            thread_tcb_list[i].r[7] = 0;
        }
    }

}

/**
 * Marks a thread as unblocked with the reason that its timer has been interrupted.
 * 
 * @param tcb       A pointer to the thread to unblock
 */
void thread_unblock_for_timer_prematurely(struct thread_tcb* tcb) {

    if (threads_blocked_for_timer[tcb->id - 1] == -1) {
        return;
    }
    tcb->status = THREAD_STATUS_READY;
    tcb->r[7] = threads_blocked_for_timer[tcb->id - 1];
    threads_blocked_for_timer[tcb->id - 1] = -1;

}

/* END Functions to manage blocking reasons */


/* BEGIN Debugging functions */

void thread_print_info(struct thread_tcb* tcb) {

    printf_isr("================================\n");

    printf_isr("Info for thread #%x\n", tcb->id);
    printf_isr("Parent ID: %x\n", tcb->parent_id);
    printf_isr("Flags:     %x\n", tcb->flags);
    printf_isr("Status:    "); thread_print_status(tcb->status); printf_isr("\n");
    printf_isr("Priority:  %x\n", tcb->prio);
    printf_isr("TTB:       %x\n", (uint32_t) tcb->ttb);
    if (tcb->flags & THREAD_FLAG_TASK) {
        printf_isr("Parent TTB:       %x\n", (uint32_t) thread_tcb_list[tcb->parent_id -1].ttb);
    }
    printf_isr("Registers:\n");
    printf_isr("  r0:   %x\n", tcb->r[0]);
    printf_isr("  r1:   %x\n", tcb->r[1]);
    printf_isr("  r2:   %x\n", tcb->r[2]);
    printf_isr("  r3:   %x\n", tcb->r[3]);
    printf_isr("  r4:   %x\n", tcb->r[4]);
    printf_isr("  r5:   %x\n", tcb->r[5]);
    printf_isr("  r6:   %x\n", tcb->r[6]);
    printf_isr("  r7:   %x\n", tcb->r[7]);
    printf_isr("  r8:   %x\n", tcb->r[8]);
    printf_isr("  r9:   %x\n", tcb->r[9]);
    printf_isr("  r10:  %x\n", tcb->r[10]);
    printf_isr("  r11:  %x\n", tcb->r[11]);
    printf_isr("  r12:  %x\n", tcb->r[12]);
    printf_isr("  sp:   %x\n", tcb->r[13]);
    printf_isr("  lr:   %x\n", tcb->r[14]);
    printf_isr("  pc:   %x\n", tcb->r[15]);
    printf_isr("  cpsr: %x\n", tcb->r[16]);

    printf_isr("================================\n");

}

void thread_print_status(uint8_t status) {
    switch (status) {
    case THREAD_STATUS_INACTIVE:
        printf_isr("THREAD_STATUS_INACTIVE");
        break;
    case THREAD_STATUS_READY:
        printf_isr("THREAD_STATUS_READY");
        break;
    case THREAD_STATUS_RUNNING:
        printf_isr("THREAD_STATUS_RUNNING");
        break;
    case THREAD_STATUS_BLOCKED:
        printf_isr("THREAD_STATUS_BLOCKED");
        break;
    case THREAD_STATUS_TERMINATED:
        printf_isr("THREAD_STATUS_TERMINATED");
        break;
    default:
        printf_isr("THREAD_STATUS_INVALID");
    }
}

/* END Debugging functions */
