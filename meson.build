project('CLI11', ['cpp'],
  version         : run_command(find_program('scripts/ExtractVersion.py'), check: true).stdout().strip(),
  default_options : ['cpp_std=c++11', 'warning_level=3']
)

cxx = meson.get_compiler('cpp')

CLI11_inc = include_directories(['include'])

CLI11_dep = declare_dependency(
  include_directories : CLI11_inc,
  version             : meson.project_version(),
)

if get_option('tests')
    warnings = ['-Wshadow', '-Wsign-conversion', '-Wswitch-enum']
    if cxx.get_id() == 'gcc' and cxx.version().version_compare('>=4.9')
        warnings += '-Weffc++'
    endif
    add_project_arguments(cxx.get_supported_arguments(warnings), language: 'cpp')

    subdir('tests')
endif
