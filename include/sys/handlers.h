/*! \file handlers.h
    \brief Interface: Handler and Kernel lock
    \author Jochen Hoenicke

    Handlers are short functions running with highest priority just below
    interrupts.  They are enqueued to the handler_queue by the interrupts or
    user events (such as semaphores) when the handler needs to run.  As soon
    as the interrupt returns and the user code releases the kernel locks that
    it may hold, the handlers are invoked in sequential order.
 */

#ifndef __sys_handlers_h__
#define __sys_handlers_h__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * The handler queue data structure.
 * @internal
 */
typedef struct _handler_t {
    /** pointer to the next handler on the handler_queue linked list.
     */
    struct _handler_t *next;
    /** Pointer to the function to invoke.  
     */
    void (*code)(void);
} handler_t;

/**
 * The linked list of active handlers.
 */
extern handler_t * volatile handler_queue;
extern handler_t * volatile * volatile handler_tail;

extern inline unsigned char cli() {
    unsigned char oldflags;
    __asm__ ("\tstc   ccr, %0l   ; save flags\n"
	     "\torc   #0x80, ccr ; disable IRQ\n" : "=r" (oldflags));
    return oldflags;
}
extern inline void sti(unsigned char oldflags) {
    __asm__ ("	ldc   %0l, ccr ; restore flags\n" : : "r" (oldflags));
}

/**
 * Enqueue a handler.  May only be called from interrupts
 */
#define enqueue_handler_irq(handler) \
    "mov.w #_" # handler ", r6\n" \
    "mov.w @r6, r0\n" \
    "bne   7f\n" \
    "mov.w @_handler_tail, r0\n" \
    "mov.w r6, @r0\n" \
    "mov.w r6, @_handler_tail\n" \
    "mov.w #_kernel_lock, r6\n" \
    "bclr  #7, @r6\n" /* need to run handlers */ \
    "7:\n"

/**
 * Enqueue a handler.  Should only be called with grabbed kernel_lock.
 */
extern inline void enqueue_handler(handler_t *handler) {
    extern volatile char kernel_lock;
    unsigned char oldflags = cli();
    *handler_tail = handler;
    handler_tail = &handler->next;
    kernel_lock &= ~0x80;
    sti(oldflags);
}

#ifdef  __cplusplus
}
#endif

#endif /* __sys_handlers_h__ */
