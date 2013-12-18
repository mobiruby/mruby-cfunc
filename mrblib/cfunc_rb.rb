class CFunc::Type
    def to_ffi_value(ffi_type)
        self.addr
    end

    def self.ffi_type
        @ffi_type
    end
end

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
    # A pointer to a function
    class FunctionPointer < Pointer
        attr_accessor :result_type, :arguments_type

        #
        # @param [void] args swarm of arguments (if any) to pass to the function
        # @return [::Object] the result of calling the function
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
    # @see(CFunc::[])
    #
    # @param [CFunc::Type] result_type the type of the result of calling the function
    # @param [String] funcname the name of the function (note: the name must be of a symbol in `@dlh` or from a library that `@dlh` is linked against)
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
class CFunc::Pointer
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
    # When passed a Hash we will create the pointer we wrap, its size will be the size of the struct multiplied by the value of the `:size` member of the hash
    # When passed nil or when no arguments are passed we create the pointer we wrap
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
