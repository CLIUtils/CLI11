if catch2_dep.version().version_compare('< 3')
    testmain = static_library(
        'catch_main',
        'main.cpp', 'catch.hpp',
        dependencies: catch2_dep,
    )
    testdep = declare_dependency(
        link_with: testmain,
        dependencies: [catch2_dep, cli11_dep]
    )
else
    testdep = declare_dependency(
        dependencies: [cli11_dep, dependency('catch2-with-main')],
        compile_args: '-DCLI11_CATCH3'
    )
endif

link_test_lib = static_library(
    'link_test_1',
    'link_test_1.cpp',
    dependencies: cli11_dep,
    build_by_default: false,
)

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
    'HelpersTest': {},
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
    # multi-only
    'TimerTest': {},
    # link_test
    'link_test_2': {'link_with': link_test_lib},
}

fs = import('fs')
app_cfgdata = configuration_data()
app_cflags = []
app_tgts = []
foreach app: ['ensure_utf8', 'ensure_utf8_twice']
    app_tgt = executable(
        app, 'applications'/app + '.cpp',
        dependencies: cli11_dep,
        build_by_default: false,
    )
    app_cfgdata.set_quoted(app, fs.relative_to(app_tgt.full_path(), meson.current_build_dir()))
    app_cflags += '-DCLI11_@0@_EXE=@1@'.format(app.to_upper(), app_cfgdata.get(app))
    app_tgts += app_tgt
endforeach

foreach name, kwargs: testnames
    test(name, executable(name, name + '.cpp',
        cpp_args: app_cflags + kwargs.get('cpp_args', []),
        build_by_default: false,
        dependencies: [testdep] + kwargs.get('dependencies', []),
        link_with: kwargs.get('link_with', [])
    ), depends: app_tgts, workdir: meson.current_build_dir())
endforeach
