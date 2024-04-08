# frozen_string_literal: true

require "mkmf"

unless ENV["FRIDA_CORE_DEVKIT"] && File.directory?(ENV["FRIDA_CORE_DEVKIT"])
  abort "please download frida-core devkit, and set its path in the env variable FRIDA_CORE_DEVKIT"
end

# todo: use the native #have_header and #have_library
unless File.file?("#{ENV["FRIDA_CORE_DEVKIT"]}/frida-core.h") && File.file?("#{ENV["FRIDA_CORE_DEVKIT"]}/libfrida-core.a")
  abort "could not find frida-core.h or libfrida-core.a in the configured env variable FRIDA_CORE_DEVKIT"
end

W =
  if false
    # only on DEV mode because it detects lot of unused var/params, in our code and Ruby's.
    "-Wno-error=cast-function-type -Wno-error=unused-parameter -Wno-error=unused-variable -Wno-error=unused-function -Wall -Werror -Wextra"
  else
    ""
  end

$CFLAGS = "#{W} -g3 -ffunction-sections -fdata-sections -I#{ENV["FRIDA_CORE_DEVKIT"]} -I#{File.dirname(__FILE__)}/inc"
$LDFLAGS = "-Wl,--export-dynamic -static-libgcc -Wl,-z,noexecstack,--gc-sections -L#{ENV["FRIDA_CORE_DEVKIT"]} -lfrida-core -ldl -lrt -lm -lresolv -pthread"

create_makefile("c_frida")
