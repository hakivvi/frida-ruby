# Frida

Frida Ruby bindings.

## Installation

First install frida-core from frida releases, for instance [frida-core-devkit-16.1.11-linux-x86_64](https://github.com/frida/frida/releases/download/16.1.11/frida-core-devkit-16.1.11-linux-x86_64.tar.xz):

    $ wget https://github.com/frida/frida/releases/download/16.1.11/frida-core-devkit-16.1.11-linux-x86_64.tar.xz

then extract the library and the header file and set `FRIDA_CORE_DEVKIT` env variable to the directory:

    $ tar -xf frida-core-devkit-16.1.11-linux-x86_64.tar.xz
    $ export FRIDA_CORE_DEVKIT=`pwd`

Install the gem and add to the application's Gemfile by executing:

    $ bundle add frida

If bundler is not being used to manage dependencies, install the gem by executing:

    $ gem install frida

## OS Support

Currently frida-ruby does not work on Windows because frida-core is built with Visual Studio, and Ruby native extensions by default build with MinGW, and these two do not link together, i'm not sure yet if i can get the native gem to build with VS. 

## Usage

the class names, function names and parameters names are same as [frida-python](https://github.com/frida/frida-python) for ease of migration (note: the whole gem is based on frida-python), every method has a documentation on how to call it, for instance:
```C
/*
    call-seq:
        #create_script(source, name:, snapshot:, runtime:) -> Script
*/
static VALUE Session_create_script(int argc, VALUE *argv, VALUE self)
{
}
```

`Session.create_script` takes one positional argument `source` and three optional keyword arguments (keyword arguments are always optional) and returns a `Script` object.  
`test/` directory has some example scripts to try.


## Development

After checking out the repo, run `bin/setup` to install dependencies. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`.

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/hakivvi/frida-ruby. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [code of conduct](https://github.com/[USERNAME]/frida/blob/master/CODE_OF_CONDUCT.md).

things to improve:
* GVL bridge: this is a workaround i made for acquiring the GVL, frida has lot of callbacks and those callbacks fire from its own threads, CRuby does not support setting up non Ruby created threads to call its APIs. check [gvl_bridge.c](ext/c_frida/gvl_bridge.c) for more informations.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).

## Code of Conduct

Everyone interacting in the Frida project's codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/[USERNAME]/frida/blob/master/CODE_OF_CONDUCT.md).
