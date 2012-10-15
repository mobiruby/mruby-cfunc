# makefile discription.
# basic build file for cfunc library

# project-specific macros
# extension of the executable-file is modifiable(.exe .out ...)
BASEDIR = .
TESTSRCS := $(wildcard $(BASEDIR)/t/*.rb)
TESTOBJS := $(patsubst %.rb,%.o,$(TESTSRCS))
TESTS := $(patsubst %.rb,%.exe,$(TESTSRCS))

LIBR := $(BASEDIR)/../lib/libmruby-cfunc.a
EXCEPT1 := $(MRBCSRC)
MRBC := $(BASEDIR)/../vendors/bin/mrbc

# libffi
LIBFFI_CFLAGS = $(shell pkg-config libffi --cflags)
LIBFFI_LIBS = $(shell pkg-config libffi --libs)

# libraries, includes
LIBS = -ldl -lm

ifeq ($(shell uname -s),Darwin)
  LDFLAGS = -Wl,-allow_stack_execute
else
  LDFLAGS = 
endif

ifeq ($(strip $(COMPILE_MODE)),)
  # default compile option
  COMPILE_MODE = debug
endif

ifeq ($(COMPILE_MODE),debug)
  CFLAGS = -g -O3 -rdynamic
else ifeq ($(COMPILE_MODE),release)
  CFLAGS = -O3 -rdynamic
else ifeq ($(COMPILE_MODE),small)
  CFLAGS = -Os -rdynamic
endif

ALL_CFLAGS = -Wall -Werror-implicit-function-declaration -std=c99 $(CFLAGS) $(MRUBY_CFLAGS) $(LIBFFI_CFLAGS)



##############################
# internal variables

export CP := cp
export RM_F := rm -f
export CAT := cat


##############################
# generic build targets, rules

.PHONY : all
all : $(TESTS)

.PHONY : run
run : $(TESTS)
	@echo
	@echo "*** Start tests ***"
	@echo
	@for command in $(TESTS) ; do \
	    $$command ; \
	    echo ; \
	done

# executable constructed using linker from object files
$(TESTS) : %.exe : %.o
	$(LL) -o $@ $(CFLAGS) $(LDFLAGS) $(MRUBY_LIBS) $(LIBFFI_LIBS) $(LIBS) $(LIBR) $<

-include $(OBJS:.o=.d) $(OBJMRB:.o=.d)

# mrby complie
$(TESTOBJS) : %.o : %.rb
	cat $< | ruby -e "print STDIN.read.gsub(/#\s*BEGIN\s+C.*/m,'')" | $(MRBC) -s -Btest_irep > $(basename $<).c
	echo "const char* appname = \"$(basename $<)\";" >> $(basename $<).c
	cat main.c >> $(basename $<).c
	cat $< | ruby -e 's=STDIN.read;print s.gsub(/.*#\s*BEGIN\s+C(.*)/m,"\\1") if s.match(/#\s*BEGIN\s+C/)' >> $(basename $<).c
	$(CC) $(ALL_CFLAGS) -MMD $(INCLUDES) -c $(basename $<).c -o $@
	rm $(basename $<).c


# clean up
.PHONY : clean #cleandep
clean :
	@echo "make: removing targets, objects and depend files of `pwd`"
	-$(RM_F) $(TESTS) $(TESTOBJS) $(TESTS)
	-$(RM_F) $(TESTOBJS:.o=.d) $(TESTOBJS:.o=.c)
