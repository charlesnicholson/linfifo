#pragma once

#include "linfifo.h"

// Internal API for OS-specific detail functions

size_t linfifo_os_mem_page_size(void);
linfifo_retval_t linfifo_os_mbuf_create(size_t len, void **out_mbuf);
linfifo_retval_t linfifo_os_mbuf_free(void *p, size_t len);

