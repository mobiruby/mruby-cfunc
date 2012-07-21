# makefile discription.
# basic build file for cfunc library

# project-specific macros
# extension of the executable-file is modifiable(.exe .out ...)
BASEDIR = .
TARGET := $(BASEDIR)/cfunc_test
MRBCSRC := $(patsubst %.rb,%.c,$(wildcard $(BASEDIR)/mrb/*.rb))
LIBR := $(BASEDIR)/../vendors/lib/libmruby.a \
	$(BASEDIR)/../lib/libmruby-cfunc.a

EXCEPT1 := $(MRBCSRC)

OBJ1 := $(patsubst %.c,%.o,$(filter-out $(EXCEPT1),$(wildcard $(BASEDIR)/*.c)))
OBJMRB := $(patsubst %.c,%.o,$(MRBCSRC))
OBJS := $(OBJ1) $(OBJ2) $(OBJ3)

MRBC := $(BASEDIR)/../vendors/bin/mrbc


# libraries, includes
INCLUDES = -I$(BASEDIR) -I$(BASEDIR)/../include -I$(BASEDIR)/../vendors/include

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

ALL_CFLAGS = -Wall -Werror-implicit-function-declaration -std=c99 $(CFLAGS)


##############################
# internal variables

export CP := cp
export RM_F := rm -f
export CAT := cat


##############################
# generic build targets, rules

.PHONY : all
all : $(TARGET)

.PHONY : run
run : $(TARGET)
	./$(TARGET)

# executable constructed using linker from object files
$(TARGET) : $(OBJS) $(OBJMRB) $(LIBR)
	$(LL) -o $@ $(CFLAGS) $^

-include $(OBJS:.o=.d) $(OBJMRB:.o=.d)

# mrby complie
$(OBJMRB) : %.o : %.rb
	$(MRBC) -Cinit_$(*F) $<
	$(CC) $(ALL_CFLAGS) -MMD $(INCLUDES) -c $(basename $<).c -o $@

# objects compiled from source
$(OBJS) : %.o : %.c
	$(CC) $(ALL_CFLAGS) -MMD $(INCLUDES) -c $< -o $@

# clean up
.PHONY : clean #cleandep
clean :
	@echo "make: removing targets, objects and depend files of `pwd`"
	-$(RM_F) $(TARGET) $(OBJS) $(OBJMRB) $(MRB)
	-$(RM_F) $(OBJS:.o=.d) $(OBJY:.o=.d)
	-$(RM_F) $(patsubst %.c,%.o,$(EXCEPT1)) $(patsubst %.c,%.d,$(EXCEPT1))
