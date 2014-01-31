# @abstract 
class CFunc::Type
    # @!scope class
    # @!method size()
    # @!method align()
    # @!method get(ptr)
    # @!method set(ptr,val)
    # @!method refer(ptr) 

    # @!scope instance
    # @!method addr()
    # @!method set(val)
    # @!method get()
    
    def to_ffi_value(ffi_type)
        self.addr
    end

    def self.ffi_type
        @ffi_type
    end
end

# @example
#    #
#    # Calling functions
#    #
#
#    # call libc's 'puts'
#    #   note: the first argument, the result type, could be 'nil'
#    CFunc::define_function(CFunc::Void, 'puts', CFunc::Pointer);
#    CFunc[:puts].call('foo')
#
#    # Use a function from a library
#    # Say we have a library (./mylib.so)
#    =begin
#      int
#      my_add(int a, int b)
#      {
#        return(a + b);
#      }
#
#      int*
#      my_array()
#      {
#        static int a[3];
#        a[0]=65;
#        a[1]=66;
#        a[2]=67;
#        return a;
#      }
#
#      void 
#      my_out(int* a)
#      {
#        static int b[2];
#        b[0]=0;
#        b[1]=1;
#        *a = b;
#      }
#    =end
#
#    #
#    # Dynamic Loading (specified by path unless findable by dlopen())
#    #
#
#    # Get a Dynamic Loaded library Handle
#    dlh     = CFunc::call(CFunc::Pointer, "dlopen", "./mylib.so", CFunc::Int.new(1))
#
#    #
#    # Get function pointers from the handle
#    #
#
#    fun_ptr = CFunc::call(CFunc::Pointer, :dlsym, dlh, "my_add")
#    add     = CFunc::FunctionPointer.new(fun_ptr)
#
#    add.result_type    = CFunc::Int
#    add.arguments_type = [CFunc::Int, CFunc::Int]
#
#    fun_ptr = CFunc::call(CFunc::Pointer, :dlsym, dlh, "my_array")
#    array   = CFunc::FunctionPointer.new(fun_ptr)
#
#    array.result_type    = CFunc::CArray(CFunc::Int)
#    array.arguments_type = []
#
#    # This functions takes an out parameter
#    fun_ptr = CFunc::call(CFunc::Pointer, :dlsym, dlh, "my_out")
#    out   = CFunc::FunctionPointer.new(fun_ptr)
#
#    out.result_type    = CFunc::Void
#    # The out parameter's type will be CFunc::Pointer
#    out.arguments_type = [CFunc::Pointer]
#
#    #
#    # Calling functions
#    #
#    add.call(1,3).value                                    #=> 4
#
#    a = array.call(); a = (0..2).map do |i| a[i].value end #=> [65,66,67]
#
#    # Out parameter
#    a = CFunc::Int[2]
#    out.call(a.addr)
#    a = (0..1).map do |i| a[i].value end                   #=> [0,1]
#
#    # 
#    # Type casting
#    # Descendants of CFunc::Type have a `refer` method that takes a pointer argument
#    # note how we pass a pointer to the address of the value
#    #
#
#    # Lets make our functions return pointers
#    add.result_type   = CFunc::Pointer
#    array.result_type = CFunc::Pointer
#
#    CFunc::Int.get(add.call(1,3).addr)                                                           #=> 4
#    a = CFunc::CArray(CFunc::Int).refer(array.call().addr); a = (0..2).map do |i| a[i].value end #=> [65,66,67]
#
#    #
#    # Memory Management
#    # CFunc::Pointer#autofree
#    #
#
#    # This pointer is free'd once GC-ed
#    ptr = CFunc::Pointer.malloc(8)
#    ptr.autofree
module CFunc
    # 
    # @param [Integer] val the number to represent
    # @return [CFunc::Int] representing +val+
    def self.Int(val); Int.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::UInt8] representing +val+
    def self.UInt8(val); UInt8.new(val) end

    #
    # @param [Integer] val the number to represent
    # @return [CFunc::SInt8] representing +val+    
    def self.SInt8(val); SInt8.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::UInt16] representing +val+
    def self.UInt16(val); UInt16.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::SInt16] representing +val+    
    def self.SInt16(val); SInt16.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::UInt32] representing +val+    
    def self.UInt32(val); UInt32.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::SInt32] representing +val+    
    def self.SInt32(val); SInt32.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::UInt64] representing +val+    
    def self.UInt64(val); UInt64.new(val) end
    
    #
    # @param [Integer] val the number to represent
    # @return [CFunc::SInt64] representing +val+    
    def self.SInt64(val); SInt64.new(val) end
    
    #
    # @param [Numeric] val the number to represent
    # @return [CFunc::Float] representing +val+    
    def self.Float(val); Float.new(val) end
    
    #
    # @param [Numeric] val the number to represent
    # @return [CFunc::Double] representing +val+    
    def self.Double(val); Double.new(val) end

    class Type
        # Shorthand to create an CFunc::CArray of type `self` with +size+ elements
        # @note The returned value's class is a subclass of CFunc::CArray
        #
        # @param [Integer] size the number of elements
        # @return [CFunc::CArray]  
        def self.[](size)
            CFunc::CArray(self).new(size)
        end
        
        def to_i
            val = self.value
            val ? val.to_i : 0
        end

        def to_f
            val = self.value
            val ? val.to_f : 0
        end

        def to_s
            self.value.to_s
        end
    end
