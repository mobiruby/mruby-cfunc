require 'open-uri'
MRuby::Gem::Specification.new('mruby-cfunc') do |spec|
  spec.license = 'MIT'
  spec.authors = 'MobiRuby developers'
  
  LIBFFI_VERSION = '3.0.11'
  LIBFFI_URL = "ftp://sourceware.org/pub/libffi/libffi-#{LIBFFI_VERSION}.tar.gz"
  DOWNLOADER = %Q{curl "#{LIBFFI_URL}"}
  # DOWNLOADER = %Q{wget -o- "#{LIBFFI_URL}"}
  TAR = 'tar'

  spec.mruby.linker.libraries << "dl"

  if `uname`.chomp == 'Darwin'
    spec.cc.flags << %w(-pthread)
    spec.linker.flags << %w(-Wl,-allow_stack_execute -all_load)
  else
    spec.cc.flags << %w(-pthread)
    spec.mruby.cc.flags << %w(-pthread)
    spec.mruby.linker.flags << %w(-pthread -Wl,--export-dynamic -Wl,--whole-archive)
    spec.mruby.linker.flags_before_libraries << %w(-Wl,--no-whole-archive)
  end
  # spec.mruby_includes << ["#{LIBFFI_DIR}/lib/libffi-#{LIBFFI_VERSION}/include"]
  # spec.rbfiles = Dir.glob("#{dir}/mrblib/*.rb")
  # spec.objs << ["#{LIBFFI_DIR}/lib/libffi.a"]
  # spec.test_rbfiles = Dir.glob("#{dir}/test/*.rb")
  # spec.test_objs = Dir.glob("#{dir}/test/*.{c,cpp,m,asm,S}").map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X.o") }
  spec.test_preload = "#{dir}/test/mobitest.rb"

  build_root = "build/libffi/#{build.name}"
  libffi_dir = "#{build_root}/libffi-#{LIBFFI_VERSION}"
  unless File.directory?(libffi_dir)
    FileUtils.mkdir_p build_root
    puts "Downloading #{LIBFFI_URL}"
    sh "#{DOWNLOADER} | #{TAR} xfz - -C #{filename build_root}"
  end

  libffi_a = "#{libffi_dir}/lib/libffi.a"
  unless File.exists?(libffi_a)
    sh %Q{(cd #{filename libffi_dir} && CC=#{build.cc.command} CFLAGS="#{build.cc.all_flags}" ./configure --prefix=`pwd` && make clean install)}
  end

  libffi_objs = "#{libffi_dir}/objs"
  unless File.directory?(libffi_objs)
    sh %Q{cp #{filename libffi_dir+"/lib/libffi-#{LIBFFI_VERSION}/include"}/* #{dir}/include}
    sh %Q{mkdir -p #{filename libffi_objs} && cd #{filename libffi_objs}; #{build.archiver.command} x #{filename File.expand_path(libffi_a)} }
  end

  spec.objs << Dir.glob("#{libffi_objs}/*.o")

  rubyvm1_rbx = "#{dir}/test/_rubyvm1.rbx"
  rubyvm1_c = "#{build_dir}/test/_rubyvm1.c"
  rubyvm1_o = rubyvm1_c.ext('o')
  spec.test_objs << rubyvm1_o

  file rubyvm1_o => rubyvm1_c
  file rubyvm1_c => rubyvm1_rbx do |t|
    open(rubyvm1_c, 'w') do |f|
      build.mrbc.run f, rubyvm1_rbx, 'mruby_data__rubyvm1'
    end
  end

end
