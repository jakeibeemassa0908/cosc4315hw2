add_library (
  libmypython
  mypython/ast.cpp
)

target_include_directories(libmypython PUBLIC ../include)
target_include_directories(libmypython PRIVATE .)
