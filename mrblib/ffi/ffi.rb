
module FFI
 C_NUMERICS = [CFunc::Int,
              CFunc::SInt8,
              CFunc::SInt16,
              CFunc::SInt32,
              CFunc::SInt64,
              CFunc::UInt32,
              CFunc::UInt8,
              CFunc::UInt16,
              CFunc::UInt32,
              CFunc::UInt64,
              CFunc::Float,
              CFunc::Double]
  TYPES = {
    :self => :pointer,
    :int=>CFunc::Int,
    :uint=>CFunc::UInt32,
    :bool=>CFunc::Int,
    :string=>CFunc::Pointer,
    :pointer=>CFunc::Pointer,
    :void=>CFunc::Void,
    :double=>CFunc::Double,
    :size_t=>CFunc::UInt32,
    :ulong=>CFunc::UInt64,
    :long=>CFunc::SInt64,
    :uint64=>CFunc::UInt64,
    :uint8=>CFunc::UInt8,
    :uint16=>CFunc::UInt16,
    :uint32=>CFunc::UInt32,
    :int64=>CFunc::SInt64,
    :int16=>CFunc::SInt16,
    :int8=>CFunc::SInt8,
    :int32=>CFunc::Int,
    :short=>CFunc::SInt16,
    :ushort=>CFunc::UInt16,
    :callback=>CFunc::Closure,
    :struct=>CFunc::Pointer,
    :array=>CFunc::CArray
  }

end

module CFunc
  define_function CFunc::Pointer,"dlopen",[CFunc::Pointer,CFunc::Int]
  class Library
    @@instances = {}
    def initialize where
      @funcs = {}
      @@instances[where] = self
      @libname=where
      if @libname.to_s == 'c'
        @libname = nil
      end
      
      @dlh = CFunc::call(CFunc::Pointer, "dlopen", @libname, CFunc::Int.new(1))
    end

    def call rt,name,*args
      # @dlh ||= CFunc[:dlopen].call(@libname,CFunc::Int.new(1))
      if !(f=@funcs[name])
        fun_ptr = CFunc::call(CFunc::Pointer,:dlsym,@dlh,name)
        f = CFunc::FunctionPointer.new(fun_ptr)
        f.result_type = rt
        @funcs[name] = f
      end
      
      f.arguments_type = args.map do |a| a.class end      
      return f.call(*args)
    end
    
    def self.for where
      if n=@@instances[where]
        n
      else
        return new(where)
      end
    end
  end
  
  def self.libcall2 rt,where,n,*o
    return CFunc::Library.for(where).call rt,n,*o
  end
end


module FFI; 
  class FFI::Closure < CFunc::Closure
    CLOSURES = {}
  
    def initialize *o,&b
      @block = b
      super(*o) do |*a|
        @block.call(*a)
      end
    end
    
    def set_closure b
      @block = b
    end
    
    def self.add name,rt,at,&b
      return FFI::Closure::CLOSURES[name] ||= FFI::Closure.new(rt,at,&b)
    end
  end
  
  class DefaultClosure < FFI::Closure
    def initialize &b
      super CFunc::Void,[],&b
    end
  end
end


