# mruby-cfunc

Interface to C functions on mruby. it's based on [libffi](http://sourceware.org/libffi/).

## Build status

[![Build Status](https://secure.travis-ci.org/mobiruby/mruby-cfunc.png)](http://travis-ci.org/mobiruby/mruby-cfunc)

## Install

It's mrbgems.

When you use in your project, please add below to your `build_config.rb`.

```ruby
  conf.gem 'path/to/here' do |g|
    # g.use_pkg_config # use pkg-config for libffi linking
    # g.download_libffi # download and link latest libffi

    # if your libffi is installed in a non standard path
    # g.cc.include_paths << '[...]/include'
    # g.linker.library_paths << '[...]/lib'
  end
```

If you want to run tests, please run below command.

    make test

## Windows

To compile on windows you have to provie dlfcn-win32 libraray.

- install ruby from rubyinstaller.org - get newest with devkit, 64bit
- after installing run as admin:

`ridk install 1 2 3`

- run msys2 console & install library:

`pacman -S mingw-w64-x86_64-dlfcn`

- active ridk ruby build system by adding at the top of you Rakefile line:

`ruby RubyInstaller::Runtime.enable_msys_apps `

- then add to your build_config.rb - to build libffi & link it staically:

```ruby
MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gem mgem: "cfunc" do |gem|
    gem.download_libffi
    gem.linker.flags << "-static"
  end

  # ... rest of build_config.rb
end
```

- now you can build mruby with cfunc mgem! :)

## Todo

- Test!
- Improve error handling
- Support anonymous struct
- Examples
- Documents

## Contributing

Feel free to open tickets or send pull requests with improvements.
Thanks in advance for your help!

## Authors

Original Authors "MobiRuby developers" are [https://github.com/mobiruby/mobiruby-ios/tree/master/AUTHORS](https://github.com/mobiruby/mobiruby-ios/tree/master/AUTHORS)

## License

See Copyright Notice in [cfunc.h](https://github.com/mobiruby/mruby-cfunc/blob/master/include/cfunc.h).

```

```
