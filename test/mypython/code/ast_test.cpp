#include <mypython/code/ast.hpp>
#include "catch.hpp"

#include <iostream>
#include <sstream>
#include <string>

TEST_CASE("Evaluates bool operators properly", "[eval_expr]") {
  namespace Code = MyPython::Code;

  Code::Str true_str;
  true_str.s = "truthy";
  Code::Str false_str;
  false_str.s = "";

  Code::BoolOp bool_op;
  bool_op.left = std::make_shared<Code::Expression>(true_str);
  bool_op.right = std::make_shared<Code::Expression>(false_str);

  Code::Stack stack;

  SECTION("Evalutes and properly") {
    bool_op.op = Code::BoolOperator::and_op;
    auto observed = *eval_expr(bool_op, stack);
    REQUIRE(MyPython::cmp(observed, false) == 0);
  }

  SECTION("Evalutes or properly") {
    bool_op.op = Code::BoolOperator::or_op;
    auto observed = *eval_expr(bool_op, stack);
    REQUIRE(MyPython::cmp(observed, true) == 0);
  }
}

TEST_CASE("Evaluates binary operators properly", "[eval_expr]") {
  namespace Code = MyPython::Code;

  Code::Stack stack;
  Code::Num num;
  num.n = 100;

  Code::BinOp bin_op;
  bin_op.left = std::make_shared<Code::Expression>(num);
  bin_op.right = std::make_shared<Code::Expression>(num);

  SECTION("Evaluates + properly") {
    bin_op.op = Code::Op::add;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 200) == 0);
  }

  SECTION("Evaluates - properly") {
    bin_op.op = Code::Op::sub;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 0) == 0);
  }

  SECTION("Evaluates * properly") {
    bin_op.op = Code::Op::mul;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 10000) == 0);
  }

  SECTION("Evaluates / properly") {
    bin_op.op = Code::Op::div;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 1) == 0);
  }
}

TEST_CASE("Evaluates comparisons properly", "[eval_expr]") {
  namespace Code = MyPython::Code;

  Code::Stack stack;
  Code::Num a;
  a.n = 10;
  Code::Num b;
  b.n = 20;

  Code::Compare cmp;
  cmp.left = std::make_shared<Code::Expression>(a);
  cmp.comparators = {b};

  SECTION("Compares == properly") {
    cmp.ops = {Code::CmpOp::eq};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == false);
  }

  SECTION("Compares != properly") {
    cmp.ops = {Code::CmpOp::eq_not};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == true);
  }

  SECTION("Compares < properly") {
    cmp.ops = {Code::CmpOp::lt};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == true);
  }

  SECTION("Compares <= properly") {
    cmp.ops = {Code::CmpOp::lt_eq};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == true);
  }

  SECTION("Compares > properly") {
    cmp.ops = {Code::CmpOp::gt};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == false);
  }

  SECTION("Compares >= properly") {
    cmp.ops = {Code::CmpOp::gt_eq};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == false);
  }
}

TEST_CASE("Evalutes named constants", "[eval_expr]") {
  namespace Code = MyPython::Code;

  Code::NameConstant nc;
  Code::Stack stack;

  SECTION("Evaluates None literal") {
    nc.value = Code::Singleton::none;
    auto result = *eval_expr(nc, stack);
    REQUIRE(MyPython::str(result).value == "None");
  }

  SECTION("Evaluates True literal") {
    nc.value = Code::Singleton::true_value;
    auto result = *eval_expr(nc, stack);
    REQUIRE(MyPython::str(result).value == "True");
  }

  SECTION("Evaluates False literal") {
    nc.value = Code::Singleton::false_value;
    auto result = *eval_expr(nc, stack);
    REQUIRE(MyPython::str(result).value == "False");
  }
}

TEST_CASE("Evalutes string literals", "[eval_expr]") {
  namespace Code = MyPython::Code;

  Code::Str str;
  str.s = "foobar";

  Code::Stack stack;

  auto result = *eval_expr(str, stack);
  REQUIRE(MyPython::cmp(result, MyPython::Str("foobar")) == 0);
}

