#include <iostream>
#include <sstream>
#include <string>

#include <mypython/ast.hpp>
#include "catch.hpp"

TEST_CASE("Evaluates bool operators properly", "[eval_expr]") {
  MyPython::Str true_str;
  true_str.s = "truthy";
  MyPython::Str false_str;
  false_str.s = "";

  MyPython::BoolOp bool_op;
  bool_op.left = std::make_shared<MyPython::Expression>(true_str);
  bool_op.right = std::make_shared<MyPython::Expression>(false_str);

  MyPython::Stack stack;

  SECTION("Evalutes and properly") {
    bool_op.op = MyPython::BoolOperator::and_op;
    auto observed = *eval_expr(bool_op, stack);
    REQUIRE(MyPython::cmp(observed, false) == 0);
  }

  SECTION("Evalutes or properly") {
    bool_op.op = MyPython::BoolOperator::or_op;
    auto observed = *eval_expr(bool_op, stack);
    REQUIRE(MyPython::cmp(observed, true) == 0);
  }
}

TEST_CASE("Evaluates binary operators properly", "[eval_expr]") {
  MyPython::Stack stack;
  MyPython::Num num;
  num.n = 100;

  MyPython::BinOp bin_op;
  bin_op.left = std::make_shared<MyPython::Expression>(num);
  bin_op.right = std::make_shared<MyPython::Expression>(num);

  SECTION("Evaluates + properly") {
    bin_op.op = MyPython::Op::add;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 200) == 0);
  }

  SECTION("Evaluates - properly") {
    bin_op.op = MyPython::Op::sub;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 0) == 0);
  }

  SECTION("Evaluates * properly") {
    bin_op.op = MyPython::Op::mul;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 10000) == 0);
  }

  SECTION("Evaluates / properly") {
    bin_op.op = MyPython::Op::div;
    auto observed = *eval_expr(bin_op, stack);
    REQUIRE(MyPython::cmp(observed, 1) == 0);
  }
}

TEST_CASE("Evaluates comparisons properly", "[eval_expr]") {
  MyPython::Stack stack;
  MyPython::Num a;
  a.n = 10;
  MyPython::Num b;
  b.n = 20;

  MyPython::Compare cmp;
  cmp.left = std::make_shared<MyPython::Expression>(a);
  cmp.comparators = {b};

  SECTION("Compares == properly") {
    cmp.ops = {MyPython::CmpOp::eq};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == false);
  }

  SECTION("Compares != properly") {
    cmp.ops = {MyPython::CmpOp::eq_not};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == true);
  }

  SECTION("Compares < properly") {
    cmp.ops = {MyPython::CmpOp::lt};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == true);
  }

  SECTION("Compares <= properly") {
    cmp.ops = {MyPython::CmpOp::lt_eq};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == true);
  }

  SECTION("Compares > properly") {
    cmp.ops = {MyPython::CmpOp::gt};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == false);
  }

  SECTION("Compares >= properly") {
    cmp.ops = {MyPython::CmpOp::gt_eq};
    bool observed = MyPython::truth_value(*eval_expr(cmp, stack));
    REQUIRE(observed == false);
  }
}

TEST_CASE("Evalutes named constants", "[eval_expr]") {
  MyPython::NameConstant nc;
  MyPython::Stack stack;

  SECTION("Evaluates None literal") {
    nc.value = MyPython::Singleton::none;
    auto result = *eval_expr(nc, stack);
    REQUIRE(MyPython::str(result).value == "None");
  }

  SECTION("Evaluates True literal") {
    nc.value = MyPython::Singleton::true_value;
    auto result = *eval_expr(nc, stack);
    REQUIRE(MyPython::str(result).value == "True");
  }

  SECTION("Evaluates False literal") {
    nc.value = MyPython::Singleton::false_value;
    auto result = *eval_expr(nc, stack);
    REQUIRE(MyPython::str(result).value == "False");
  }
}

TEST_CASE("Evalutes string literals", "[eval_expr]") {
  MyPython::Str str;
  str.s = "foobar";

  MyPython::Stack stack;

  auto result = *eval_expr(str, stack);
  REQUIRE(MyPython::cmp(result, "foobar") == 0);
}

TEST_CASE("Evalutes num literals", "[eval_expr]") {
  MyPython::Num num_ast;
  num_ast.n = 1337;

  MyPython::Stack stack;

  auto result = *eval_expr(num_ast, stack);
  REQUIRE(MyPython::cmp(result, 1337) == 0);
}

