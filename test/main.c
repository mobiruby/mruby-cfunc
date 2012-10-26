#include "cfunc.h"

#include "mruby.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mruby/compile.h"

struct mrb_state_ud {
    struct cfunc_state cfunc_state;
};

const char* assert_rb = 
"$ok_test = 0" "\n"
"$ko_test = 0" "\n"
"$kill_test = 0" "\n"
"$asserts = []" "\n"
""
"def assert(test, failure_message = 'Assertion failed')" "\n"
"  begin" "\n"
"    if test" "\n"
"      $ok_test += 1" "\n"
"      print('.')" "\n"
"    else" "\n"
"      $asserts.push(['Fail: ', failure_message])" "\n"
"      $ko_test += 1" "\n"
"      print('F')" "\n"
"    end" "\n"
"  rescue Exception => e" "\n"
"    $asserts.push(['Error: ', failure_message, e])" "\n"
"    $kill_test += 1" "\n"
"    print('X')" "\n"
"  end" "\n"
"end" "\n"
""
"def assert_equal(expected, actual, failure_message = nil)" "\n"
"  failure_message ||= \"#{actual} expected '#{expected}' assertion failed\"" "\n"
"  assert(expected === actual, failure_message)" "\n"
"end" "\n"
""
"def assert_not_equal(expected, actual, failure_message = nil)" "\n"
"  assert(!(expected === actual), \"#{actual} expected not '#{expected}' assertion failed\")" "\n"
"end" "\n"
""
"def assert_raise(*args)" "\n"
"  failure_message = args.last.is_a?(String) ? args.pop : nil" "\n"
"  begin" "\n"
"    yield" "\n"
"    assert(false, \"#{args} expected but none was thrown.\")" "\n"
"  rescue Exception => e" "\n"
"    assert(args.include?(e.class), \"#{args} expected but was #{e}.\")" "\n"
"  end" "\n"
"end" "\n"
""
// This method copy from mruby/test/assert.rb" "\n"
// License: MITL mruby developers" "\n"
// Report the test result and print all assertions" "\n"
// which were reported broken." "\n"
"def test_results()" "\n"
"  print \"\\n\"" "\n"
"  $asserts.each do |err, str, e|" "\n"
"    print(err);" "\n"
"    print(str);" "\n"
"    if e" "\n"
"      print(\" => \")" "\n"
"      print(e.message)" "\n"
"    end" "\n"
"    print(\"\\n\")" "\n"
"  end" "\n"
""
"  $total_test = $ok_test.+($ko_test)" "\n"
"  print('Total: ')" "\n"
"  print($total_test)" "\n"
"  print(\"\\n\")" "\n"
""
"  print('   OK: ')" "\n"
"  print($ok_test)" "\n"
"  print(\"\\n\")" "\n"
"  print('   KO: ')" "\n"
"  print($ko_test)" "\n"
"  print(\"\\n\")" "\n"
"  print('Crash: ')" "\n"
"  print($kill_test)" "\n"
"  print(\"\\n\")" "\n"
""
"  ($ko_test + $kill_test) == 0" "\n"
"end";

static 
void mrb_state_init(mrb_state *mrb)
{
    mrb->ud = malloc(sizeof(struct mrb_state_ud));
    init_cfunc_module(mrb, mrb_state_init);
}


int main(int argc, char *argv[])
{
    printf("%s: ", appname);

    cfunc_state_offset = cfunc_offsetof(struct mrb_state_ud, cfunc_state);

    mrb_state *mrb = mrb_open();
    mrb_state_init(mrb);

    mrb_load_string(mrb, assert_rb);
    if (mrb->exc) {
        mrb_p(mrb, mrb_obj_value(mrb->exc));
        exit(1);
    }

    int n = mrb_read_irep(mrb, test_irep);
    mrb_run(mrb, mrb_proc_new(mrb, mrb->irep[n]), mrb_top_self(mrb));
    if (mrb->exc) {
        mrb_p(mrb, mrb_obj_value(mrb->exc));
        mrb_load_string(mrb, "test_results();");
        exit(1);
    }
    if(mrb_test(mrb_load_string(mrb, "test_results()"))) {
        exit(0);
    }
    else {
        exit(1);
    }
}
