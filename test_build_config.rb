#!/usr/bin/env ruby

if __FILE__ == $PROGRAM_NAME
  require 'fileutils'
  FileUtils.mkdir_p 'tmp'
  unless File.exists?('tmp/mruby')
    system 'git clone https://github.com/mruby/mruby.git tmp/mruby'
  end
  puts(%Q[cd tmp/mruby; CONFIG=#{File.expand_path __FILE__} rake #{ARGV.join(' ')}])
  exit system(%Q[cd tmp/mruby; CONFIG=#{File.expand_path __FILE__} rake #{ARGV.join(' ')}])
end

MRuby::Build.new do |conf|
  conf.cc = ENV['CC'] || 'clang'
  conf.ld = ENV['LD'] || 'clang'
  conf.ar = ENV['AR'] || 'ar'
  # conf.cxx = conf.cc
  # conf.objcc = conf.cc
  # conf.asm = conf.cc
  # conf.yacc = 'bison'
  # conf.gperf = 'gperf'
  # conf.cat = 'cat'
  # conf.git = 'git'

  conf.cflags << (ENV['CFLAGS'] || %w(-g -O3 -Wall -Werror-implicit-function-declaration -DMRB_INT64))
  conf.ldflags << (ENV['LDFLAGS'] || %w(-lm))
  # conf.cxxflags = []
  # conf.objccflags = []
  # conf.asmflags = []

  conf.gem '../../'
  # conf.gem 'doc/mrbgems/ruby_extension_example'
  # conf.gem 'doc/mrbgems/c_extension_example'
  # conf.gem 'doc/mrbgems/c_and_ruby_extension_example'
  # conf.gem :git => 'git@github.com:masuidrive/mrbgems-example.git', :branch => 'master'
end

=begin
MRuby::CrossBuild.new('i386') do |conf|
  conf.cc = ENV['CC'] || 'gcc'
  conf.ld = ENV['LD'] || 'gcc'
  conf.ar = ENV['AR'] || 'ar'
  # conf.cxx = 'gcc'
  # conf.objcc = 'gcc'
  # conf.asm = 'gcc'
  # conf.yacc = 'bison'
  # conf.gperf = 'gperf'
  # conf.cat = 'cat'
  # conf.git = 'git'

  if ENV['OS'] == 'Windows_NT' # MinGW
    conf.cflags = %w(-g -O3 -Wall -Werror-implicit-function-declaration -Di386_MARK)
    conf.ldflags = %w(-s -static)
  else
    conf.cflags << %w(-g -O3 -Wall -Werror-implicit-function-declaration -arch i386)
    conf.ldflags << %w(-arch i386)
  end
  # conf.cxxflags << []
  # conf.objccflags << []
  # conf.asmflags << []

  # conf.gem 'doc/mrbgems/ruby_extension_example'
  # conf.gem 'doc/mrbgems/c_extension_example'
  # conf.gem 'doc/mrbgems/c_and_ruby_extension_example'
end
=end
