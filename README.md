# mruby-cfunc

Interface to C functions on mruby. it's based on [libffi](http://sourceware.org/libffi/).


## Install

It's mrbgems.

If you want to run tests, please run below command.

    make test


## Limitation

* mrb_int should be 64bit int. define MRB_INT64 in mrbconf.h


## Todo

* Test!
* Improve error handling
* Support anonymous struct
* Examples
* Documents


## Contributing

Feel free to open tickets or send pull requests with improvements.
Thanks in advance for your help!


## Authors

Original Authors "MobiRuby developers" are [https://github.com/mobiruby/mobiruby-ios/tree/master/AUTHORS](https://github.com/mobiruby/mobiruby-ios/tree/master/AUTHORS)


## License

See Copyright Notice in [cfunc.h](https://github.com/mobiruby/mruby-cfunc/blob/master/include/cfunc.h).

