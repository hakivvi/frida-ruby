# frozen_string_literal: true
require "mkmf"

unless ENV["FRIDA_CORE_DEVKIT"] && File.directory?(ENV["FRIDA_CORE_DEVKIT"])
  abort "please download frida-core devkit, and set its path in the env variable FRIDA_CORE_DEVKIT"
end

# todo: use the native #have_header and #have_library
unless File.file?("#{ENV["FRIDA_CORE_DEVKIT"]}/frida-core.h") && File.file?("#{ENV["FRIDA_CORE_DEVKIT"]}/libfrida-core.a")
  abort "could not find frida-core.h or libfrida-core.a in the configured env variable FRIDA_CORE_DEVKIT"
end

if RUBY_PLATFORM =~ /darwin/ && RUBY_PLATFORM =~ /x86_64/
  target = "x86_64-apple-macos10.9"
  $CFLAGS << " -I#{ENV["FRIDA_CORE_DEVKIT"]} -I#{File.dirname(__FILE__)}/inc"
  $CFLAGS << " -target #{target} -w"
  $libs << " -L. -L#{ENV["FRIDA_CORE_DEVKIT"]} -lfrida-core -lbsm -ldl -lm -lresolv"
  $LDFLAGS << " -Wl,-framework,Foundation,-framework,AppKit,-dead_strip"
elsif RUBY_PLATFORM =~ /linux/
  $CFLAGS = "-g3 -ffunction-sections -fdata-sections -I#{ENV["FRIDA_CORE_DEVKIT"]} -I#{File.dirname(__FILE__)}/inc"
  $LDFLAGS = "-Wl,--export-dynamic -static-libgcc -Wl,-z,noexecstack,--gc-sections -L#{ENV["FRIDA_CORE_DEVKIT"]} -lfrida-core -ldl -lrt -lm -lresolv -pthread"
else
  abort "Platform not supported."
end

create_makefile("c_frida")