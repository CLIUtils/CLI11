project('CLI11', ['cpp'],
  version         : run_command(find_program('scripts/ExtractVersion.py'), check: true).stdout().strip(),
  license         : 'BSD-3-clause',
  meson_version   : '>= 0.60',
  default_options : ['cpp_std=c++11', 'warning_level=3']
)

cxx = meson.get_compiler('cpp')

use_single_header = get_option('single-file-header')
use_precompiled =   get_option('precompiled')

if use_precompiled and use_single_header
  error('Options "single-file"header" and "precompiled" are mutually exclusive')
endif

subdir('include/CLI')

CLI11_inc = include_directories(['include'])

warnings = ['-Wshadow', '-Wsign-conversion', '-Wswitch-enum']
if cxx.get_id() == 'gcc' and cxx.version().version_compare('>=4.9')
  warnings += '-Weffc++'
endif
if cxx.get_id() == 'clang'
  warnings += [
    '-Wcast-align',
    '-Wimplicit-atomic-properties',
    '-Wmissing-declarations',
    '-Woverlength-strings',
    '-Wstrict-selector-match',
    '-Wundeclared-selector',
  ]
endif
add_project_arguments(cxx.get_supported_arguments(warnings), language: 'cpp')

if use_precompiled
  libcli11 = static_library(
    'CLI11',
    'src/Precompile.cpp',
    include_directories : CLI11_inc,
    cpp_args            : ['-DCLI11_COMPILE'],
  )
else
  libcli11 = []
endif

CLI11_dep = declare_dependency(
  sources             : single_header,
  link_with           : libcli11,
  include_directories : CLI11_inc,
  version             : meson.project_version(),
)

meson.override_dependency('CLI11', CLI11_dep)

if get_option('tests')
  subdir('tests')
endif
