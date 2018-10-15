#ifndef COSC4315HW2_SRC_MYPYTHON_CODE_HPP_
#define COSC4315HW2_SRC_MYPYTHON_CODE_HPP_

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <mpark/variant.hpp>
#include <mypython/term.hpp>

namespace MyPython {
namespace Code {
struct BoolOp;
struct BinOp;
struct Compare;
struct Num;
struct Str;
struct NameConstant;
struct Name;

using Expression =
    mpark::variant<BoolOp, BinOp, Compare, Num, Str, NameConstant, Name>;

struct Return;
struct Assign;
struct If;
struct Expr;
struct Print;

using Statement = mpark::variant<Return, Assign, If, Expr, Print>;

struct Module;

using AST = mpark::variant<Module, Expression, Statement>;

using BindingMap = std::map<std::string, std::shared_ptr<Term>>;

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

struct BoolOp {
  std::shared_ptr<Expression> left = nullptr;
  BoolOperator op = BoolOperator::and_op;
  std::shared_ptr<Expression> right = nullptr;
};

struct Assign {
  std::vector<Expression> targets = {};
  std::shared_ptr<Expression> value = nullptr;
  Metadata meta = {};
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

struct EarlyReturn {
  std::shared_ptr<Term> result = nullptr;
};

struct Expr {
  std::shared_ptr<Expression> value = nullptr;
  Metadata meta = {};
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
  std::shared_ptr<Expression> data = nullptr;
  std::ostream* file = &std::cout;
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

auto eval_expr(Expression const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(BoolOp const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(BinOp const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(Compare const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(Num const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(Str const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(Name const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;
auto eval_expr(NameConstant const& expr, Stack const& stack = {})
    -> std::shared_ptr<Term>;

void eval_stmt(Statement const& stmt, Stack& stack);
void eval_stmt(Return const& stmt, Stack& stack);
void eval_stmt(Assign const& stmt, Stack& stack);
void eval_stmt(If const& stmt, Stack& stack);
void eval_stmt(Expr const& stmt, Stack& stack);
void eval_stmt(Print const& stmt, Stack& stack);

void eval_ast(Module const& ast, Stack& stack);
}  // namespace Code
}  // namespace MyPython

#endif
