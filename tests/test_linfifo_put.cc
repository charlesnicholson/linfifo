#include "linfifo/linfifo.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"

struct ScopedLinFifo {
  ScopedLinFifo(linfifo_t &lf, size_t len) : lf_(lf) {
    REQUIRE(linfifo_create(len, &lf) == LINFIFO_RETVAL_SUCCESS);
  }
  ~ScopedLinFifo() { REQUIRE(linfifo_destroy(&lf_) == LINFIFO_RETVAL_SUCCESS); }
  linfifo_t& lf_;
};

TEST_CASE("linfifo_put_acquire") {
  linfifo_t lf;
  ScopedLinFifo const _(lf, linfifo_mem_page_size());

  void *put_pos;
  size_t put_len;

  SUBCASE("bad args") {
    REQUIRE(linfifo_put_acquire(nullptr, &put_pos, &put_len) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_put_acquire(&lf, nullptr, &put_len) == LINFIFO_RETVAL_ERR_ARG);
    REQUIRE(linfifo_put_acquire(&lf, &put_pos, nullptr) == LINFIFO_RETVAL_ERR_ARG);
  }

  SUBCASE("pristine") {
    REQUIRE(lf.head == 0);
    REQUIRE(linfifo_put_acquire(&lf, &put_pos, &put_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(put_pos == lf.seat);
    REQUIRE(put_len == lf.capacity);
  }

  SUBCASE("one byte present") {
    lf.head = 1;
    REQUIRE(linfifo_put_acquire(&lf, &put_pos, &put_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(put_pos == static_cast<char *>(lf.seat) + 1);
    REQUIRE(put_len == lf.capacity - 1);
  }

  SUBCASE("one byte left") {
    lf.head = lf.capacity - 1;
    REQUIRE(linfifo_put_acquire(&lf, &put_pos, &put_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(put_pos == static_cast<char *>(lf.seat) + lf.capacity - 1);
    REQUIRE(put_len == 1);
  }

  SUBCASE("already full") {
    lf.head = lf.capacity;
    put_len = 12345;
    REQUIRE(linfifo_put_acquire(&lf, &put_pos, &put_len) == LINFIFO_RETVAL_ERR_NO_MEM);
    REQUIRE(put_len == 0);
  }

  SUBCASE("advancing tail shrinks available len") {
    lf.head = lf.capacity - 100;
    lf.tail = 250;
    REQUIRE(linfifo_put_acquire(&lf, &put_pos, &put_len) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(put_len == 350);
  }
}

TEST_CASE("linfifo_put_commit") {
}

