# makefile discription.
# basic build file for mruby

# compiler, linker (gcc), archiver, parser generator
export CC = gcc
export LL = gcc

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

BASEDIR = $(shell pwd)
INCLUDES = -I$(BASEDIR)/include

MRUBY_CFLAGS = -I$(BASEDIR)/vendors/include
MRUBY_LIBS = -L$(BASEDIR)/vendors/lib -lmruby

LIBFFI_CFLAGS = $(shell pkg-config $(BASEDIR)/vendors/lib/pkgconfig/libffi.pc --cflags)
LIBFFI_LIBS = $(BASEDIR)/vendors/lib/libffi.a

ALL_CFLAGS = $(CFLAGS) $(INCLUDES) $(MRUBY_CFLAGS)
ifeq ($(OS),Windows_NT)
  MAKE_FLAGS = --no-print-directory CC=$(CC) LL=$(LL) CFLAGS='$(ALL_CFLAGS)' LIBFFI_CFLAGS='$(LIBFFI_CFLAGS)' LIBFFI_LIBS='$(LIBFFI_LIBS)' MRUBY_CFLAGS='$(MRUBY_CFLAGS)' MRUBY_LIBS='$(MRUBY_LIBS)'
else
  MAKE_FLAGS = --no-print-directory CC='$(CC)' LL='$(LL)' CFLAGS='$(ALL_CFLAGS)' LIBFFI_CFLAGS='$(LIBFFI_CFLAGS)' LIBFFI_LIBS='$(LIBFFI_LIBS)' MRUBY_CFLAGS='$(MRUBY_CFLAGS)' MRUBY_LIBS='$(MRUBY_LIBS)'
endif

##############################
# internal variables

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
	mkdir -p include/ffi
	cp -r `pkg-config vendors/lib/pkgconfig/libffi.pc --cflags-only-I |sed -e "s/ /\/*/" | sed -e "s/-I/ /"` include/ffi


##################
# libmruby.a
tmp/mruby:
	mkdir -p tmp/mruby
	cd tmp; git clone https://github.com/mruby/mruby.git
	sed -i -e s/typedef\ int\ mrb_int/typedef\ long\ mrb_int/g tmp/mruby/include/mrbconf.h
	sed -i -e s/define\ MRB_INT_MIN\ INT_MIN/define\ MRB_INT_MIN\ LONG_MIN/g tmp/mruby/include/mrbconf.h
	sed -i -e s/define\ MRB_INT_MAX\ INT_MAX/define\ MRB_INT_MAX\ LONG_MAX/g tmp/mruby/include/mrbconf.h

vendors/lib/libmruby.a: tmp/mruby
	cd tmp/mruby && make
	cp -r tmp/mruby/include vendors/
	cp -r tmp/mruby/lib vendors/
	cp -r tmp/mruby/bin vendors/