class FFI::Pointer < CFunc::Pointer
  def self.new(*args)
    if args.size == 1
      new(args[0])
    else
      type, ptr = args
      CFunc::Pointer(type).refer(ptr.addr)
    end
  end
  
  def write_array_of type,a
    ca = CFunc::CArray(TYPES[type]).refer(self.addr)

    a.each_with_index do |q,i|
      ca[i].value = q
    end
    return self
  end
  
  def read_void
    nil
  end
  
  def read_array_of type,len
    ca = CFunc::CArray(TYPES[type]).refer(self.addr)
    (0...len).map do |i|
      type == :pointer ? FFI::Pointer.refer(ca[i].value.addr) : ca[i].value
    end 
  end
  
  def write_array_of_string sa
    ca = CFunc::CArray(CFunc::Pointer).refer(self.addr)
    subt = 0
    
    sa.each_with_index do |q,i|
      ia = CFunc::SInt8[q.length]
      c = 0
      
      q.each_byte do |b|
        ia[c].value = b
        c += 1
      end
      
      ia[c].value = 0
      ca[i].value = ia
    end
    return self
  end
  
  def read_array_of_string len
    read_array_of :pointer,len do |y|
      yield CFunc::CArray(CFunc::SInt8).refer(y.addr).to_s
    end
  end
  
  def get_pointer offset
    FFI::Pointer.refer(self[offset].addr)
  end
  
  def read_string
    value.to_s
  end
  
  def write_string(str)
    CFunc::call(CFunc::Pointer, "strcpy", self, str)
    # ca = CFunc::CArray(CFunc::SInt8).refer(self.addr)
    # c = 0
    
    # s.each_byte do |b|
    #   ca[c].value = b
    #   c += 1
    # end
    
    # ca[c].value = 0
    return self
  end
  
  def read_type type
    return TYPES[type].get(self)
  end
  
  def write_type n,type
    TYPES[type].set(self, n)
    return self
  end
  
  FFI::TYPES.keys.each do |k|
    unless k == :string or k == :array
      define_method :"read_#{k}" do
        next read_type k
      end

      define_method :"write_#{k}" do |v|
        next write_type v,k
      end
      define_method :"write_array_of_#{k}" do |v|
        next write_array_of k,v
      end  
      define_method :"read_array_of_#{k}" do |v,&b|
        next read_array_of(k,v)
      end            
    end
  end
  
  def read_bool
    return read_int == 1
  end
  
  def read_array_of_bool len
    read_array_of_int len do |i|
      yield i == 1
    end
    return nil
  end
end
  
class FFI::MemoryPointer < FFI::Pointer
  def self.new *o
    count = 1
    clear = false
    
    if o.length == 1
      size = o[0]
      if size.is_a?(Numeric)
        return malloc(size)
      end
    elsif o.length == 2
      size, count = o
    elsif o.length == 3
      size,count,clear = o
    else
      raise "arguments error > 4 for 1..3"
    end
    
    s = TYPES[size].size
    ins = malloc(s*count)

    return ins
  end  
end

module FFI
  class Struct < CFunc::Struct
    def self.members
      elements.map{|e| e[1] }
    end
    
    def self.is_struct?
      true
    end
    def self.every(a,i)
      b=[]
      q=a.clone
      d=[]
      c=0
      until q.empty?
        for n in 0..i-1
          d << q.shift
        end

        d[1] = FFI.find_type(d[1]) unless d[1].respond_to?(:"is_struct?")
        b.push(*d.reverse)
        d=[]
      end
      b
    end
  
    def self.layout *o
      define(*every(o,2))
    end
  end
  class Union < Struct
  end  
  
  def self.type_size t
    return FFI::TYPES[t].size
  end
end

class Array
  def find_all_indices a=self,&b
    o = []
    a.each_with_index do |q,i|
      if b.call(q)
        o << i
      end
    end
    return o
  end
  
  def flatten
    a=[]
    each do |q|
      a.push(*q)
    end
    return a
  end  
end




module FFI::Library
  def ffi_lib n=nil
    @ffi_lib = n if n
    return @ffi_lib
  end
  
  @@callbacks = {}
  
  def callback n,at,rt
    return @@callbacks[n] = [at.map do |t| FFI::TYPES[t] end,FFI::TYPES[rt]]
  end
  
  def self.callbacks
    return @@callbacks
  end
  

  
  def typedef *o
    return FFI::TYPES[o[1]] = q=FFI.find_type(o[0])
  end  
  
    @@enums = {}
    def enum t,a
      if a.find() do |q| q.is_a?(Integer) end
        b = []
 
        for i in 0..((a.length/2)-1)
          val= a[i*2] 
          idx = a[(i*2)+1]
          b[idx] = val
        end

        a=b
      end
      @@enums[t] = a
      typedef :int,t
      return self
    end

    def self.enums
      r=@@enums
      return r
    end  
  
  def attach_function name,at,rt
    f=add_function ffi_lib,name,at,rt

    self.class_eval do
      class << self;self;end.define_method name do |*o,&b|  
        next f.invoke(*o,&b)
      end
    end
    return self
  end
end

module FFI
  def self.errno
    return CFunc.errno
  end
  
  def self.find_type t
    return FFI::TYPES[t] || CFunc::Pointer
  end  
end

class Symbol
  def enum?
    return FFI::Library.enums[self]
  end
end