end


module CFunc
    class FunctionPointer < Pointer
        attr_accessor :result_type, :arguments_type

        #
        # @param [void] args swarm of arguments (if any) to pass to the function
        # @return [CFunc::Type] the result of calling the function
        def call(*args)
            @result_type ||= CFunc::Void
            @arguments_type.each_with_index do |arg_type, idx|
                unless args[idx].is_a?(String) || args[idx].respond_to?(:addr)
                    args[idx] = arg_type.new(args[idx])
                end
            end
            CFunc::call(@result_type, self, *args)
        end
    end

    # Stores a function
    # @see []
    #
    # @param [CFunc::Type] result_type the type of the result of calling the function
    # @param [String] funcname the name of the function (note: only symbols from libc)
    # @param [void] args swarm of CFunc::Type's (if any) of the argument types.
    def self.define_function(result_type, funcname, *args)
        @dlh ||= CFunc::call(CFunc::Pointer, :dlopen, nil, nil)
        @dlsym_func ||= CFunc::call(CFunc::Pointer, :dlsym, @dlh, "dlsym")

        funcptr = FunctionPointer.new(CFunc::call(CFunc::Pointer, @dlsym_func, @dlh, funcname))
        funcptr.result_type = result_type
        funcptr.arguments_type = args

        @funcptrs ||= Hash.new
        @funcptrs[funcname.to_sym] = funcptr
    end

    #
    #
    # @return [CFunc::FunctionPointer] mapped to +funcname+. returns `nil` if no such mapping.
    def self.[](funcname)
        @funcptrs[funcname.to_sym]
    end
end

# A pointer
class CFunc::Pointer < CFunc::Type
    class << self
        def type
            @type || CFunc::Void
        end

        def type=(type_)
            raise "Can't update" if CFunc::Pointer == self
            @type = type_
        end
    end

   
    def refer
        self.class.type.refer(self)
    end

    def [](index)
        self.class.type.refer(self.offset(index * self.class.type.size))
    end

    def []=(index, val)
        self.class.type.refer(self.offset(index * self.class.type.size)).value = val
    end

end

class String
    def self.to_ffi_type
        CFunc::Pointer(CFunc::UInt8).to_ffi_type
    end
    
    # Get a String from a pointer
    #
    # @param [CFunc::Pointer] ptr
    # @return [String] the string value of +ptr+
    def self.refer(ptr)
        CFunc::Pointer.refer(ptr).to_s
    end

    # defined addr in cfunc_type.c
    def to_ffi_value(ffi_type)
        self.addr
    end
