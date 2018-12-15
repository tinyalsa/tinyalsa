examples = ['pcm-readi', 'pcm-writei']

foreach e : examples
  executable(e, '@0@.c'.format(e),
    include_directories: tinyalsa_includes,
    link_with: tinyalsa,
    install: false)
endforeach
