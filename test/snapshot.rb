require "bundler/setup"
require "frida"

embed_script = """
const button = {
  color: 'blue',
};

function mutateButton() {
  button.color = 'red';
}
"""

warmup_script = """
mutateButton();
"""

test_script = """
console.log('Button before:', JSON.stringify(button));
mutateButton();
console.log('Button after:', JSON.stringify(button));
"""

runtime = "v8"

mgr = Frida::DeviceManager.new
d = mgr.enumerate_devices[3]
fd = mgr.add_remote_device("127.0.0.1:5656")
session = fd.attach(fd.enumerate_processes[-1].pid)
snapshot = session.snapshot_script(embed_script, warmup_script:warmup_script, runtime:runtime)


def on_message(message, data)
  puts("on_message: #{message}")
end


script = session.create_script(test_script, snapshot:snapshot, runtime:runtime)
script.on("message", method(:on_message))
script.load()
sleep 3