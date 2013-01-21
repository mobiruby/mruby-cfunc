.PHONY : test
test:
	ruby ./run_test.rb test --verbose

.PHONY : all
all:
	echo "NOOP"

.PHONY : clean
clean:
	ruby ./run_test.rb clean
