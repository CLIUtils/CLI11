project('CLI11', ['cpp'],
  version         : run_command(find_program('scripts/ExtractVersion.py')).stdout().strip(),
  default_options : ['cpp_std=c++11']
)

CLI11_inc = include_directories(['include'])

CLI11_dep = declare_dependency(
  include_directories : CLI11_inc,
  version             : meson.project_version(),
)