end

$klasses = []
module CFunc
    # Creates a class subclassing CFunc::Pointer, and sets it +type+
    #
    # @param [CFunc::Type] type the type of the pointer
    # @return [::Class]
    def self.Pointer(type)
        return Pointer if Void == type
        klass = ::Class.new(Pointer)
        klass.type = type
        $klasses << klass
        klass
    end
end

# Wraps a pointer of an array
# @example
#    # This allocates the space for four elements of SInt32
#    ary = CFunc::CArray(CFunc::Int)[4]
#    # set the values
#    ary[0].value = 1
#    ary[1].value = 2
#    ary[2].value = 3
#    ary[3].value = 4
#    # get a value
#    ary[1].value #=> 2
class CFunc::CArray < CFunc::Pointer
    attr_accessor :size

    #
    # @param [Integer] size the size of the array
    # @return [CFunc::CArray]
    def self.new(size)
        obj = self.malloc(type.size * size)
        obj.size = size
        obj
    end
end

module CFunc
    # Creates a class subclassing CFunc::CArray whose `type` is +type+ 
    #
    # @param [CFunc::Type] type the type of the array
    # @return [::Class]
    def self.CArray(type)
        klass = ::Class.new(CArray)
        klass.type = type
        klass
    end

    # Represents int8_t
    class SInt8 < Type; end
    # Represents int16_t    
    class SInt16 < Type; end
    # Represents int32_t    
    class SInt32 < Type; end
    # Represents int64_t    
    class SInt64 < Type; end
    
    # Represents uint8_t      
    class UInt8 < Type; end
    # Represents uint16_t      
    class UInt16 < Type; end
    # Represents uint32_t      
    class UInt32 < Type; end
    
    # Represents uint64_t      
    class UInt64 < Type;
      # @!method divide(q)
      # @!method low()
      # @!method low=(val)
      # @!method high()
      # @!method high=(val)
    end  
    
    # Represents float      
    class Float < Type; end
    # represents double
    class Double < Type; end
    
    # A Void type
    class Void < Type;end  
            
    # Convienience, is CFunc::SInt32
    class Int < SInt32
    end
end

