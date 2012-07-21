# makefile discription.
# basic build file for mruby

# compiler, linker (gcc), archiver, parser generator
export CC = gcc
export LL = gcc
export AR = ar

ifeq ($(strip $(COMPILE_MODE)),)
  # default compile option
  COMPILE_MODE = debug
endif

ifeq ($(COMPILE_MODE),debug)
  CFLAGS = -g -O3
else ifeq ($(COMPILE_MODE),release)
  CFLAGS = -O3
else ifeq ($(COMPILE_MODE),small)
  CFLAGS = -Os
endif

LIBFFI_CFLAGS = $(shell pkg-config ./vendors/lib/pkgconfig/libffi.pc --cflags)

ALL_CFLAGS = -Wall -Werror-implicit-function-declaration -std=c99 $(CFLAGS) $(LIBFFI_CFLAGS)
ifeq ($(OS),Windows_NT)
  MAKE_FLAGS = --no-print-directory CC=$(CC) LL=$(LL) ALL_CFLAGS='$(ALL_CFLAGS)'
else
  MAKE_FLAGS = --no-print-directory CC='$(CC)' LL='$(LL)' ALL_CFLAGS='$(ALL_CFLAGS)'
endif

##############################
# internal variables

export MSG_BEGIN = @for line in
export MSG_END = ; do echo "$$line"; done

export CP := cp
export RM_F := rm -f
export CAT := cat


##############################
# generic build targets, rules

.PHONY : all
all : vendors/lib/libffi.a vendors/lib/libmruby.a
	@$(MAKE) -C src $(MAKE_FLAGS)

# mruby test
.PHONY : test
test : all
	@$(MAKE) -C test $(MAKE_FLAGS) run

# clean up
.PHONY : clean
clean :
	@$(MAKE) clean -C src $(MAKE_FLAGS)
	@$(MAKE) clean -C test $(MAKE_FLAGS)


##################
# libffi.a
tmp/libffi:
	mkdir -p tmp/libffi
	cd tmp && git clone https://github.com/atgreen/libffi.git

vendors/lib/libffi.a: tmp/libffi
	cd tmp/libffi && ./configure --prefix=`pwd`/../../vendors && make install


##################
# libmruby.a
tmp/mruby:
	mkdir -p tmp/mruby
	cd tmp; git clone https://github.com/mruby/mruby.git

vendors/lib/libmruby.a: tmp/mruby
	cd tmp/mruby && make
	cp -r tmp/mruby/include vendors/
	cp -r tmp/mruby/lib vendors/
	cp -r tmp/mruby/bin vendors/
