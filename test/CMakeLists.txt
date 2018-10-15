add_executable (
  test_libmypython
  test_init.cpp
  mypython/ast_test.cpp
)

target_include_directories(test_libmypython PUBLIC ../include)
target_include_directories(test_libmypython PUBLIC ../src)
target_include_directories(test_libmypython PRIVATE .)

target_link_libraries (
  test_libmypython
  libmypython
)
