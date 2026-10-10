if catch2_dep.version().version_compare('< 3')
    testmain = static_library(
        'catch_main',
        'main.cpp',
        dependencies: catch2_dep,
    )
    testdep = declare_dependency(
        link_with: testmain,
        dependencies: catch2_dep,
    )
else
    testdep = declare_dependency(
        dependencies: dependency('catch2-with-main'),
        compile_args: '-DCLI11_CATCH3'
    )
endif

link_test_lib = static_library(
    'link_test_1',
    'link_test_1.cpp',
    dependencies: cli11_nonamalgamated_dep,
    build_by_default: false,
)
link_test_exe = executable(
  'link_test_2',
  'link_test_2.cpp',
  link_with: link_test_lib,
  dependencies: [testdep, cli11_nonamalgamated_dep],
  build_by_default: false,
)
test('link_test_2', link_test_exe)

if cxx.get_argument_syntax() == 'msvc'
    nodeprecated = ['/wd4996']
else
    nodeprecated = ['-Wno-deprecated-declarations']
endif

boost_dep = dependency('boost', required: false, disabler: true)
if boost_dep.found()
    boost_dep = declare_dependency(
        dependencies: boost_dep,
        compile_args: '-DCLI11_BOOST_OPTIONAL',
    )
    maybe_boost_dep = boost_dep
else
    maybe_boost_dep = declare_dependency()
endif

testnames = {
    'HelpersTest': {'workdir': meson.project_build_root()},
    'ConfigFileTest': {},
    'OptionTypeTest': {},
    'NumericTypeTest': {},
    'SimpleTest': {},
    'AppTest': {},
    'SetTest': {},
    'TransformTest': {},
    'CreationTest': {},
    'SubcommandTest': {},
    'HelpTest': {},
    'FormatterTest': {},
    'NewParseTest': {},
    'OptionalTest': {'dependencies': maybe_boost_dep},
    'BoostOptionTypeTest': {'dependencies': boost_dep},
    'DeprecatedTest': {'cpp_args': nodeprecated},
    'StringParseTest': {},
    'ComplexTypeTest': {},
    'TrueFalseTest': {},
    'localeTest': {},
    'OptionGroupTest': {},
    'ExtraValidatorsTest': {},
    'EncodingTest': {},
    'WindowsTest': {'dependencies': host_machine.system() == 'windows' ? declare_dependency() : disabler()},
}

fs = import('fs')
app_cfgdata = configuration_data()
app_cflags = []
app_tgts = []
foreach app: ['ensure_utf8', 'ensure_utf8_twice']
    app_tgt = executable(
        app, 'applications'/app + '.cpp',
        dependencies: cli11_nonamalgamated_dep,
        build_by_default: false,
    )
    app_cfgdata.set_quoted(app, fs.relative_to(app_tgt.full_path(), meson.current_build_dir()))
    app_cflags += '-DCLI11_@0@_EXE=@1@'.format(app.to_upper(), app_cfgdata.get(app))
    app_tgts += app_tgt
endforeach

timertest_exe = executable(
  'TimerTest',
  'TimerTest.cpp',
  cpp_args: app_cflags,
  build_by_default: false,
  dependencies: [testdep, cli11_nonamalgamated_dep],
)
test('TimerTest', timertest_exe)

app_cflags_single = []
if get_option('mode') == 'amalgamated'
  app_cflags_single += '-DCLI11_SINGLE_FILE'
endif

foreach name, kwargs: testnames
    test(name, executable(name, name + '.cpp',
        cpp_args: app_cflags + kwargs.get('cpp_args', []) + app_cflags_single,
        build_by_default: false,
        dependencies: [testdep, cli11_dep] + kwargs.get('dependencies', []),
    ), depends: app_tgts, workdir: kwargs.get('workdir', meson.current_build_dir()))
endforeach
