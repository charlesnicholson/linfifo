#include "../linfifo.h"
#include "doctest.h"

#include <cstdlib>
#include <numeric>

TEST_CASE("linfifo_create") {
  linfifo_t lf;
  size_t const ps = linfifo_mem_page_size();

  SUBCASE("bad args") {
    REQUIRE(linfifo_create(ps, nullptr) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_create(0, &lf) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_create(ps - 1, &lf) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_create(ps + 1, &lf) == LINFIFO_RETVAL_ERR_ARG);
  }

  REQUIRE(linfifo_create(ps, &lf) == LINFIFO_RETVAL_SUCCESS);
  REQUIRE(lf.capacity == ps);
  REQUIRE(lf.head == 0);
  REQUIRE(lf.tail == 0);
  REQUIRE(lf.seat);

  SUBCASE("mirror buf") {
    char *buf = static_cast<char *>(lf.seat);
    memset(&buf[lf.capacity], 0, lf.capacity);
    std::iota(buf, buf + lf.capacity, 0);
    REQUIRE(!memcmp(buf, buf + lf.capacity, lf.capacity));
  }

  REQUIRE(linfifo_destroy(&lf) == LINFIFO_RETVAL_SUCCESS);
}
