project ('tinyalsa', 'c',
  version : run_command(find_program('scripts/version.sh'), 'print', '-s').stdout().strip(),
  meson_version : '>= 0.48.0')

tinyalsa_includes = include_directories('.', 'include')

cc = meson.get_compiler('c')

# Dependency on libdl
dl_dep = cc.find_library('dl')

tinyalsa = library('tinyalsa',
  'src/mixer.c', 'src/pcm.c', 'src/pcm_hw.c', 'src/pcm_plugin.c', 'src/snd_card_plugin.c', 'src/mixer_hw.c', 'src/mixer_plugin.c',
  include_directories: tinyalsa_includes,
  version: meson.project_version(),
  install: true,
  dependencies: dl_dep)

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
