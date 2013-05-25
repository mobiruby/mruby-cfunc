module FFIType2CFuncType
  # Returns a Class of CFunc namespace types
  def ffi_type
    case type
    when :object
      CFunc::Pointer
    when :struct
      CFunc::Pointer
    when :union
      CFunc::Pointer
    when :array
      CFunc::CArray(array.ffi_type)
    else
      FFI::TYPES[type] || CFunc::Pointer
    end
  end
end

# Represents Argument/Return array member
# specifies properties of the array
class ArrayStruct < Struct.new(:length,:fixed_size,:type,:enum,:zero_terminated)
end


class ClosureStruct < Struct.new(:return_type,:arguments)
end

module Cvalue2RubyValue
  # Converts a C value to Ruby from information of a (Argument || Return)
  def to_ruby ptr,len=nil
    if array
      if !len
        len = array.length
      end
     
      if len < 0
        len = array.fixed_size
      end    
     
      a = []
      tt = type == :array ? array.type : type
      #if !array.zero_terminated
  	    if tt == :enum
		  ptr.send :"read_array_of_#{:int}",len do |q|
		    a << q
		  end
		  return a
	    else
		  ptr.send :"read_array_of_#{tt}",len do |q|
		    a << q
		  end
		  return a
	    end
      #else

      #end
    elsif type == :pointer
      return ptr
    elsif type == :enum
      r = ptr.send :"read_int"
      return r if r == -1
      return enum.enum?[r]
    elsif type == :object or type == :struct
      ns = object[:namespace]
      name = object[:name]
      return ::Object.const_get(ns).const_get(name).wrap ptr
    else 
      tt = type
      return ptr.send(:"read_#{tt}")  
    end
  end
  
  # true if type is a C numeric type
  def is_numeric?
    FFI::C_NUMERICS.index(FFI::TYPES[type])
  end
  
  # Converts Symbol to Integer
  # if v is a member of the (Argument || Return)#enum
  def get_enum v
    r = enum.enum?
    if r
      r = r.map do |v| v.name.to_sym end
    end
    r.index(v)
  end
end 

# Represents Argument information
# and defines ways to create and convert
# Arguments to/from c
class Argument < Struct.new(:object,:type,:struct,:array,:direction,:allow_null,:callback,:closure,:destroy,:value,:index,:enum)
  # Sets value from an argument passed to Function#invoke
  def set v
    self[:value] = v
  end
  
  include FFIType2CFuncType
  
  # Resolve and create the appropiate 
  # pointer from Argument properties for Function#invoke
  def make_pointer mul = 1
    if type == :callback
      if value.is_a?(CFunc::Closure)
        return value
      elsif cb=FFI::Library.callbacks[callback]
        return FFI::Closure.add callback,cb[1],cb[0]
      else
        return FFI::DefaultClosure.new
      end
    elsif array
      ptr = CFunc::Pointer.malloc(4)
      return FFI::MemoryPointer.refer(ptr)
    else
      tt = FFI::TYPES[type] ? type : :pointer
      
       q=FFI::MemoryPointer.new(tt,mul)
      return q#FFI::MemoryPointer.new(tt,mul)
    end
  end
  
  include Cvalue2RubyValue
  
  # Sets the appropiate value for an Argument for Function#invoke
  def for_invoke
    if type == :string and direction == :in
      return value
    end
    
    if type == :pointer
      if value.is_a?(CFunc::Pointer)
        return value
      end
    elsif type == :object
        return value
    end
    
    ptr = make_pointer

    if type == :callback
      ptr.set_closure value if ptr.respond_to?(:set_closure)
      return ptr
    end
    
    tt = type
    
    tt = (tt == :enum ? :int : tt)
    
    if value;
      q = value
      if type == :enum
        if value.is_a?(Symbol)
          q = get_enum(value)
        end
      end
      
      if type == :pointer
        unless q.is_a?(CFunc::Pointer)
          if q == 0
            q = CFunc::Int.new(0)
          end
        end
      end

      if array
        tt = array.type
        meth = :"write_array_of_#{tt}"
      else
        meth = :"write_#{tt}"
      end

      ptr.send meth,q unless direction == :out

    elsif allow_null or omit?
      ptr.write_int 0 unless direction == :out
    end
 
    return ptr
  end
 
  # Returns true if the argument may be omitted from invoke arguments
  # The argument will be automatically resolved to nil
  # Callbacks are resolved to passed block of Function#invoke or nil
  def omit?
    (type == :destroy) || (type == :error) || (type == :callback) || (type == :data) || (direction == :out)
  end
  
  # Returns  true if the argument is allowed to be null
  # may be ommitted from arguments to Function#invoke
  def optional?
    allow_null
  end
end


# Represents information of a return_value of Function#invoke
class Return < Struct.new(:object,:type,:struct,:array,:enum)
  include FFIType2CFuncType
  
  def get_c_type
    if type == :enum
      CFunc::Int
    elsif type == :object
      CFunc::Pointer
    else
      FFI::TYPES[type]
    end
  end
  
  include Cvalue2RubyValue
  
  # returns a Ruby value of the passes ptr
  # resolved from Return properties
  def to_ruby ptr,len=1
    if FFI::C_NUMERICS.index(ptr.class)
      if type == :bool
        ptr.value == 1
      else
        n = ptr.value
  
        if type == :enum
          return enum.enum?[n]
        end
  
        return n
      end
    else
      ptr = FFI::Pointer.refer(ptr.addr)
      q=super ptr,len
      return q
    end
  end
end
