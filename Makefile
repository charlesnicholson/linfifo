BUILD_DIR := build
OS := $(shell uname)
COMPILER_VERSION := $(shell $(CXX) --version)

SRCS := linfifo.c \
		tests/unittest_main.cc \
		tests/linfifo_fixture.cc \
		tests/test_linfifo_create.cc \
		tests/test_linfifo_get.cc \
		tests/test_linfifo_put.cc

ifeq ($(OS),Darwin)
SRCS += linfifo_os_mac.c
endif

BUILD_DIR := build
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS = -std=c17
CXXFLAGS = -std=c++20 -Wno-c++98-compat
CPPFLAGS += -MMD -MP -Os -g -Wall -Werror -Wextra -Wno-padded -Iinclude

ifneq '' '$(findstring clang,$(COMPILER_VERSION))'
CPPFLAGS += -Weverything -Wno-poison-system-directories -Wno-format-pedantic
CXXFLAGS += -Wno-c++98-compat-pedantic -Wno-c++98-compat-bind-to-temporary-copy
endif


$(BUILD_DIR)/linfifo_unittests: $(OBJS) Makefile
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c Makefile
	mkdir -p $(dir $@) && $(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cc.o: %.cc Makefile
	mkdir -p $(dir $@) && $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/linfifo_unittests.timestamp: $(BUILD_DIR)/linfifo_unittests
	$(BUILD_DIR)/linfifo_unittests -m && touch $(BUILD_DIR)/linfifo_unittests.timestamp

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

.DEFAULT_GOAL := $(BUILD_DIR)/linfifo_unittests.timestamp

-include $(DEPS)

