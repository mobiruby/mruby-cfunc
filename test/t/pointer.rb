ptr1 = CFunc::Pointer.new
assert ptr1.is_null?


ptr2 = CFunc::Pointer.malloc(10)
assert !ptr2.is_null?
ptr2a = ptr2.offset(5)

result = CFunc::call(CFunc::Int, "check_offset5", ptr2, ptr2a)
assert_equal 1, result.to_i

ptr2.free # check by MallocGuard or Valgrind


ptr3 = CFunc::Pointer.malloc(10)
assert !ptr3.realloc(20).is_null?


ptr4 = CFunc::Pointer.malloc(10)
assert !ptr4.realloc(1).is_null?


############
# BEGIN C
int check_offset5(uint8_t *p1, uint8_t *p2) {
    return (p1+5)==p2 ? 1 : 0;
}