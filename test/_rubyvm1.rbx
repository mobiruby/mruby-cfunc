$action1_result = nil

def sleep(sec)
    CFunc::call CFunc::Void, "sleep", CFunc::Int(sec)
    sec
end

def reset_action1
  $action1_result = nil
end

def set_action1(v)
  $action1_result = v
end

def get_action1
  $action1_result
end

def class_name(v)
  v.class.to_s
end

def sum(ary)
  ary.inject(0) { | sum, i | sum + i }
end
