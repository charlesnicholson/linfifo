#include "linfifo_fixture.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"

TEST_CASE_FIXTURE(LinFifoFixture, "linfifo_get_acquire") {
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

  SUBCASE("some bytes") {
    lf.head = 1234;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(get_pos == lf.seat);
    REQUIRE(get_len == 1234);
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

  SUBCASE("length is distance to head") {
    lf.head = 321;
    lf.tail = 100;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(get_pos == static_cast<char *>(lf.seat) + 100);
    REQUIRE(get_len == 321 - 100);
  }

  SUBCASE("position is wrapped around into buffer") {
    lf.tail = (lf.capacity * 9) + 100; // arbitrary
    lf.head = lf.tail + 432;
    REQUIRE(linfifo_get_acquire(&lf, &get_pos, &get_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(get_pos == static_cast<char *>(lf.seat) + 100);
    REQUIRE(get_len == 432);
  }
}

TEST_CASE_FIXTURE(LinFifoFixture, "linfifo_get_commit") {
  SUBCASE("bad args") {
    REQUIRE(linfifo_get_commit(nullptr, 8) == LINFIFO_RETVAL_ERR_ARG);
  }

  SUBCASE("zero to one") {
    lf.head = 1;
    REQUIRE(linfifo_get_commit(&lf, 1) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.tail == 1);
  }

  SUBCASE("increments tail by len") {
    lf.head = 1000;
    REQUIRE(linfifo_get_commit(&lf, 123) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.tail == 123);
  }

  SUBCASE("increments from nonzero tail") {
    lf.head = 1000;
    lf.tail = 201;
    REQUIRE(linfifo_get_commit(&lf, 123) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.tail == 201 + 123);
  }

  SUBCASE("drains buffer completely") {
    lf.head = lf.capacity;
    REQUIRE(linfifo_get_commit(&lf, lf.capacity) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.tail == lf.capacity);
  }

  SUBCASE("fails if getting more than has been put") {
    lf.head = 123;
    lf.tail = 122;
    REQUIRE(linfifo_get_commit(&lf, 2) == LINFIFO_RETVAL_ERR_NO_MEM);
  }
}
