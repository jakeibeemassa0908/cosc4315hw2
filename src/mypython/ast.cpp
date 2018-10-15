#include <mypython/ast.hpp>

#include <util/variant.hpp>

namespace MyPython {
// Non-matching functions.
namespace {
template <class T1, class T2>
auto cmp(T1 const& a, T2 const& b) -> int {
  throw "Error comparing two types";
}

template <class T>
auto str(T const&) -> PyStr {
  throw "Error converting to str";
}

template <class T>
auto truth_value(T const&) -> bool {
  throw "No truth value exists";
}

template <class T1, class T2>
auto add(T1 const& a, T2 const& b) -> PyObj {
  throw "Cannot add two types";
}

template <class T1, class T2>
auto sub(T1 const& a, T2 const& b) -> PyObj {
  throw "Cannot add sub types";
}

template <class T1, class T2>
auto mul(T1 const& a, T2 const& b) -> PyObj {
  throw "Cannot mul two types";
}

template <class T1, class T2>
auto div(T1 const& a, T2 const& b) -> PyObj {
  throw "Cannot div two types";
}
}  // namespace

auto eval_expr(Expression const& expr, Stack const& stack)
    -> std::shared_ptr<PyObj> {
  auto visitor = [&](auto&& expr) { return eval_expr(expr, stack); };
  return mpark::visit(visitor, expr);
}

auto eval_expr(BoolOp const& expr, Stack const& stack)
    -> std::shared_ptr<PyObj> {
  PyObj left_term = *eval_expr(*expr.left);
  PyObj right_term = *eval_expr(*expr.right);

  PyObj result;
  switch (expr.op) {
    case BoolOperator::and_op:
      result = truth_value(left_term) && truth_value(right_term);
      break;
    case BoolOperator::or_op:
      result = truth_value(left_term) || truth_value(right_term);
      break;
    default:
      throw "Invalid bool operator";
      break;
  }
  return std::make_shared<PyObj>(result);
}

auto eval_expr(BinOp const& expr, Stack const& stack)
    -> std::shared_ptr<PyObj> {
  // enum class Op {
  //   add,
  //   sub,
  //   mult,
  //   mat_mult,
  //   div,
  //   mod,
  //   pow,
  //   lshift,
  //   rshift,
  //   bit_or,
  //   bit_xor,
  //   bit_and,
  //   floor_div
  // };

  PyObj left_eval = *eval_expr(*expr.left, stack);
  PyObj right_eval = *eval_expr(*expr.right, stack);

  PyObj result;
  switch (expr.op) {
    case Op::add:
      result = add(left_eval, right_eval);
      break;
    case Op::sub:
      result = sub(left_eval, right_eval);
      break;
    case Op::mul:
      result = mul(left_eval, right_eval);
      break;
    case Op::div:
      result = div(left_eval, right_eval);
      break;
    default:
      throw "BinOp not yet implemented";
      break;
  }
  return std::make_shared<PyObj>(result);
}

auto eval_expr(Compare const& expr, Stack const& stack)
    -> std::shared_ptr<PyObj> {
  if (expr.ops.size() != expr.comparators.size())
    throw "Not enough ops/comparators";

  auto result = *eval_expr(*expr.left, stack);
  for (int i = 0; i < expr.ops.size(); ++i) {
    auto op = expr.ops[i];
    auto cmp_term = *eval_expr(expr.comparators[i], stack);

    switch (op) {
      case CmpOp::eq:
        result = cmp(result, cmp_term) == 0;
        break;
      case CmpOp::eq_not:
        result = cmp(result, cmp_term) != 0;
        break;
      case CmpOp::lt:
        result = cmp(result, cmp_term) < 0;
        break;
      case CmpOp::lt_eq:
        result = cmp(result, cmp_term) <= 0;
        break;
      case CmpOp::gt:
        result = cmp(result, cmp_term) > 0;
        break;
      case CmpOp::gt_eq:
        result = cmp(result, cmp_term) >= 0;
        break;
      default:
        throw "Compare not yet implemented";
        break;
    }
  }

  return std::make_shared<PyObj>(result);
}

auto eval_expr(Num const& expr, Stack const& stack) -> std::shared_ptr<PyObj> {
  return std::make_shared<PyObj>(expr.n);
}

auto eval_expr(Str const& expr, Stack const& stack) -> std::shared_ptr<PyObj> {
  return std::make_shared<PyObj>(expr.s);
}

auto eval_expr(Name const& expr, Stack const& stack) -> std::shared_ptr<PyObj> {
  // TODO: FIX THIS
  // We should be throwing a Python exception here.

  if (stack.call_stack.empty()) {
    return stack.globals.at(expr.id);
  } else {
    if (stack.locals.count(expr.id) > 0) {
      return stack.locals.at(expr.id);
    } else {
      return stack.globals.at(expr.id);
    }
  }
}

auto eval_expr(NameConstant const& expr, Stack const& stack)
    -> std::shared_ptr<PyObj> {
  PyObj result;
  switch (expr.value) {
    case Singleton::none:
      result = PyNoneType();
      break;
    case Singleton::true_value:
      result = [] {
        PyBool b;
        b.value = true;
        return b;
      }();
      break;
    case Singleton::false_value:
      result = [] {
        PyBool b;
        b.value = false;
        return b;
      }();
      break;
  }
  return std::make_shared<PyObj>(result);
}

void eval_stmt(Statement const& stmt, Stack& stack) {
  auto visitor = [&](auto&& stmt) { return eval_stmt(stmt, stack); };
  return mpark::visit(visitor, stmt);
}

void eval_stmt(Return const& stmt, Stack& stack) {
  EarlyReturn er;
  er.result = eval_expr(*stmt.value, stack);
  throw er;
}

void eval_stmt(Assign const& stmt, Stack& stack) {
  auto result = eval_expr(*stmt.value, stack);

  if (stmt.targets.size() == 1) {
    auto visitor = Util::make_visitor(
        [&](Name const& name) {
          if (stack.call_stack.empty()) {
            stack.globals[name.id] = result;
          } else {
            stack.locals[name.id] = result;
          }
        },
        [](auto other) { throw "Not yet implemented"; });

    mpark::visit(visitor, stmt.targets.front());
  } else {
    throw "Not yet implemented";
  }
}

void eval_stmt(If const& stmt, Stack& stack) {
  auto result = eval_expr(*stmt.test, stack);
  if (truth_value(*result)) {
    for (auto&& body_stmt : stmt.body) {
      eval_stmt(body_stmt, stack);
    }
  } else {
    for (auto&& body_stmt : stmt.or_else) {
      eval_stmt(body_stmt, stack);
    }
  }
}

void eval_stmt(Expr const& stmt, Stack& stack) {
  eval_expr(*stmt.value, stack);
}

void eval_stmt(Print const& stmt, Stack& stack) {
  bool first = true;
  for (auto&& obj : stmt.objects) {
    auto result = *eval_expr(obj);
    if (first) {
      first = false;
      (*stmt.file) << str(result).value;
    } else {
      (*stmt.file) << " " << str(result).value;
    }
  }
  (*stmt.file) << "\n";
}

void eval_ast(Module const& ast, Stack& stack) {
  for (auto&& stmt : ast.body) {
    eval_stmt(stmt, stack);
  }
}

auto add(PyObj const& a, PyObj const& b) -> PyObj {
  auto visitor = [](auto&& a, auto&& b) { return add(a, b); };
  return mpark::visit(visitor, a, b);
}

auto add(PyInt const& a, PyInt const& b) -> PyObj {
  return PyInt(a.value + b.value);
}
auto add(PyStr const& a, PyStr const& b) -> PyObj {
  return PyStr(a.value + b.value);
}

auto cmp(PyObj const& a, PyObj const& b) -> int {
  auto visitor = [](auto&& a, auto&& b) { return cmp(a, b); };
  return mpark::visit(visitor, a, b);
}

auto cmp(PyInt const& a, PyInt const& b) -> int {
  if (a.value < b.value)
    return -1;
  else if (a.value > b.value)
    return 1;
  else
    return 0;
}

auto cmp(PyStr const& a, PyStr const& b) -> int {
  return a.value.compare(b.value);
}

auto div(PyObj const& a, PyObj const& b) -> PyObj {
  auto visitor = [](auto&& a, auto&& b) { return div(a, b); };
  return mpark::visit(visitor, a, b);
}

auto div(PyInt const& a, PyInt const& b) -> PyObj {
  return PyInt(a.value / b.value);
}

auto mul(PyObj const& a, PyObj const& b) -> PyObj {
  auto visitor = [](auto&& a, auto&& b) { return mul(a, b); };
  return mpark::visit(visitor, a, b);
}

auto mul(PyInt const& a, PyInt const& b) -> PyObj {
  return PyInt(a.value * b.value);
}

auto mul(PyStr const& a, PyInt const& b) -> PyObj {
  PyStr result;
  for (int i = 0; i < b.value; ++i) {
    result.value += a.value;
  }
  return result;
}

auto str(PyObj const& term) -> PyStr {
  auto visitor = [](auto&& term) { return str(term); };
  return mpark::visit(visitor, term);
}

auto str(PyInt const& i) -> PyStr { return std::to_string(i.value); }
auto str(PyStr const& str) -> PyStr { return str; }
auto str(PyNoneType const& n) -> PyStr { return "None"; }
auto str(PyBool const& b) -> PyStr { return (b.value != 0) ? "True" : "False"; }

auto sub(PyObj const& a, PyObj const& b) -> PyObj {
  auto visitor = [](auto&& a, auto&& b) { return sub(a, b); };
  return mpark::visit(visitor, a, b);
}

auto sub(PyInt const& a, PyInt const& b) -> PyObj {
  return PyInt(a.value - b.value);
}

auto truth_value(PyObj const& term) -> bool {
  auto visitor = [](auto&& term) { return truth_value(term); };
  return mpark::visit(visitor, term);
}

auto truth_value(PyNoneType const& n) -> bool { return false; }
auto truth_value(PyStr const& str) -> bool { return !str.value.empty(); }
auto truth_value(PyInt const& i) -> bool { return i.value != 0; }
}  // namespace MyPython