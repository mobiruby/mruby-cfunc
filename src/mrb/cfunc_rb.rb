class CFunc::Type
    def to_ffi_type
        self.class.to_ffi_type
    end

    def to_ffi_value(ffi_type)
        self.to_pointer
    end
end

module CFunc
    def self.Int(val); Int.new(val) end
    def self.UInt8(val); UIn8.new(val) end
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
        def to_i
            self.value.to_i
        end

        def to_f
            self.value.to_f
        end

        def to_s
            self.value.to_s
        end
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

    # defined to_pointer in cfunc_type.c
    def to_ffi_value(ffi_type)
        self.to_pointer
    end
end

module CFunc
    def self.Pointer(type)
        return Pointer if Void == type
        klass = ::Class.new(Pointer)
        klass.type = type
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

module Enumerable
    def each_slice(n, &block)
        ary = []
        each_with_index do |val, i|
            ary << val
            if i % n == n - 1
                block.call(ary)
                ary = []
            end
        end
    end
end

class CFunc::Struct
    class << self
        attr_accessor :elements, :size, :align
        
        def define(*args)
            args = args.first if args.is_a?(Array) && args.size == 1
            @elements = []
            types, offset = [], 0
            max_align = 0
            args.each_slice(2) do |el|
                offset = calc_align(offset, el[0].align)
                types << el[0]
                @elements << [el[0], el[1].to_s, offset]
                offset += el[0].size
                max_align = el[0].align if el[0].align > max_align
            end
            @align = @size = offset + ((-offset) & (max_align - 1))
            self.define_struct(types)
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

    def to_pointer
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
$mruby_cfunc_null_pointer = CFunc::Pointer.new.to_pointer
class NilClass
    def to_pointer
        $mruby_cfunc_null_pointer
    end

    def to_ffi_value(ffi_type)
        $mruby_cfunc_null_pointer
    end
end

$mruby_cfunc_true_pointer = CFunc::Int(-1).to_pointer
class TrueClass
    def to_pointer
        $mruby_cfunc_true_pointer
    end

    def to_ffi_value(ffi_type)
        $mruby_cfunc_true_pointer
    end
end

$mruby_cfunc_false_pointer = CFunc::Int(0).to_pointer
class FalseClass
    def to_pointer
        $mruby_cfunc_false_pointer
    end

    def to_ffi_value(ffi_type)
        $mruby_cfunc_false_pointer
    end
end
