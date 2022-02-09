catch2 = dependency('catch2')

testmain = static_library(
    'catch_main',
    'main.cpp', 'catch.hpp',
    dependencies: catch2,
)
testdep = declare_dependency(
    link_with: testmain,
    dependencies: [catch2, CLI11_dep]
)

link_test_lib = library(
    'link_test_1',
    'link_test_1.cpp',
    dependencies: CLI11_dep,
)

if cxx.get_id() == 'msvc'
    nodeprecated = ['/wd4996']
else
    nodeprecated = ['-Wno-deprecated-declarations']
endif

boost = dependency('boost', required: false)
if boost.found()
    boost_dep = declare_dependency(
        dependencies: boost,
        compile_args: '-DCLI11_BOOST_OPTIONAL',
    )
else
    boost_dep = declare_dependency()
endif

testnames = [
    ['HelpersTest', {}],
    ['ConfigFileTest', {}],
    ['OptionTypeTest', {}],
    ['SimpleTest', {}],
    ['AppTest', {}],
    ['SetTest', {}],
    ['TransformTest', {}],
    ['CreationTest', {}],
    ['SubcommandTest', {}],
    ['HelpTest', {}],
    ['FormatterTest', {}],
    ['NewParseTest', {}],
    ['OptionalTest', {'dependencies': boost_dep}],
    ['DeprecatedTest', {'cpp_args': nodeprecated}],
    ['StringParseTest', {}],
    ['ComplexTypeTest', {}],
    ['TrueFalseTest', {}],
    ['OptionGroupTest', {}],
    # multi-only
    ['TimerTest', {}],
    # link_test
    ['link_test_2', {'link_with': link_test_lib}],
]

if host_machine.system() == 'windows'
    testnames += [['WindowsTest', {}]]
endif

if boost.found()
    testnames += [['BoostOptionTypeTest', {'dependencies': boost_dep}]]
endif

foreach n: testnames
    name = n[0]
    kwargs = n[1]
    t = executable(name, name + '.cpp',
        cpp_args: kwargs.get('cpp_args', []),
        build_by_default: false,
        dependencies: [testdep] + kwargs.get('dependencies', []),
        link_with: kwargs.get('link_with', [])
    )
    test(name, t)
endforeach
