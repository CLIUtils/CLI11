project('CLI11', ['cpp'],
  version         : run_command(find_program('scripts/ExtractVersion.py'), check: true).stdout().strip(),
  license         : 'BSD-3-clause',
  meson_version   : '>= 1.3',
  default_options : ['cpp_std=c++17,c++14,c++11', 'warning_level=3']
)

cxx = meson.get_compiler('cpp')

buildmode = get_option('mode')

cli11_headers = files(
  'include/CLI/App.hpp',
  'include/CLI/Argv.hpp',
  'include/CLI/CLI.hpp',
  'include/CLI/Config.hpp',
  'include/CLI/ConfigFwd.hpp',
  'include/CLI/Encoding.hpp',
  'include/CLI/Error.hpp',
  'include/CLI/Formatter.hpp',
  'include/CLI/FormatterFwd.hpp',
  'include/CLI/Macros.hpp',
  'include/CLI/Option.hpp',
  'include/CLI/Split.hpp',
  'include/CLI/StringTools.hpp',
  'include/CLI/TypeTools.hpp',
  'include/CLI/Validators.hpp',
  'include/CLI/ExtraValidators.hpp',
  'include/CLI/Version.hpp',
)

cli11_impl_headers = files(
  'include/CLI/impl/App_inl.hpp',
  'include/CLI/impl/Argv_inl.hpp',
  'include/CLI/impl/Config_inl.hpp',
  'include/CLI/impl/Encoding_inl.hpp',
  'include/CLI/impl/Formatter_inl.hpp',
  'include/CLI/impl/Option_inl.hpp',
  'include/CLI/impl/Split_inl.hpp',
  'include/CLI/impl/StringTools_inl.hpp',
  'include/CLI/impl/Validators_inl.hpp',
  'include/CLI/impl/ExtraValidators_inl.hpp',
)

cli11_inc = include_directories('include')

if cxx.get_argument_syntax() == 'gcc'
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
endif

if buildmode == 'amalgamated'
  subdir('single-include')

  cli11_dep = declare_dependency(
    sources: single_header,
    include_directories : include_directories(meson.current_build_dir() / 'single-include'),
  )
elif buildmode == 'headeronly'
  install_headers(cli11_headers, subdir: 'CLI')

  cli11_dep = declare_dependency(include_directories : cli11_inc)
elif buildmode == 'precompiled'
  cli11_cflags = ['-DCLI11_COMPILE', '-DCLI11_ENABLE_EXTRA_VALIDATORS=1']

  libcli11 = library(
    'CLI11',
    'src/Precompile.cpp',
    include_directories : cli11_inc,
    cpp_args            : cli11_cflags,
    install             : true,
  )

  pkg = import('pkgconfig', required: false)
  if pkg.found()
    pkg.generate(
      libcli11,
      description  : 'CLI11 is a command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface.',
      extra_cflags : cli11_cflags,
      url          : 'https://github.com/CLIUtils/CLI11',
    )
  endif

  install_headers(cli11_headers, subdir: 'CLI')

  cli11_dep = declare_dependency(
    link_with           : libcli11,
    include_directories : cli11_inc,
  )
endif

meson.override_dependency('CLI11', cli11_dep)

tests = get_option('tests')
tests = tests.disable_auto_if(meson.is_subproject())
catch2_dep = dependency('catch2', required: tests)
subdir('tests', if_found: catch2_dep)
