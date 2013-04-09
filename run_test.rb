#!/usr/bin/env ruby

if __FILE__ == $PROGRAM_NAME
  require 'fileutils'
  FileUtils.mkdir_p 'tmp'
  unless File.exists?('tmp/mruby')
    system 'git clone https://github.com/mruby/mruby.git tmp/mruby'
  end
  exit system(%Q[cd tmp/mruby; MRUBY_CONFIG=#{File.expand_path __FILE__} ./minirake #{ARGV.join(' ')}])
end

MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gem "#{root}/mrbgems/mruby-print"
  conf.gem "#{root}/mrbgems/mruby-sprintf"
  conf.gem "#{root}/mrbgems/mruby-math"
  conf.gem "#{root}/mrbgems/mruby-time"
  conf.gem "#{root}/mrbgems/mruby-struct"
  conf.gem "#{root}/mrbgems/mruby-enum-ext"
  conf.gem "#{root}/mrbgems/mruby-string-ext"
  conf.gem "#{root}/mrbgems/mruby-numeric-ext"
  conf.gem "#{root}/mrbgems/mruby-array-ext"
  conf.gem "#{root}/mrbgems/mruby-hash-ext"
  conf.gem "#{root}/mrbgems/mruby-random"

  conf.gem "#{root}/mrbgems/mruby-eval"

  conf.gem File.expand_path(File.dirname(__FILE__)) do |g|
    # g.use_pkg_config
    g.download_libffi
  end
end
