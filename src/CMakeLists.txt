add_library (
  libmypython
  mypython/term.cpp
  mypython/code/ast.cpp
)

target_include_directories(libmypython PUBLIC ../include)
target_include_directories(libmypython PRIVATE .)
