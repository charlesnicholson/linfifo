#include "linfifo_os.h"

#include <mach/mach.h>
#include <mach/vm_map.h>

size_t linfifo_os_mem_page_size(void) { return vm_page_size; }

typedef enum {
  LINFIFO_OS_MBUF_RETVAL_SUCCESS = LINFIFO_RETVAL_SUCCESS,
  LINFIFO_OS_MBUF_RETVAL_NO_MEM = LINFIFO_RETVAL_ERR_NO_MEM,
  LINFIFO_OS_MBUF_RETVAL_OS = LINFIFO_RETVAL_ERR_OS,
  LINFIFO_OS_MBUF_RETVAL_LOST_RACE,
} linfifo_os_mbuf_retval_t;

static linfifo_os_mbuf_retval_t try_alloc_mbuf(size_t len, void **out_mbuf) {
  // grab the full range
  vm_address_t first_half;
  if (vm_allocate(mach_task_self(),
                  &first_half,
                  len * 2,
                  VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
    return LINFIFO_OS_MBUF_RETVAL_NO_MEM;
  }

  // deallocate the second half
  if (vm_deallocate(mach_task_self(), first_half + len, len) != KERN_SUCCESS) {
    vm_deallocate(mach_task_self(), first_half, len * 2);
    return LINFIFO_OS_MBUF_RETVAL_OS;
  }

  // remap the first half onto the second half
  vm_prot_t cur_prot, max_prot;
  vm_address_t second_half = first_half + len;
  kern_return_t const remap_res = vm_remap(mach_task_self(),
                                           &second_half,
                                           len,
                                           0, // mask
                                           VM_FLAGS_FIXED,
                                           mach_task_self(),
                                           first_half,
                                           FALSE, // copy
                                           &cur_prot,
                                           &max_prot,
                                           VM_INHERIT_COPY);

  if (remap_res == KERN_NO_SPACE) {
    vm_deallocate(mach_task_self(), first_half, len);
    return LINFIFO_OS_MBUF_RETVAL_LOST_RACE;
  }

  *out_mbuf = (void *)first_half;
  return LINFIFO_OS_MBUF_RETVAL_SUCCESS;
}


linfifo_retval_t linfifo_os_mbuf_create(size_t len, void **out_mbuf) {
  if (!len) { return LINFIFO_RETVAL_ERR_ARG; }
  if (len != trunc_page(len)) { return LINFIFO_RETVAL_ERR_ARG; }

  linfifo_os_mbuf_retval_t ok;
  while((ok = try_alloc_mbuf(len, out_mbuf)) == LINFIFO_OS_MBUF_RETVAL_LOST_RACE) {}
  return (linfifo_retval_t)ok;
}

linfifo_retval_t linfifo_os_mbuf_free(void *p, size_t len) {
  if (vm_deallocate(mach_task_self(), (vm_address_t)p, len * 2) != KERN_SUCCESS) {
    return LINFIFO_RETVAL_ERR_OS;
  }
  return LINFIFO_RETVAL_SUCCESS;
}

