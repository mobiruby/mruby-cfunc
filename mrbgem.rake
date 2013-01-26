require 'open-uri'

MRuby::Gem::Specification.new('mruby-cfunc') do |spec|
  spec.license = 'MIT'
  spec.authors = 'MobiRuby developers'

  def spec.use_pkg_config(pkg_config='pkg-config')
    self.linker.flags << `"#{pkg_config}" libffi --libs-only-L --libs-only-other`.chomp
    [self.cc, self.cxx, self.objc, self.mruby.cc, self.mruby.cxx, self.mruby.objc].each do |cc|
      cc.include_paths << `"#{pkg_config}" libffi --cflags`.chomp
    end
  end

  def spec.download_libffi(libffi_version = '3.0.11', tar = 'tar')
    libffi_url = "ftp://sourceware.org/pub/libffi/libffi-#{libffi_version}.tar.gz"
    libffi_build_root = "build/libffi/#{build.name}"
    libffi_dir = "#{libffi_build_root}/libffi-#{libffi_version}"
    libffi_a = "#{libffi_dir}/lib/libffi.a"

    unless File.exists?(libffi_a)
      puts "Downloading #{libffi_url}"
      open(libffi_url, 'r') do |ftp|
        libffi_tar = ftp.read
        puts "Extracting"
        FileUtils.mkdir_p libffi_build_root
        IO.popen("#{tar} xfz - -C #{filename libffi_build_root}", 'w') do |f|
          f.write libffi_tar
        end
        puts "Done"
      end
      sh %Q{(cd #{filename libffi_dir} && CC=#{build.cc.command} CFLAGS="#{build.cc.all_flags.gsub('\\','\\\\').gsub('"', '\\"')}" ./configure --prefix=`pwd` && make clean install)}
    end
    
    self.linker.library_paths << File.dirname(libffi_a)
    [self.cc, self.cxx, self.objc, self.mruby.cc, self.mruby.cxx, self.mruby.objc].each do |cc|
      cc.include_paths << File.dirname(libffi_a)
    end
  end

  spec.mruby.linker.libraries << %w(ffi dl)

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
