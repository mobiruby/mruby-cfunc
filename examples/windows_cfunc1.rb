module Win32
  module User32
    # win32 api compatibile version
    # each libcall format is like this:
    # - return tyupe
    # - libraray
    # - function to call
    # - variable number of arguments to function
    # - last param CFunc::Int.ew(X) - where X is number of arguments you pass above
    DLL = CFunc.libcall(CFunc::Pointer, "kernel32.dll", "LoadLibraryA", "user32.dll", CFunc::Int.new(1))
    MessageBeep = begin
        ptr = CFunc.libcall(CFunc::Pointer, "kernel32.dll", "GetProcAddress", DLL, "MessageBeep", CFunc::Int.new(2))
        CFunc::FunctionPointer.new(ptr).tap do |func|
          func.arguments_type = [CFunc::UInt32]
          func.result_type = CFunc::UInt32
        end
      end

    MB_ICONASTERISK = 0x40

    # shortcut version - it's a little bit slower, because it will try to open user32.dll again
    MessageBox = proc do |text, caption, type|
      CFunc.libcall(
        CFunc::UInt32, # retrun type
        "user32.dll", # dll
        "MessageBoxA", # function
        nil, text, caption, CFunc::Int.new(type), # args
        CFunc::Int.new(4) # number of args
      )
    end

    MB_OKCANCEL = 0x1
  end
end

# then you can call MessageBeep like that
Win32::User32::MessageBeep.call(Win32::User32::MB_ICONASTERISK)

# if you want get return value:
ret = Win32::User32::MessageBeep.call(Win32::User32::MB_ICONASTERISK).value

# and calling shortcut version
Win32::User32::MessageBox.("MRuby MessageBox", "MRuby MessageBox", Win32::User32::MB_OKCANCEL)
