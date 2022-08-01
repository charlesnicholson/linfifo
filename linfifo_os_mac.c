#include "linfifo_os.h"

#include <mach/mach.h>
#include <mach/vm_map.h>

_Static_assert(KERN_SUCCESS == 0, "");
_Static_assert(sizeof(void *) == sizeof(vm_address_t), "");

size_t linfifo_os_mem_page_size(void) { return vm_page_size; }

linfifo_retval_t linfifo_os_mbuf_create(linfifo_t *lf) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (lf->capacity != trunc_page(lf->capacity)) { return LINFIFO_RETVAL_ERR_ARG; }

  // Cache verbose constants for less typing.
  mach_port_t const self = mach_task_self();
  int const ow = VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE;
  int const rw = VM_PROT_READ | VM_PROT_WRITE;
  size_t cap = lf->capacity;

  // Allocate space for both buffer and mirror.
  vm_address_t addr;
  if (vm_allocate(self, &addr, cap * 2, VM_FLAGS_ANYWHERE)) {
    return LINFIFO_RETVAL_ERR_NO_MEM;
  }

  lf->seat = (void *)addr;

  // Overwrite allocation to just buffer size, without freeing the mirror space.
  if (vm_allocate(self, &addr, cap, ow) || (addr != (vm_address_t)lf->seat)) {
    vm_deallocate(self, addr, cap * 2);
    return LINFIFO_RETVAL_ERR_OS;
  }

  // Create a memory port for just the buffer size.
  mem_entry_name_port_t p;
  if (mach_make_memory_entry(self, &cap, addr, rw, &p, 0) || (cap != lf->capacity)) {
    vm_deallocate(self, addr, cap * 2);
    return LINFIFO_RETVAL_ERR_OS;
  }

  // Overwrite-remap the mirror space to the new original-buffer memory port.
  vm_address_t mirror = addr + cap;
  if (vm_map(self, &mirror, cap, 0, ow, p, 0, VM_INHERIT_SHARE, rw, rw, VM_INHERIT_COPY)) {
    vm_deallocate(p, addr, cap * 2);
    return LINFIFO_RETVAL_ERR_OS;
  }

  return LINFIFO_RETVAL_SUCCESS;
}

linfifo_retval_t linfifo_os_mbuf_free(linfifo_t *lf) {
  if (!lf || !lf->seat) { return LINFIFO_RETVAL_ERR_ARG; }
  return vm_deallocate(mach_task_self(), (vm_address_t)lf->seat, lf->capacity * 2) ?
    LINFIFO_RETVAL_ERR_OS : LINFIFO_RETVAL_SUCCESS;
}

