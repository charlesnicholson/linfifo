#pragma once

#include "linfifo/linfifo.h"

size_t linfifo_os_mem_page_size(void);
linfifo_retval_t linfifo_os_mbuf_create(linfifo_t *lf);
linfifo_retval_t linfifo_os_mbuf_free(linfifo_t *lf);

