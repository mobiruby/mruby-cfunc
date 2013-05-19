class CFunc::Type
    def to_ffi_value(ffi_type)
        self.addr
    end

    def self.ffi_type
        @ffi_type
    end
end

module CFunc
    def self.Int(val); Int.new(val) end
    def self.UInt8(val); UInt8.new(val) end
    def self.SInt8(val); SInt8.new(val) end
    def self.UInt16(val); UInt16.new(val) end
    def self.SInt16(val); SInt16.new(val) end
    def self.UInt32(val); UInt32.new(val) end
    def self.SInt32(val); SInt32.new(val) end
    def self.UInt64(val); UInt64.new(val) end
    def self.SInt64(val); SInt64.new(val) end
    def self.Float(val); Float.new(val) end
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
    class FunctionPointer < Pointer
        attr_accessor :result_type, :arguments_type

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

    def self.define_function(result_type, funcname, *args)
        @dlh ||= CFunc::call(CFunc::Pointer, :dlopen, nil, nil)
        @dlsym_func ||= CFunc::call(CFunc::Pointer, :dlsym, @dlh, "dlsym")

        funcptr = FunctionPointer.new(CFunc::call(CFunc::Pointer, @dlsym_func, @dlh, funcname))
        funcptr.result_type = result_type
        funcptr.arguments_type = args

        @funcptrs ||= Hash.new
        @funcptrs[funcname.to_sym] = funcptr
    end

    def self.[](funcname)
        @funcptrs[funcname.to_sym]
    end
end

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
end

class String
    def self.to_ffi_type
        CFunc::Pointer(CFunc::UInt8).to_ffi_type
    end

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
    def self.Pointer(type)
        return Pointer if Void == type
        klass = ::Class.new(Pointer)
        klass.type = type
        $klasses << klass
        klass
    end
end

class CFunc::CArray < CFunc::Pointer
    attr_accessor :size

    def self.new(size)
        obj = self.malloc(type.size * size)
        obj.size = size
        obj
    end
end

module CFunc
    def self.CArray(type)
        klass = ::Class.new(CArray)
        klass.type = type
        klass
    end

    class Int < SInt32
    end
end

class CFunc::Struct
    class << self
        attr_accessor :elements, :size, :align
        
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
                offset = calc_align(offset, el[0].align)
                types << el[0]
                @elements << [el[0], el[1].to_s, offset]
                offset += el[0].size
                max_align = el[0].align if el[0].align > max_align
            end
            @align = @size = offset + ((-offset) & (max_align - 1))
            klass.define_struct(types)
            klass
        end
    end

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
            raise "Unknow"
        end
    end

    def self.refer(pointer)
        self.new(pointer)
    end
    
    def element(fieldname)
        field = lookup(fieldname)
        field[0].refer(@pointer.offset(field[2]))
    end
    
    def [](fieldname)
        field = lookup(fieldname)
        el = field[0].refer(@pointer.offset(field[2]))
        el.respond_to?(:value) ? el.value : el
    end

    def []=(fieldname, val)
        field = lookup(fieldname)
        el = field[0].refer(@pointer.offset(field[2]))
        el.value = val
    end

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
