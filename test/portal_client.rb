require 'json'
require "bundler/setup"
require "frida"

require 'json'
require 'digest'

class Application
  def initialize(nick)
    token = { "nick" => nick, "secret" => "knock-knock" }
    @device = Frida::DeviceManager.new.add_remote_device("127.0.0.1:27042", token: JSON.dump(token))

    @bus = @device.bus
    @bus.on("message", method(:_on_bus_message))
    @channel = nil
    @prompt = "> "
  end

  def run
    _start
  end

  private

  def _start
    @bus.attach
    _process_input
  end

  def _process_input()
    loop do
      print "\r"
      begin
        text = $stdin.gets.strip
      rescue
        return
      end
      print "\033[1A\033[K"
      $stdout.flush

      if text.empty?
        _print("Processes:", @device.enumerate_processes)
        next
      end

      if text.start_with?("/join ")
        @bus.post(JSON.dump({ "type" => "part", "channel" => @channel })) if @channel
        @channel = text[6..-1]
        @prompt = "#{@channel} > "
        @bus.post(JSON.dump({ "type" => "join", "channel" => @channel }))
        next
      end

      if text.start_with?("/announce ")
        @bus.post(JSON.dump({ "type" => "announce", "text" => text[10..-1] }))
        next
      end

      if @channel
        @bus.post(JSON.dump({ "channel" => @channel, "type" => "say", "text" => text }))
      else
        _print("*** Need to /join a channel first")
      end
    end
  end

  def _on_bus_message(message, _data)
    message = JSON.parse message
    mtype = message["type"]
    case mtype
    when "welcome"
      _print("*** Welcome! Available channels:", message["channels"].inspect)
    when "membership"
      _print("*** Joined", message["channel"])
      _print(
        "- Members:\n\t" +
          message["members"].map(&JSON.method(:parse)).map { |m| "#{m['nick']} (connected from #{m['address']})" }.join("\n\t")
      )
      message["history"].each do |item|
        _print("<#{item['sender']}> #{item['text']}")
      end
    when "join"
      user = JSON.parse(message["user"])
      puts user
      _print("👋 #{user['nick']} (#{user['address']}) joined #{message['channel']}")
    when "part"
      user = message["user"]
      _print("🚪 #{user['nick']} (#{user['address']}) left #{message['channel']}")
    when "chat"
      _print("<#{message['sender']}> #{message['text']}")
    when "announce"
      _print("📣 <#{message['sender']}> #{message['text']}")
    else
      _print("Unhandled message:", message.inspect)
    end
  end

  def _print(*words)
    puts "#{words.join(' ')}"
    print @prompt
    $stdout.flush
  end
end

nick = ARGV[0]
app = Application.new(nick)
app.run
