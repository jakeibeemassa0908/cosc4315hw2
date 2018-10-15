#ifndef COSC4315HW2_SRC_MYPYTHON_AST_HPP_
#define COSC4315HW2_SRC_MYPYTHON_AST_HPP_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <mpark/variant.hpp>

namespace MyPython {
struct Assign;
struct BoolOp;
struct BinOp;
struct Compare;
struct Expr;
struct FunctionDef;
struct If;
struct Module;
struct Name;
struct NameConstant;
struct Num;
struct Print;
struct PyFunction;
struct PyInt;
struct PyBool;
struct PyNoneType;
struct PyStr;
struct Return;
struct Str;

using Expression =
    mpark::variant<BoolOp, BinOp, Compare, Num, Str, NameConstant, Name>;
using Statement = mpark::variant<FunctionDef, Return, Assign, If, Expr, Print>;
using PyObj = mpark::variant<PyNoneType, PyBool, PyInt, PyStr, PyFunction>;
using BindingMap = std::map<std::string, std::shared_ptr<PyObj>>;

enum class BoolOperator { and_op, or_op };
enum class CmpOp { eq, eq_not, lt, lt_eq, gt, gt_eq, is, is_not, in, not_in };

enum class Op {
  add,
  sub,
  mul,
  mat_mult,
  div,
  mod,
  pow,
  lshift,
  rshift,
  bit_or,
  bit_xor,
  bit_and,
  floor_div
};

enum class Singleton { none, true_value, false_value };

struct Metadata {
  int line = 1;
};

struct EarlyReturn {
  std::shared_ptr<PyObj> result = nullptr;
};

struct Assign {
  std::vector<Expression> targets = {};
  std::shared_ptr<Expression> value = nullptr;
  Metadata meta = {};
};

struct BoolOp {
  std::shared_ptr<Expression> left = nullptr;
  BoolOperator op = BoolOperator::and_op;
  std::shared_ptr<Expression> right = nullptr;
};

struct BinOp {
  std::shared_ptr<Expression> left = nullptr;
  Op op = Op::add;
  std::shared_ptr<Expression> right = nullptr;
  Metadata meta = {};
};

struct Compare {
  std::shared_ptr<Expression> left = nullptr;
  std::vector<CmpOp> ops = {};
  std::vector<Expression> comparators = {};
  Metadata meta = {};
};

struct Expr {
  std::shared_ptr<Expression> value = nullptr;
  Metadata meta = {};
};

struct FunctionDef {
  std::string name = "";
  std::vector<std::string> args = {};
  std::vector<Statement> body = {};
};

struct If {
  std::shared_ptr<Expression> test = nullptr;
  std::vector<Statement> body = {};
  std::vector<Statement> or_else = {};
  Metadata meta = {};
};

struct Module {
  std::vector<Statement> body = {};
  Metadata meta = {};
};

struct Name {
  std::string id = "";
  Metadata meta = {};
};

struct NameConstant {
  Singleton value = Singleton::none;
};

struct Num {
  int n = 0;
  Metadata meta = {};
};

struct Print {
  std::vector<Expression> objects = {};
  std::ostream* file = &std::cout;
};

struct PyInt {
  long value = 0;

  PyInt() = default;
  PyInt(long value) : value(value) {}
};

struct PyBool : public PyInt {};

struct PyNoneType {};

struct PyStr {
  std::string value = "";

  PyStr() = default;
  PyStr(std::string const& value) : value(value) {}
  PyStr(char const* cstr) : PyStr(std::string(cstr)) {}
};

struct PyFunction {
  FunctionDef def = {};
};

struct Return {
  std::shared_ptr<Expression> value = nullptr;
};

struct Stack {
  BindingMap globals = {};
  BindingMap locals = {};
  std::vector<std::string> call_stack = {};
};

struct Str {
  std::string s = "";
  Metadata meta = {};
};

void eval_ast(Module const& ast, Stack& stack);

auto eval_expr(Expression const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(BoolOp const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(BinOp const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(Compare const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(Num const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(Str const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(Name const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;
auto eval_expr(NameConstant const& expr, Stack const& stack = {})
    -> std::shared_ptr<PyObj>;

void eval_stmt(Statement const& stmt, Stack& stack);
void eval_stmt(FunctionDef const& stmt, Stack& stack);
void eval_stmt(Return const& stmt, Stack& stack);
void eval_stmt(Assign const& stmt, Stack& stack);
void eval_stmt(If const& stmt, Stack& stack);
void eval_stmt(Expr const& stmt, Stack& stack);
void eval_stmt(Print const& stmt, Stack& stack);

auto add(PyObj const& a, PyObj const& b) -> PyObj;
auto add(PyInt const& a, PyInt const& b) -> PyObj;
auto add(PyStr const& a, PyStr const& b) -> PyObj;

auto cmp(PyObj const& a, PyObj const& b) -> int;
auto cmp(PyStr const& a, PyStr const& b) -> int;
auto cmp(PyInt const& a, PyInt const& b) -> int;

auto div(PyObj const& a, PyObj const& b) -> PyObj;
auto div(PyInt const& a, PyInt const& b) -> PyObj;

auto mul(PyObj const& a, PyObj const& b) -> PyObj;
auto mul(PyStr const& a, PyInt const& b) -> PyObj;
auto mul(PyInt const& a, PyInt const& b) -> PyObj;

auto str(PyObj const& term) -> PyStr;
auto str(PyNoneType const& n) -> PyStr;
auto str(PyBool const& b) -> PyStr;
auto str(PyInt const& i) -> PyStr;
auto str(PyStr const& str) -> PyStr;

auto sub(PyObj const& a, PyObj const& b) -> PyObj;
auto sub(PyInt const& a, PyInt const& b) -> PyObj;

auto truth_value(PyObj const& term) -> bool;
auto truth_value(PyInt const& i) -> bool;
auto truth_value(PyStr const& str) -> bool;
}  // namespace MyPython

#endif
