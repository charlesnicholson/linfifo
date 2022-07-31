#include "linfifo/linfifo.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"

struct F {
  F() { REQUIRE(linfifo_create(linfifo_mem_page_size(), &lf) == LINFIFO_RETVAL_SUCCESS); }
  virtual ~F();
  linfifo_t lf;
};

F::~F() { REQUIRE(linfifo_destroy(&lf) == LINFIFO_RETVAL_SUCCESS); } // weak-vtable warning

TEST_CASE_FIXTURE(F, "linfifo_put_acquire") {
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

TEST_CASE_FIXTURE(F, "linfifo_put_commit") {
  SUBCASE("bad args") {
    REQUIRE(linfifo_put_commit(nullptr, 0) == LINFIFO_RETVAL_ERR_ARG);
  }

  SUBCASE("zero to one") {
    REQUIRE(linfifo_put_commit(&lf, 1) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.head == 1);
  }

  SUBCASE("increments head by len") {
    REQUIRE(linfifo_put_commit(&lf, 123) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.head == 123);
  }

  SUBCASE("increments from non-zero head") {
    lf.head = 123;
    REQUIRE(linfifo_put_commit(&lf, 100) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.head == 223);
  }

  SUBCASE("fills buffer completely") {
    REQUIRE(linfifo_put_commit(&lf, lf.capacity) == LINFIFO_RETVAL_SUCCESS);
    REQUIRE(lf.head == lf.capacity);
  }

  SUBCASE("fails if committing more than capacity") {
    REQUIRE(linfifo_put_commit(&lf, lf.capacity + 1) == LINFIFO_RETVAL_ERR_NO_MEM);
  }

  SUBCASE("fails if committing exhausts space") {
    lf.head = lf.capacity - 2;
    REQUIRE(linfifo_put_commit(&lf, 3) == LINFIFO_RETVAL_ERR_NO_MEM);
  }

  SUBCASE("exhausting space respects tail index") {
    lf.tail = 5;
    lf.head = 4;
    REQUIRE(linfifo_put_commit(&lf, 2) == LINFIFO_RETVAL_ERR_NO_MEM);
  }
}