# @abstract Implements wrapping a struct
# Must be subclassed (either directly or as the result of CFunc::Struct.define())
#
# @example
#   =begin
#     typedef struct {
#       int    bar;
#       double quux;
#     } Foo;
#   =end
#
#   class Foo < CFunc::Struct
#     define CFunc::Int, :bar,
#            CFunc::Double, :quux   
#   end
class CFunc::Struct
    class << self
        attr_accessor :elements, :size, :align
    
        # Defines the fields and thier types. 
        # Returns a subclass of CFunc::Struct OR if called on a subclass, the subclass
        #
        # @example
        #   Foo.define CFunc::Int, :bar, CFunc::Double, :quux
        #
        # @param [void] args the layout
        # @return [::Class] the resulting class    
        def define(*args)
            args = args.first if args.is_a?(Array) && args.size == 1
            @elements = []
            types, offset = [], 0
            klass = self
            if klass == CFunc::Struct
              return Kernel.const_set(args.shift.to_s, Class.new(CFunc::Struct)).define(*args)
            end
            
            max_align = 0
            args.each_slice(2) do |el|
                if el[0].is_a?(CFunc::CArray)
                    type = el[0].class.type
                    @elements << [CFunc::Pointer(type), el[1].to_s, offset, :new]
                    align = type.align
                    size = type.size
                    el[0].size.times do |i|
                        offset = calc_align(offset, align)
                        types << type
                        @elements << [type, "#{el[1]}-#{i}", offset]
                        offset += size
                    end
                    max_align = align if align > max_align
                else
                    align = el[0].align
                    size = el[0].size
                    offset = calc_align(offset, align)
                    types << el[0]
                    @elements << [el[0], el[1].to_s, offset]
                    offset += size
                    max_align = align if align > max_align
                end
            end
            @align = @size = offset + ((-offset) & (max_align - 1))
            klass.define_struct(types)
            klass
        end
    end

    # When passed a CFunc::Pointer we will wrap that pointer
    # When passed a Hash we will create the pointer we wrap, its size will be the size of the struct multiplied by the value of the `:size` member of the hash and set it to be autofree'd
    # When passed nil or when no arguments are passed we create the pointer we wrap and set it to be autofree'd
    #
    # @raise [RuntimeError] when passed something we cant handle
    #
    # @param [void] val
    def initialize(val=nil)
        if val == nil
            @pointer = CFunc::Pointer.malloc(self.class.size)
            @pointer.autofree
        elsif val.is_a?(CFunc::Pointer)
            @pointer = val
        elsif val.is_a?(Hash)
            @pointer = CFunc::Pointer.malloc(self.class.size * val[:size])
            @pointer.autofree
        else
            raise "Do not know how to initialize the structure from #{val}"
        end
    end

    # Wrap a pointer
    # 
    # @param [CFunc::Pointer] pointer the pointer to wrap
    # @return [CFunc::Struct] the instance of the class being called on
    def self.refer(pointer)
        if self == CFunc::Struct
          raise "CFunc::Struct cannot be used directly"
        end
    
        self.new(pointer)
    end
    
    # Retieve the value of member +fieldname+
    #
    # @param [Symbol] fieldname the name of the field to retrive
    # @return [CFunc::Type] the value
    def element(fieldname)
        field = lookup(fieldname)
        field[0].refer(@pointer.offset(field[2]))
    end
    
    # Retrieves the value of +fieldname+
    # if the CFunc::Type for that field responds to `value`
    # then the result of `value` called on that CFunc::Type is returned
    #
    # @param [Symbol] fieldname the name of the field to retrieve
    # @return [Object, CFunc::Type] the value
    def [](fieldname)
        field = lookup(fieldname)
        if field[3] == :new
            el = field[0].new(@pointer.offset(field[2]))
        else
            el = field[0].refer(@pointer.offset(field[2]))
            el.respond_to?(:value) ? el.value : el
        end
    end

    # Sets a fields value
    # the fields CFunc::Type must respond to `value=`
    #
    # @param [Symbol] fieldname the name of the field to set
    # @param [Object] val the value to set
    # @return [void]
    def []=(fieldname, val)
        field = lookup(fieldname)
        el = field[0].refer(@pointer.offset(field[2]))
        el.value = val
    end

    # @return [CFunc::Pointer] the pointer we wrap
    def addr
        @pointer
    end

    def to_ffi_value(ffi_type)
        @pointer
    end

    # private
    def lookup(fieldname)
        fieldname = fieldname.to_s
        field = self.class.elements.find{|el| el[1] == fieldname}
        raise "not defined #{self.class}.#{fieldname}" unless field
        field
    end

    # private
    def self.calc_align(offset, alignment)
        rest = offset % alignment
        (0 == rest) ? offset : (offset + alignment - rest)
    end
end



# immediate values cannot have instance variables.
# it's mruby limitation.
# https://github.com/mruby/mruby/issues/438
$mruby_cfunc_null_pointer = CFunc::Pointer.new.addr
class NilClass
    def addr
        $mruby_cfunc_null_pointer
    end
    
    def to_ffi_value(ffi_type)
        self.addr
    end
end


$mruby_cfunc_true_pointer = CFunc::Int(1).addr
class TrueClass
    def addr
        $mruby_cfunc_true_pointer
    end

    def to_ffi_value(ffi_type)
        self.addr
    end
end

TrueClass.instance_eval do
    @ffi_type = CFunc::Int.ffi_type
end


$mruby_cfunc_false_pointer = CFunc::Int(0).addr
class FalseClass
    def addr
        $mruby_cfunc_false_pointer
    end
    
    def to_ffi_value(ffi_type)
        self.addr
    end
end

FalseClass.instance_eval do
    @ffi_type = CFunc::Int.ffi_type
end
