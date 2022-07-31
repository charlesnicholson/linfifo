#include "linfifo_fixture.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"

LinFifoFixture::LinFifoFixture() {
  REQUIRE(linfifo_create(linfifo_mem_page_size(), &lf) == LINFIFO_RETVAL_SUCCESS);
}

LinFifoFixture::~LinFifoFixture() {
 REQUIRE(linfifo_destroy(&lf) == LINFIFO_RETVAL_SUCCESS);
}
