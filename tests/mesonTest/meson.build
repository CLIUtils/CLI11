project('mesonTest', ['c', 'cpp'], default_options: ['cpp_std=c++11'])

cli11_dep = subproject('CLI11').get_variable('CLI11_dep')

mainExe = executable('main', ['main.cpp'], dependencies: [cli11_dep])
