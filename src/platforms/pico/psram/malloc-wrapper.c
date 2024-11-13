/*
    Malloc/etc. interrupt locking wrappers

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef GPIO_PIN_PSRAM_CS

#include <stdio.h>
#include <malloc.h>
#include <hardware/sync.h>
#include <pico/platform.h>

#include "psram.h"

extern void *__real_malloc(size_t size);

extern void *__real_calloc(size_t count, size_t size);

extern void *__real_realloc(void *mem, size_t size);

extern void __real_free(void *mem);

extern struct mallinfo __real_mallinfo();

#ifdef GPIO_PIN_PSRAM_CS
extern uint8_t __psram_start__;
extern uint8_t __psram_heap_start__;

void __malloc_lock(struct _reent *ptr);

void __malloc_unlock(struct _reent *ptr);

static void *__ram_start = (void *) 0x20000000; // TODO - Is there a SDK exposed variable/macro?
#endif

#ifndef maxIRQs
#define maxIRQs 15
#endif
static uint32_t _irqStackTop[2] = {0, 0};
static uint32_t _irqStack[2][maxIRQs];

static void interrupts() {
    const uint core = get_core_num();
    if (!_irqStackTop[core]) {
        // ERROR
        return;
    }
    restore_interrupts(_irqStack[core][--_irqStackTop[core]]);
}

static void noInterrupts() {
    const uint core = get_core_num();
    if (_irqStackTop[core] == maxIRQs) {
        // ERROR
        panic("IRQ stack overflow");
    }
    _irqStack[core][_irqStackTop[core]++] = save_and_disable_interrupts();
}

// Utilize the existing malloc lock infrastructure and interrupt blocking
// to work with multicore and FreeRTOS
void *pmalloc(size_t size) {
    noInterrupts();
    void *rc = __psram_malloc(size);
    interrupts();
    return rc;
}

void *pcalloc(size_t count, size_t size) {
    noInterrupts();
    void *rc = __psram_calloc(count, size);
    interrupts();
    return rc;
}

extern char __StackLimit;
extern char __bss_end__;

static inline int total_heap() {
    return &__StackLimit - &__bss_end__;
}

static inline int used_heap() {
    struct mallinfo m = mallinfo();
    return m.uordblks;
}

static inline int free_heap() {
    return total_heap() - used_heap();
}

void *__wrap_malloc(size_t size) {
    void *rc;
    noInterrupts();
    //printf("__wrap_malloc(%zu) -> free_heap: %zu\r\n", size, free_heap());
    if (free_heap() < size + 1024 * 10) rc = __psram_malloc(size);
    else rc = __real_malloc(size);
    interrupts();
    return rc;
}

void *__wrap_calloc(size_t count, size_t size) {
    void *rc;
    noInterrupts();
    //printf("__wrap_calloc(%zu) -> free_heap: %zu\r\n", size, free_heap());
    if (free_heap() < size + 1024 * 10) rc = __psram_calloc(count, size);
    else rc = __real_calloc(count, size);
    interrupts();
    return rc;
}

void *__wrap_realloc(void *mem, size_t size) {
    void *rc;
    noInterrupts();
    if (mem && (mem < __ram_start)) {
        rc = __psram_realloc(mem, size);
    } else {
        rc = __real_realloc(mem, size);
    }
    interrupts();
    return rc;
}

void __wrap_free(void *mem) {
    noInterrupts();
    if (mem && (mem < __ram_start)) {
        __psram_free(mem);
    } else {
        __real_free(mem);
    }
    interrupts();
}

struct mallinfo __wrap_mallinfo() {
    noInterrupts();
    const struct mallinfo ret = __real_mallinfo();
    interrupts();
    return ret;
}
#endif
