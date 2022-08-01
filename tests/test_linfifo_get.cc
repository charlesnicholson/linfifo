#include "linfifo_fixture.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"

TEST_CASE_FIXTURE(LinFifoFixture, "linfifo_get_reserve") {
  void *get_pos;
  size_t get_len;

  SUBCASE("bad args") {
    REQUIRE(linfifo_get_acquire(nullptr, &get_pos, &get_len) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_get_acquire(&lf, nullptr, &get_len) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, nullptr) == LINFIFO_RETVAL_ERR_ARG);
  }

  SUBCASE("empty") {
    get_len = 1234;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_ERR_NO_MEM);
    REQUIRE(get_len == 0);
  }

  SUBCASE("one byte") {
    lf.head = 1;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(get_pos == lf.seat);
    REQUIRE(get_len == 1);
  }

  SUBCASE("almost full") {
    lf.head = lf.capacity - 1;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(get_pos == lf.seat);
    REQUIRE(get_len == lf.capacity - 1);
  }

  SUBCASE("full") {
    lf.head = lf.capacity;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(get_pos == lf.seat);
    REQUIRE(get_len == lf.capacity);
  }
}

TEST_CASE_FIXTURE(LinFifoFixture, "linfifo_get_commit") {
}
