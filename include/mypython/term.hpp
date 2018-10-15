#ifndef COSC4315HW2_SRC_MYPYTHON_TERM_HPP_
#define COSC4315HW2_SRC_MYPYTHON_TERM_HPP_

#include <iostream>

#include <mpark/variant.hpp>

namespace MyPython {
struct NoneType;
struct Int;
struct Bool;
struct Str;

using Term = mpark::variant<NoneType, Int, Bool, Str>;

auto cmp(Term const& a, Term const& b) -> int;
auto str(Term const& term) -> Str;
auto truth_value(Term const& term) -> bool;

auto add(Term const& a, Term const& b) -> Term;
auto sub(Term const& a, Term const& b) -> Term;
auto mul(Term const& a, Term const& b) -> Term;
auto div(Term const& a, Term const& b) -> Term;

struct Str {
  std::string value = "";

  Str() = default;
  Str(std::string const& value) : value(value) {}
};

auto cmp(Str const& a, Str const& b) -> int;
auto str(Str const& str) -> Str;
auto truth_value(Str const& str) -> bool;

auto add(Str const& a, Str const& b) -> Term;
auto mul(Str const& a, Int const& b) -> Term;

struct Int {
  long value = 0;

  Int() = default;
  Int(long value) : value(value) {}
};

auto cmp(Int const& a, Int const& b) -> int;
auto str(Int const& i) -> Str;
auto truth_value(Int const& i) -> bool;

auto add(Int const& a, Int const& b) -> Term;
auto sub(Int const& a, Int const& b) -> Term;
auto mul(Int const& a, Int const& b) -> Term;
auto div(Int const& a, Int const& b) -> Term;

struct Bool : Int {};

auto str(Bool const& b) -> Str;

struct NoneType {};

auto str(NoneType const& n) -> Str;
}  // namespace MyPython

#endif
