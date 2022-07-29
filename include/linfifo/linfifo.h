#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct linfifo {
  size_t head, tail, capacity;
  void *seat; // mirrored
  void *os_ctx;
} linfifo_t;

typedef enum {
  LINFIFO_RETVAL_SUCCESS = 0,
  LINFIFO_RETVAL_ERR_ARG,
  LINFIFO_RETVAL_ERR_NO_MEM,
  LINFIFO_RETVAL_ERR_OS
} linfifo_retval_t;

size_t linfifo_mem_page_size(void);

linfifo_retval_t linfifo_create(size_t len, linfifo_t *out_lf);
linfifo_retval_t linfifo_destroy(linfifo_t *lf);
linfifo_retval_t linfifo_put_acq(linfifo_t *lf, void **out_buf, size_t *out_avail);
linfifo_retval_t linfifo_put_rel(linfifo_t *lf, size_t len);
linfifo_retval_t linfifo_get_acq(linfifo_t *lf, void **out_buf, size_t *out_avail);
linfifo_retval_t linfifo_get_rel(linfifo_t *lf, size_t len);

#ifdef __cplusplus
}
#endif

