#include "linfifo/linfifo.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
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

  unsigned char *buf = static_cast<unsigned char *>(lf.seat);
  memset(buf, 0, lf.capacity * 2);

  SUBCASE("mirror buf: first byte first half mirrors to first byte second half") {
    buf[0] = 0xA5;
    REQUIRE(buf[lf.capacity] == 0xA5);
  }

  SUBCASE("mirror buf: first byte second half mirrors to first byte first half") {
    buf[lf.capacity] = 0x5A;
    REQUIRE(buf[0] == 0x5A);
  }

  SUBCASE("mirror buf: last byte first half mirrors to last byte second half") {
    buf[lf.capacity - 1] = 0xA5;
    REQUIRE(buf[(lf.capacity * 2) - 1] == 0xA5);
  }

  SUBCASE("mirror buf: last byte second half mirrors to last byte first half") {
    buf[(lf.capacity * 2) - 1] = 0xE3;
    REQUIRE(buf[lf.capacity - 1] == 0xE3);
  }

  SUBCASE("mirror buf: full range first half mirrors to full range second half") {
    std::iota(buf, buf + lf.capacity, 0);
    REQUIRE(!memcmp(buf, buf + lf.capacity, lf.capacity));
  }

  SUBCASE("mirror buf: full range second half mirrors to full range first half") {
    std::iota(buf + lf.capacity, buf + (lf.capacity * 2) - 1, 0);
    REQUIRE(!memcmp(buf, buf + lf.capacity, lf.capacity));
  }

  REQUIRE(linfifo_destroy(&lf) == LINFIFO_RETVAL_SUCCESS);
}
