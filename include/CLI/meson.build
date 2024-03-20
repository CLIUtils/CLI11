# Because Meson does not allow outputs to be placed in subfolders, we must have
# meson.build here when generating the single file header so that it is placced
# in the correct location.

prog_python = find_program('python')

cli11_headers = files(
  'App.hpp',
  'Argv.hpp',
  'CLI.hpp',
  'Config.hpp',
  'ConfigFwd.hpp',
  'Encoding.hpp',
  'Error.hpp',
  'Formatter.hpp',
  'FormatterFwd.hpp',
  'Macros.hpp',
  'Option.hpp',
  'Split.hpp',
  'StringTools.hpp',
  'TypeTools.hpp',
  'Validators.hpp',
  'Version.hpp',
)

cli11_impl_headers = files(
  'impl/App_inl.hpp',
  'impl/Argv_inl.hpp',
  'impl/Config_inl.hpp',
  'impl/Encoding_inl.hpp',
  'impl/Formatter_inl.hpp',
  'impl/Option_inl.hpp',
  'impl/Split_inl.hpp',
  'impl/StringTools_inl.hpp',
  'impl/Validators_inl.hpp',
)

single_main_file = files('../../CLI11.hpp.in')

single_header = custom_target(
  'CLI11.hpp',
  input: [files('../../scripts/MakeSingleHeader.py'), cli11_headers, cli11_impl_headers],
  output: 'CLI11.hpp',
  command : [prog_python, '@INPUT@', '--main', single_main_file, '--output', '@OUTPUT@'],
  depend_files: [single_main_file],
)
