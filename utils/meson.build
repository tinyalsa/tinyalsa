utils = ['tinyplay', 'tinycap', 'tinymix', 'tinypcminfo']

foreach util : utils
  executable(util, '@0@.c'.format(util),
    include_directories: tinyalsa_includes,
    link_with: tinyalsa,
    install: true)
  install_man('@0@.1'.format(util))
endforeach
