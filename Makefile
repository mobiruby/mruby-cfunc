.PHONY : test
test:
	ruby ./test_build_config.rb test --verbose

.PHONY : all
all:
	echo "NOOP"

.PHONY : clean
clean:
	ruby ./test_build_config.rb clean
