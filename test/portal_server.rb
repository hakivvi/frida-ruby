require "bundler/setup"
require "frida"

require 'json'
require 'digest'


require 'pathname'
class Application
  def initialize
    cluster_params = Frida::EndpointParameters.new(
      address:"unix:/tmp/testing/cluster",
      certificate:"/tmp/testing/cert.pem",
      auth_callback:method(:_authenticate)
      )
    control_params = Frida::EndpointParameters.new(
       address:"127.0.0.1", port:27042, auth_callback:method(:_authenticate)
    )
    # control_params = nil
    service = Frida::PortalService.new(cluster_params, control_params: control_params)
    @_service = service
    @_device = service.device
    @_peers = {}
    @_nicks = []
    @_channels = {}

    service.on("node-connected", method(:_on_node_connected))
    service.on("node-joined", method(:_on_node_joined))
    service.on("node-left", method(:_on_node_left))
    service.on("node-disconnected", method(:_on_node_disconnected))
    service.on("controller-connected", method(:_on_controller_connected))
    service.on("controller-disconnected", method(:_on_controller_disconnected))
    service.on("authenticated", method(:_on_authenticated))
    service.on("subscribe", method(:_on_subscribe))
    service.on("message", method(:_on_message))
  end

  def run()
    _start
  end

  def _start()
    @_service.start()
    @_device.enable_spawn_gating()
    _process_input
  end

  def _stop()=@_service.stop()

  def _process_input()
    while true do
      print "Enter command: "
      command = gets.strip()
      if command.size == 0
        puts("Processes: %s" % @_device.enumerate_processes())
        next
      end

      if command == "stop"
        _stop
        break
      end
    end
  end

    def _authenticate(raw_token)
        token = JSON.parse(raw_token)
        nick = token["nick"]
        secret = token["secret"]

        unless Digest::SHA1.digest(secret) == Digest::SHA1.digest("knock-knock")
          raise RuntimeError.new("get outta here")
        end
        return {
          "nick": nick,
        }
      end

      def _on_node_connected(connection_id, remote_address)
        puts("on_node_connected() %s %s" % [connection_id, remote_address])
      end

      def _on_node_joined(connection_id, application)
        puts("on_node_joined() %s %s" % [connection_id, application])
        puts("\ttags: %s", @_service.enumerate_tags(connection_id))
      end

      def _on_node_left(connection_id, application)
        puts("on_node_left() %s %s" %[connection_id, application])
      end
      def _on_node_disconnected(connection_id, remote_address)
        puts("on_node_disconnected() %s %s" %[connection_id, remote_address])
      end

      def _on_controller_connected(connection_id, remote_address)
        puts("on_controller_connected() %s %s" %[connection_id, remote_address])
        @_peers[connection_id] = Peer.new(connection_id, remote_address)
      end

      def _on_controller_disconnected(connection_id, remote_address)
        puts("on_controller_disconnected() %s %s" % [connection_id, remote_address])
        peer = @_peers.pop(connection_id)
        peer.memberships.each {channel.remove_member(_1)}
        if peer.nick
          _release_nick(peer.nick)
        end
      end

      def _on_authenticated(connection_id, session_info)
        puts("on_authenticated() %s %s" %[connection_id, session_info])
          peer = @_peers[connection_id]
          unless peer
            return
          end
          peer.nick = _acquire_nick(session_info["nick"])
      end

      def _on_subscribe(connection_id)
        puts("on_subscribe() %s" % connection_id)
          @_service.post(connection_id, JSON.dump({"type": "welcome", "channels": @_channels.keys()}))
      end

      def _on_message(connection_id, message, data)
        begin
        peer = @_peers[connection_id]

        message = JSON.parse(message)
        puts "on message: #{message}"
        mtype = message["type"]
        if mtype == "join"
          _get_channel(message["channel"]).add_member(peer)
        elsif mtype == "part"
          channel = @_channels[message["channel"]]
          unless channel
            return
          end
          channel.remove_member(peer)
        elsif mtype == "say"
          channel = @_channels[message["channel"]]
          unless channel
            return
          end
          channel.post(message["text"], peer)
        elsif mtype == "announce"
          @_service.broadcast(JSON.dump({"type": "announce", "sender": peer.nick, "text": message["text"]}))
        else
          puts("Unhandled message: %s" % message)
        end
        rescue Exception => e
          puts "Exc > #{e}"
        end
      end

      def _acquire_nick(requested)
          candidate = requested
          serial = 2
          while @_nicks[condidata] do
            candidate = requested + serial.to_s
            serial += 1
          end
          nick = candidate
          @_nicks.add(nick)
          nick
      end

      def _release_nick(nick)
        @_nicks.delete(nick)
      end

      def _get_channel(name)
            channel = @_channels[name]
            unless channel
              channel = Channel.new(name, @_service)
            end
            @_channels[name] = channel
            channel
      end
  end


class Peer
  attr_accessor :nick, :memberships, :remote_address, :connection_id
  def initialize(connection_id, remote_address)
    @nick = nil
    @connection_id = connection_id
    @remote_address = remote_address
    @memberships = []
  end

  def to_json()
    return JSON.dump({"nick": @nick, "address": @remote_address})
  end
end

class Channel
  def initialize(name, service)
    @name = name
    @members = []
    @history = []
    @_service = service
  end

  def add_member(peer)
    if peer.memberships.include? self
      return
    end
    peer.memberships.push(self)
    @members.push(peer)

    @_service.narrowcast(@name, JSON.dump({"type": "join", "channel": @name, "user": peer.to_json()}))
    @_service.tag(peer.connection_id, @name)

    @_service.post(
      peer.connection_id,
      JSON.dump(
      {
        "type": "membership",
        "channel": @name,
        "members": @members.map{_1.to_json},
        "history": @history,
      })
      )
    end

      def remove_member(peer)
        unless peer.memberships.include? self
          return
        end

        peer.memberships.delete(self)
        @members.delette(peer)

        @_service.untag(peer.connection_id, @name)
        @_service.narrowcast(@name, JSON.dump({"type": "part", "channel": @name, "user": peer.to_json()}))
      end

      def post(text, peer)
        unless peer.memberships.include? self
            return
        end
        item = {"type": "chat", "sender": peer.nick, "text": text}
        @_service.narrowcast(@name, JSON.dump(item))
        history = self.history
        history.append(item)
        if history.size == 20
          history.pop(0)
        end
      end
end


Application.new.run()
