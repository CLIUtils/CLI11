# Because Meson does not allow outputs to be placed in subfolders, we must have
# meson.build here when generating the single file header so that it is placed
# in the correct location.

single_main_file = files('CLI11.hpp.in')
single_header = custom_target(
  'CLI11.hpp',
  input: [cli11_headers, cli11_impl_headers],
  output: 'CLI11.hpp',
  command : [find_program('../scripts/MakeSingleHeader.py'), '@INPUT@', '--main', single_main_file, '--output', '@OUTPUT@'],
  depend_files: [single_main_file],
)
