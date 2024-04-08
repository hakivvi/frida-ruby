# frozen_string_literal: true

require_relative "frida/version"
require_relative "c_frida"

module Frida
  include CFrida
end
