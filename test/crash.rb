require "bundler/setup"
require "frida"

mgr = Frida::DeviceManager.new
d = mgr.enumerate_devices[3]
fd = mgr.add_remote_device("127.0.0.1:5656")

# Crashed: Crash(pid=3758, process_name="com.android.music", summary="java.lang.SecurityException: MODE_WORLD_READABLE no longer supported", report=<1330 bytes>, parameters={})
fd.on("process-crashed") {puts "Crashed: #{_1}"}
fd.resume fd.spawn("com.android.music")

sleep 3