TEST_CASE("Evalutes num literals", "[eval_expr]") {
  namespace Code = MyPython::Code;

  Code::Num num_ast;
  num_ast.n = 1337;

  Code::Stack stack;

  auto result = *eval_expr(num_ast, stack);
  REQUIRE(MyPython::cmp(result, MyPython::Int(1337)) == 0);
}

TEST_CASE("Evalutes return statements", "[eval_stmt]") {
  namespace Code = MyPython::Code;

  Code::NameConstant nc;
  nc.value = Code::Singleton::true_value;

  Code::Return return_stmt;
  return_stmt.value = std::make_shared<Code::Expression>(nc);

  Code::Stack stack;

  REQUIRE_THROWS_AS([&] { Code::eval_stmt(return_stmt, stack); }(),
                    Code::EarlyReturn);
}

TEST_CASE("Accepts Statement variant", "[eval_stmt]") {
  namespace Code = MyPython::Code;

  Code::Stack stack;
  Code::Statement stmt = [] {
    Code::Name name;
    name.id = "foo";

    Code::Num num;
    num.n = 500;

    Code::Assign assign;
    assign.targets.push_back(name);
    assign.value = std::make_shared<Code::Expression>(num);
    return assign;
  }();

  Code::eval_stmt(stmt, stack);

  REQUIRE(stack.bindings.count("foo") > 0);
}

TEST_CASE("Branches if statements", "[eval_stmt]") {
  namespace Code = MyPython::Code;

  Code::Name test_name;
  test_name.id = "foo";

  Code::Num body_num;
  body_num.n = 5;

  Code::Num or_else_num;
  or_else_num.n = 0;

  Code::Assign body;
  body.targets = {test_name};
  body.value = std::make_shared<Code::Expression>(body_num);

  Code::Assign or_else;
  or_else.targets = {test_name};
  or_else.value = std::make_shared<Code::Expression>(or_else_num);

  Code::If if_stmt;
  if_stmt.test = std::make_shared<Code::Expression>(body_num);
  if_stmt.body = {body};
  if_stmt.or_else = {or_else};

  Code::Stack stack;
  Code::eval_stmt(if_stmt, stack);

  REQUIRE(MyPython::cmp(*stack.bindings.at("foo"), 5) == 0);
}

TEST_CASE("Assigns values", "[eval_stmt]") {
  namespace Code = MyPython::Code;

  Code::Name test_name;
  test_name.id = "foo";

  Code::Stack stack;

  SECTION("Assigns a value to a new binding") {
    Code::Num num;
    num.n = 5;

    Code::Assign assign;
    assign.targets = {test_name};
    assign.value = std::make_shared<Code::Expression>(num);

    Code::eval_stmt(assign, stack);
    REQUIRE(MyPython::cmp(*stack.bindings.at("foo"), 5) == 0);
  }

  SECTION("Overwrites a value to an existing binding") {
    Code::Num num;
    num.n = 30000;

    Code::Assign assign;
    assign.targets = {test_name};
    assign.value = std::make_shared<Code::Expression>(num);

    Code::eval_stmt(assign, stack);
    REQUIRE(MyPython::cmp(*stack.bindings.at("foo"), 30000) == 0);
  }
}

TEST_CASE("Can evaluate expressions", "[eval_stmt]") {
  namespace Code = MyPython::Code;

  Code::Num num;
  num.n = 5;

  Code::Expr expr_stmt;
  expr_stmt.value = std::make_shared<Code::Expression>(num);

  Code::Stack stack;
  eval_stmt(expr_stmt, stack);
  REQUIRE(stack.bindings.empty());
}

TEST_CASE("Can print expressions", "[eval_stmt]") {
  namespace Code = MyPython::Code;
  Code::Num num;
  num.n = 100;

  std::stringstream out;

  Code::Print print;
  print.data = std::make_shared<Code::Expression>(num);
  print.file = &out;

  Code::Stack stack;

  eval_stmt(print, stack);

  REQUIRE(out.str() == "100\n");
}
