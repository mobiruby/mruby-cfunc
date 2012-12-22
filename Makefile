GEM := mruby-cfunc

include $(MAKEFILE_4_GEM)

MRUBY_CFLAGS = -pthread -std=c99
MRUBY_LDFLAGS = -L`pwd`/lib -lffi

GEM_C_FILES := $(wildcard $(SRC_DIR)/*.c)
GEM_OBJECTS := $(patsubst %.c, %.o, $(GEM_C_FILES))

GEM_RB_FILES := $(wildcard $(MRB_DIR)/*.rb)

gem-all : lib/libffi.a $(GEM_OBJECTS) gem-c-and-rb-files

gem-clean : gem-clean-c-and-rb-files

gem-test : gem_test.rbtmp $(GEM_TEST_C_FILES) test/_rubyvm1.rbx
	$(MRBC) -Bgem_mrblib_irep_$(subst -,_,$(GEM))_test -ogem_test.ctmp gem_test.rbtmp
	$(RM_F) gem_test_vm1.ctmp
	$(MRBC) -Bmruby_data__rubyvm1 -ogem_test_vm1.ctmp test/_rubyvm1.rbx
	cat $(GEM_TEST_C_FILES) gem_test_vm1.ctmp >> gem_test.ctmp
	$(RM_F) gem_test_vm1.ctmp

test: tmp/mruby
	cd tmp/mruby; ENABLE_GEMS=true ./minirake test

tmp/mruby:
	mkdir -p tmp/mruby
	cd tmp; git clone https://github.com/mruby/mruby.git
	sed -i -e 's/\/\/\#define MRB_INT64/\#define MRB_INT64/' tmp/mruby/include/mrbconf.h
	pwd > tmp/mruby/mrbgems/GEMS.active

##################
# libffi.a
tmp/libffi:
	mkdir -p tmp/libffi
	cd tmp && git clone https://github.com/atgreen/libffi.git

lib/libffi.a: tmp/libffi
	mkdir -p vendors
	cd tmp/libffi && ./configure --prefix=`pwd`/../../vendors && make clean install CFLAGS="$(CFLAGS)"
	cp -r vendors/lib/libffi-3.0.11/include/* include/
	cp  vendors/lib/libffi.a lib/
