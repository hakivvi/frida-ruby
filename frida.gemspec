# frozen_string_literal: true

require_relative "lib/frida/version"

Gem::Specification.new do |spec|
  spec.name = "frida"
  spec.version = Frida::VERSION
  spec.authors = ["hakivvi"]
  spec.email = ["hakivvi@gmail.com"]

  spec.summary = "Frida Ruby bindings."
  spec.description = "Frida Ruby bindings."
  spec.homepage = "https://github.com/hakivvi/frida-ruby"
  spec.license = "MIT"
  spec.required_ruby_version = ">= 2.7.0"

  spec.metadata["allowed_push_host"] = "https://rubygems.org"

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = spec.homepage
  spec.metadata["changelog_uri"] = spec.homepage
  spec.post_install_message = "frida has been successfully compiled and installed, frida-core is no longer required and can be safely removed."
  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files = Dir.chdir(__dir__) do
    `git ls-files -z`.split("\x0").reject do |f|
      (f == __FILE__) || f.match(%r{\A(?:(?:bin|test|spec|features)/|\.(?:git|travis|circleci)|appveyor)})
    end
  end
  # spec.bindir = "exe"
  # spec.executables = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/c_frida/extconf.rb"]
  # Uncomment to register a new dependency of your gem
  spec.add_dependency "rake-compiler", '~> 1.2.7'

  # For more information and examples about making a new gem, check out our
  # guide at: https://bundler.io/guides/creating_gem.html
end
