#include "linfifo.h"
#include "linfifo_os.h"

size_t linfifo_mem_page_size(void) { return linfifo_os_mem_page_size(); }

linfifo_retval_t linfifo_create(size_t len, linfifo_t *out_lf) {
  size_t const ps = linfifo_mem_page_size();

  if (!out_lf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (!len) { return LINFIFO_RETVAL_ERR_ARG; }
  if (((len / ps) * ps) != len) { return LINFIFO_RETVAL_ERR_ARG; }
  if (len & (len - 1)) { return LINFIFO_RETVAL_ERR_ARG; }

  out_lf->head = out_lf->tail = 0;
  out_lf->capacity = len;
  return linfifo_os_mbuf_create(len, &out_lf->seat);
}

linfifo_retval_t linfifo_destroy(linfifo_t *lf) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  linfifo_os_mbuf_free(lf->seat, lf->capacity);
  lf->seat = NULL;
  lf->head = lf->tail = lf->capacity = 0;
  return LINFIFO_RETVAL_SUCCESS;
}

linfifo_retval_t linfifo_put_acquire(linfifo_t *lf, void **out_buf, size_t *out_avail) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (!out_buf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (!out_avail) { return LINFIFO_RETVAL_ERR_ARG; }
  char *seat = (char *)lf->seat;
  *out_buf = &seat[lf->head & (lf->capacity - 1)];
  *out_avail = lf->capacity - (lf->head - lf->tail);
  return LINFIFO_RETVAL_SUCCESS;
}

linfifo_retval_t linfifo_put_advance(linfifo_t *lf, size_t len) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  lf->head += len;
  return LINFIFO_RETVAL_SUCCESS;
}

linfifo_retval_t linfifo_get_acquire(linfifo_t *lf, void **out_buf, size_t *out_avail) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (!out_buf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (!out_avail) { return LINFIFO_RETVAL_ERR_ARG; }
  char *seat = (char *)lf->seat;
  *out_buf = &seat[lf->tail & (lf->capacity - 1)];
  *out_avail = lf->head - lf->tail;
  return LINFIFO_RETVAL_SUCCESS;
}

linfifo_retval_t linfifo_get_advance(linfifo_t *lf, size_t len) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  lf->tail += len;
  return LINFIFO_RETVAL_SUCCESS;
}
