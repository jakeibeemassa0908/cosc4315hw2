#include <mypython/code/ast.hpp>

#include <util/variant.hpp>

namespace MyPython {
namespace Code {
auto eval_expr(Expression const& expr, Stack const& stack)
    -> std::shared_ptr<Term> {
  auto visitor = [&](auto&& expr) { return eval_expr(expr, stack); };
  return mpark::visit(visitor, expr);
}

auto eval_expr(BoolOp const& expr, Stack const& stack)
    -> std::shared_ptr<Term> {
  Term left_term = *eval_expr(*expr.left);
  Term right_term = *eval_expr(*expr.right);

  Term result;
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
  return std::make_shared<Term>(result);
}

auto eval_expr(BinOp const& expr, Stack const& stack) -> std::shared_ptr<Term> {
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

  Term left_eval = *eval_expr(*expr.left, stack);
  Term right_eval = *eval_expr(*expr.right, stack);

  Term result;
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
  return std::make_shared<Term>(result);
}

auto eval_expr(Compare const& expr, Stack const& stack)
    -> std::shared_ptr<Term> {
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

  return std::make_shared<Term>(result);
}

auto eval_expr(Num const& expr, Stack const& stack) -> std::shared_ptr<Term> {
  return std::make_shared<Term>(expr.n);
}

auto eval_expr(Str const& expr, Stack const& stack) -> std::shared_ptr<Term> {
  return std::make_shared<Term>(expr.s);
}

auto eval_expr(Name const& expr, Stack const& stack) -> std::shared_ptr<Term> {
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
    -> std::shared_ptr<Term> {
  Term result;
  switch (expr.value) {
    case Singleton::none:
      result = NoneType();
      break;
    case Singleton::true_value:
      result = [] {
        Bool bool_value;
        bool_value.value = true;
        return bool_value;
      }();
      break;
    case Singleton::false_value:
      result = [] {
        Bool bool_value;
        bool_value.value = false;
        return bool_value;
      }();
      break;
  }
  return std::make_shared<Term>(result);
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
}  // namespace Code
}  // namespace MyPython