TEST_CASE("Can define functions", "[eval_stmt]") {
  MyPython::Num num;
  num.n = 100;

  MyPython::Return ret;
  ret.value = std::make_shared<MyPython::Expression>(num);

  MyPython::FunctionDef def;
  def.name = "foo";
  def.body = {ret};

  MyPython::Stack stack;

  SECTION("Can define 0 arity function") {
    def.args = {};
    MyPython::eval_stmt(def, stack);
    REQUIRE(stack.globals.count("foo") == 1);
  }

  SECTION("Can define 3 arity function") {
    def.args = {"x", "y", "z"};
    MyPython::eval_stmt(def, stack);
    REQUIRE(stack.globals.count("foo") == 1);
  }
}

TEST_CASE("Evalutes return statements", "[eval_stmt]") {
  MyPython::NameConstant nc;
  nc.value = MyPython::Singleton::true_value;

  MyPython::Return return_stmt;
  return_stmt.value = std::make_shared<MyPython::Expression>(nc);

  MyPython::Stack stack;

  REQUIRE_THROWS_AS([&] { MyPython::eval_stmt(return_stmt, stack); }(),
                    MyPython::EarlyReturn);
}

TEST_CASE("Accepts Statement variant", "[eval_stmt]") {
  MyPython::Stack stack;
  MyPython::Statement stmt = [] {
    MyPython::Name name;
    name.id = "foo";

    MyPython::Num num;
    num.n = 500;

    MyPython::Assign assign;
    assign.targets.push_back(name);
    assign.value = std::make_shared<MyPython::Expression>(num);
    return assign;
  }();

  MyPython::eval_stmt(stmt, stack);

  REQUIRE(stack.globals.count("foo") > 0);
}

TEST_CASE("Branches if statements", "[eval_stmt]") {
  MyPython::Name test_name;
  test_name.id = "foo";

  MyPython::Num body_num;
  body_num.n = 5;

  MyPython::Num or_else_num;
  or_else_num.n = 0;

  MyPython::Assign body;
  body.targets = {test_name};
  body.value = std::make_shared<MyPython::Expression>(body_num);

  MyPython::Assign or_else;
  or_else.targets = {test_name};
  or_else.value = std::make_shared<MyPython::Expression>(or_else_num);

  MyPython::If if_stmt;
  if_stmt.test = std::make_shared<MyPython::Expression>(body_num);
  if_stmt.body = {body};
  if_stmt.or_else = {or_else};

  MyPython::Stack stack;
  MyPython::eval_stmt(if_stmt, stack);

  REQUIRE(MyPython::cmp(*stack.globals.at("foo"), 5) == 0);
}

TEST_CASE("Assigns values", "[eval_stmt]") {
  MyPython::Name test_name;
  test_name.id = "foo";

  MyPython::Stack stack;

  SECTION("Assigns a value to a new binding") {
    MyPython::Num num;
    num.n = 5;

    MyPython::Assign assign;
    assign.targets = {test_name};
    assign.value = std::make_shared<MyPython::Expression>(num);

    MyPython::eval_stmt(assign, stack);
    REQUIRE(MyPython::cmp(*stack.globals.at("foo"), 5) == 0);
  }

  SECTION("Overwrites a value to an existing binding") {
    MyPython::Num num;
    num.n = 30000;

    MyPython::Assign assign;
    assign.targets = {test_name};
    assign.value = std::make_shared<MyPython::Expression>(num);

    MyPython::eval_stmt(assign, stack);
    REQUIRE(MyPython::cmp(*stack.globals.at("foo"), 30000) == 0);
  }
}

TEST_CASE("Can evaluate expressions", "[eval_stmt]") {
  MyPython::Num num;
  num.n = 5;

  MyPython::Expr expr_stmt;
  expr_stmt.value = std::make_shared<MyPython::Expression>(num);

  MyPython::Stack stack;
  eval_stmt(expr_stmt, stack);
  REQUIRE(stack.globals.empty());
  REQUIRE(stack.locals.empty());
}

TEST_CASE("Can print expressions", "[eval_stmt]") {
  MyPython::Num num;
  num.n = 100;

  std::stringstream out;

  MyPython::Print print;
  print.file = &out;

  MyPython::Stack stack;

  SECTION("Can print one object") {
    print.objects = {num};
    eval_stmt(print, stack);
    REQUIRE(out.str() == "100\n");
  }

  SECTION("Can print multiple objects") {
    print.objects = {num, num, num};
    eval_stmt(print, stack);
    REQUIRE(out.str() == "100 100 100\n");
  }
}
