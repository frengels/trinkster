tests = [
    'ss/intrusive_list',
    'ss/slot',
]

catch_lib = static_library(
    'catch2',
    'catch2.cpp',
    dependencies: catch2_dep,
)

foreach t : tests
    test_exe = executable(
        t.underscorify(),
        '@0@.cpp'.format(t),
        link_with: catch_lib,
        include_directories: [ trinkster_inc ],
        dependencies: [ catch2_dep ],
    )
    test(t, test_exe)
endforeach