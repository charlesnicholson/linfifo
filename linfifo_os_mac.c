#include "linfifo_os.h"

#include <mach/mach.h>
#include <mach/vm_map.h>

_Static_assert(sizeof(void *) >= sizeof(mem_entry_name_port_t), "");

size_t linfifo_os_mem_page_size(void) { return vm_page_size; }

linfifo_retval_t linfifo_os_mbuf_create(linfifo_t *lf) {
  if (!lf) { return LINFIFO_RETVAL_ERR_ARG; }
  if (lf->capacity != trunc_page(lf->capacity)) { return LINFIFO_RETVAL_ERR_ARG; }

  vm_address_t addr;
  if (vm_allocate(mach_task_self(),
                  &addr,
                  lf->capacity * 2,
                  VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
    return LINFIFO_RETVAL_ERR_NO_MEM;
  }

  if (vm_allocate(mach_task_self(),
                  &addr,
                  lf->capacity,
                  VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE) != KERN_SUCCESS) {
    vm_deallocate(mach_task_self(), addr, lf->capacity * 2);
    return LINFIFO_RETVAL_ERR_OS;
  }

  mem_entry_name_port_t mapping_port, parent_name = 0;
  if (mach_make_memory_entry(mach_task_self(),
                             &lf->capacity,
                             addr,
                             VM_PROT_READ | VM_PROT_WRITE,
                             &mapping_port,
                             parent_name) != KERN_SUCCESS) {
    vm_deallocate(mach_task_self(), addr, lf->capacity * 2);
    return LINFIFO_RETVAL_ERR_OS;
  }

  lf->seat = (void *)addr;
  lf->os_ctx = (void *)(uintptr_t)mapping_port;

  vm_address_t mirror_addr = addr + lf->capacity;
  if (vm_map(mach_task_self(),
             &mirror_addr,
             lf->capacity,
             0,
             VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE,
             mapping_port,
             0,
             VM_INHERIT_SHARE,
             VM_PROT_READ | VM_PROT_WRITE,
             VM_PROT_READ | VM_PROT_WRITE,
             VM_INHERIT_NONE) != KERN_SUCCESS) {
    vm_deallocate(mapping_port, addr, lf->capacity * 2);
    return LINFIFO_RETVAL_ERR_OS;
  }

  return LINFIFO_RETVAL_SUCCESS;
}

linfifo_retval_t linfifo_os_mbuf_free(linfifo_t *lf) {
  if (!lf || !lf->seat) { return LINFIFO_RETVAL_ERR_ARG; }
  vm_address_t const addr = (vm_address_t)lf->seat;
  return (vm_deallocate(mach_task_self(), addr, lf->capacity * 2) == KERN_SUCCESS) ?
    LINFIFO_RETVAL_SUCCESS : LINFIFO_RETVAL_ERR_OS;
}

