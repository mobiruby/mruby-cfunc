require 'open-uri'
MRuby::Gem::Specification.new('mruby-cfunc') do |spec|
  spec.license = 'MIT'
  spec.authors = 'MobiRuby developers'
  
  LIBFFI_VERSION = '3.0.11'
  LIBFFI_URL = "https://nodeload.github.com/atgreen/libffi/zip/v#{LIBFFI_VERSION}"
  LIBFFI_DIR = "build/libffi-#{LIBFFI_VERSION}"
  DOWNLOADER = %Q{curl "#{LIBFFI_URL}"}
  # DOWNLOADER = %Q{wget -o- "#{LIBFFI_URL}"}
  TAR = 'tar'

  # spec.cflagqs = ''
 
  # spec.mruby_cflags = ''
  # spec.mruby_ldflags = ''
  spec.mruby_libs = "#{LIBFFI_DIR}/lib/libffi.a"
  spec.mruby_includes << ["#{LIBFFI_DIR}/lib/libffi-#{LIBFFI_VERSION}/include"]
 
  # spec.rbfiles = Dir.glob("#{dir}/mrblib/*.rb")
  spec.objs << ["#{LIBFFI_DIR}/lib/libffi.a"]
  # spec.test_rbfiles = Dir.glob("#{dir}/test/*.rb")
  # spec.test_objs = Dir.glob("#{dir}/test/*.{c,cpp,m,asm,S}").map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X.o") }
  # spec.test_preload = 'test/assert.rb'

  file LIBFFI_DIR do |t|
    FileUtils.mkdir_p 'build'
    puts "Downloading #{LIBFFI_URL}"
    sh "#{DOWNLOADER} | #{TAR} xfz - -C build"
  end

  file "#{LIBFFI_DIR}/lib/libffi.a" => LIBFFI_DIR do |t|
    sh %Q{cd #{LIBFFI_DIR} && ./configure --prefix=`pwd` && make clean install CFLAGS="#{build.cflags.join(' ')}"}
  end
end
