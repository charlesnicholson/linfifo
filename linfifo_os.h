#pragma once

#include "linfifo/linfifo.h"

size_t linfifo_os_mem_page_size(void);
linfifo_retval_t linfifo_os_mbuf_create(linfifo_t *lf);
linfifo_retval_t linfifo_os_mbuf_free(linfifo_t *lf);

#define LINFIFO_LIKELY(EXPR) __builtin_expect((EXPR), 1)
#define LINFIFO_UNLIKELY(EXPR) __builtin_expect((EXPR), 0)
