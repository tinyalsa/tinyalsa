project ('tinyalsa', 'c', version : '1.1.1', meson_version : '>= 0.48.0')

tinyalsa_includes = include_directories('.', 'include')

tinyalsa = library('tinyalsa',
  'src/mixer.c', 'src/pcm.c',
  include_directories: tinyalsa_includes,
  install: true)

# For use as a Meson subproject
tinyalsa_dep = declare_dependency(link_with: tinyalsa,
  include_directories: include_directories('include'))

if not get_option('docs').disabled()
  # subdir('docs') # FIXME
endif

if not get_option('examples').disabled()
  subdir('examples')
endif

subdir('include/tinyalsa')

if not get_option('utils').disabled()
  subdir('utils')
endif

pkg = import('pkgconfig')
pkg.generate(tinyalsa, description: 'TinyALSA Library')
