class Function
  attr_reader :return_type

  def initialize where,name,args,return_type,results=[-1]
    @arguments = args
    @where = where
    @name = name
    @return_type = return_type
    @results = results
  end
  
  def set_closure closure
    @closure = closure
  end
  
  
	def get_return_type
	  return @return_type.get_c_type
	end
    
  def get_omits
    oa = arguments.find_all do |a|
      a.omit?
    end
    
    oidxa = oa.map do |o|
      arguments.index o
    end
    
    return oidxa
  end
  
  def get_optionals
    oa = arguments.find_all do |a|
      a.optional?
    end
    
    oidxa = oa.map do |o|
      arguments.index o
    end
    
    return oidxa
  end

  def get_required
    a=arguments.find_all do |a|
      !a.omit? and !a.optional?
    end
    
    oidxa = a.map do |o|
      arguments.index o
    end
    
    return oidxa
  end
  
  def get_rargs
    map = arguments.find_all do |a| !a.omit? end.map do |a| arguments.index a end
    return map
  end
  
  def get_optionals_start
    oa = get_optionals
    
    get_required.each do |r|
      oa.find_all do |o|
        o < r
      end.each do |i|
        oa.delete(i)
      end
    end
    
    return oa.first
  end
  
  def get_args_to_return
    map = arguments.find_all do |a| a.direction == :out end.map do |a| arguments.index(a) end.map do |i|
      arguments[i]
    end
    
    return map
  end
  
  attr_reader :arguments
  def invoke *args,&b

    have = args.length
    required = len=get_required.length
    max = len+get_optionals.length
    rargs = []

    args.each_with_index do |a,i|
      if a.respond_to?(:to_ptr)
        args[i] = a.to_ptr
      end
      rargs << get_rargs[i]
    end 
    
    if idx = get_optionals_start
      required += get_optionals.find_all do |o|
        o < idx
      end.length
    end
    
    raise "Too few arguments. #{have} for #{required}..#{max}" if have < required
    raise "Too many arguments. #{have} for #{required}..#{max}" if have > max


    rargs.each_with_index do |i,ri|
      arguments[i].set args[ri]
    end
    
    get_optionals.find_all do |i|
      !rargs.index(i)
    end.each do |i|
      arguments[i].set nil
    end
    
    get_omits.each do |i|
      arguments[i].set nil
    end
    
    if cb=arguments.find do |a|
        a.callback
      end
      closure = b || @closure
      cb.set closure
    end
    
    invoked = []

    pointers = arguments.map do |a|
      ptr = a.for_invoke
      
      if a.value == nil
        # do not wrap nil, pass it as is !
        incoked << nil
      else
        if a.direction != :in
          invoked << ptr.addr
        else
          invoked << ptr
        end
      end
      ptr
    end

    # call the function
    r = CFunc::libcall2(get_return_type,@where,@name.to_s,*invoked)

    len = 1
    if ary = @return_type.array
      len = ary.fixed_size
    end

    # get the proper ruby value of return
    result = (@return_type.type == :void ? nil : @return_type.to_ruby(r,len))

    arguments.each do |a|
      a[:value] = nil
    end

    if @results == [-1]

      # default is to return array of out pointers and return value
      ra = get_args_to_return().map do |a|
        ptr = pointers[a.index] if @name == "g_spawn_command_line_sync"

        len = 1
        
        if ary = a.array
          
          len = ary.fixed_size
        end
        
        a.to_ruby(ptr,len)
      end 
           
      r=[result].push(*ra)

      case r.length
      when 0
        return nil
      when 1
        return r[0]
      else
        return r
      end
    else
      # specify what to return.
      # when returning out pointer of array, find the length from another out pointer 
      qq = []
      ala = @results.find_all do |q| q.is_a? Array end
      
      n = ala.map do |a|
        arguments[a[0]]
      end
      
      arguments.find_all do |a|
        !n.index(a)
      end.each do |a|
        i=arguments.index(a)
        qq[i] = a.to_ruby(pointers[i]) if @results.index(i)
      end
      
      ala.each do |q|
        qq[q[0]] = arguments[q[0]].to_ruby pointers[q[0]],qq[q[1]]
      end
      
      a = []
      @results.each do |i|
        if i == -1
          a << result
        else
          if i.is_a? Array
            a << qq[i[0]]
          else
            a << qq[i]
          end
        end
      end
      a
      case a.length
      when 0
        return nil
      when 1
        return a[0]
      else
        return a
      end      
    end
  end
end

class Function
  def pp
    t = @return_type.type
    case t
    when :array
      t = @return_type.array.type
    when :object
      t = @return_type.object
    when :struct
      t = @return_type.struct
    end  
    puts "#{ljust(@where)} #{ljust(@name)}"
    puts "#{ljust(t)} #{@return_type.array ? "1" : "0"}"
  
   arguments.each do |a|
    t = a.type
    case a.type
    when :array
      t = a.array.type
    when :object
      t = a.object
    when :struct
      t = a.struct
    end
    puts "#{ljust(t)} #{a.array ? "1" : "0"}  #{a.direction} #{a.omit? ? "1" : "0"} #{a.callback ? "1" : "0"} #{}"
   end
  end
end

def ljust s
  s=s.to_s
  max = 35
  take = nil
  add = max-s.length
  return s if add == 0
  if add < 0
    return s[0..s.length-1-add]
  else
    return s+(" "*add)
  end
end


def add_function where,name,at,rt,ret=[-1]
i=0
  args = at.map do |a|
  
    arg=Argument.new
    arg[:index] = i
    i=i+1
    direction,array,type,error,callback,data,allow_null = :in,false,:pointer,false,false,false ,false 
    
    while a.is_a?(Hash)
      case a.keys[0]
      when :out
        direction = :out
      when :inout
        direction = :inout
      when :allow_null
        allow_null = true
      when :callback
        callback = a[a.keys[0]]
      else
      end
      
      a = a[a.keys[0]]
    end
    
    if a.is_a? Array
      array = ArrayStruct.new
      arg[:array][:type] = a[0]
      a = :array
    end
    

    type = a
    arg[:type] = type    
    arg[:direction] = direction
    arg[:allow_null] = allow_null
    arg[:callback] = callback
    arg
  end
  
  interface = false
  array = false
  object = false
  
  rett = Return.new
  
  while rt.is_a? Hash
    case rt.keys[0]
    when :struct
      rett[:struct] = rt[rt.keys[0]]
      rett[:type] = :struct
      rt = nil
    when :object
      rett[:object] = rt[rt.keys[0]]
      rett[:type] = :object
      rt = nil
    end
    rt = rt[rt.keys[0]]
  end
  
  if rt.is_a? Array
    ret[:type] = :array
    ret[:array] = ArrayStruct.new
    ret[:array][:type] = rt[0]
  elsif rt
    rett[:type] = rt
  end
  
  Function.new(where,name,args,rett,ret)
end



