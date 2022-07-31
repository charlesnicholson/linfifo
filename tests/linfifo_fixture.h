#pragma once

#include "linfifo/linfifo.h"

class LinFifoFixture {
public:
  LinFifoFixture();
  virtual ~LinFifoFixture();
protected:
  linfifo_t lf;
};

