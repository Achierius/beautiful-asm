#include "src/parser/ast_builder_visitor.hh"
#include "src/parser/BeautifulAsmBaseVisitor.h"

#include <memory>
#include <iostream>
#include <string>

namespace parser {

  std::any ASTBuilderVisitor::visitProgram(BeautifulAsmParser::ProgramContext *ctx) {
    auto node = ast::ProgramNode();
    for (auto statement : ctx->statement()) {
      node.statements.push_back(std::make_shared<ast::StatementNode>(std::any_cast<ast::StatementNode>(visitStatement(statement))));
    }
    return node;
  }

  std::any ASTBuilderVisitor::visitFunction_definition(BeautifulAsmParser::Function_definitionContext *ctx) {
    auto node = ast::FunctionNode();
    node.id = ast::IdNode{ .id{ctx->ID()->getText()} };
    if (ctx->parameter_list()) {
      // node->params = std::any_cast<std::vector<std::shared_ptr<ast::ParamNode>>>(visitParameter_list(ctx->parameter_list()));
    }
    node.body = std::any_cast<std::vector<std::shared_ptr<ast::InstructionNode>>>(visitInstructions(ctx->instructions()));
    return ast::StatementNode(node);
  }

  std::any ASTBuilderVisitor::visitType_definition(BeautifulAsmParser::Type_definitionContext *ctx) {
    return visitChildren(ctx);
  }

  std::any ASTBuilderVisitor::visitInstructions(BeautifulAsmParser::InstructionsContext *ctx) {
    std::vector<std::shared_ptr<ast::InstructionNode>> instructions;
    for (auto instruction : ctx->instruction()) {
      instructions.push_back(std::any_cast<std::shared_ptr<ast::InstructionNode>>(visitInstruction(instruction)));
    }
    return instructions;
  }

  std::any ASTBuilderVisitor::visitInstruction(BeautifulAsmParser::InstructionContext *ctx) {
    if (ctx->binary_operator_instruction()) {
      return std::make_shared<ast::InstructionNode>(std::any_cast<ast::BinaryNode>(visitChildren(ctx)));
    }
    return nullptr;
  }
  
  // std::any ASTBuilderVisitor::visitNo_arg_instruction(BeautifulAsmParser::No_arg_instructionContext *ctx) {
  // }

  // std::any ASTBuilderVisitor::visitArrow_instruction(BeautifulAsmParser::Arrow_instructionContext *ctx) {
  // }

  // std::any ASTBuilderVisitor::visitPrint_instruction(BeautifulAsmParser::Print_instructionContext *ctx) {
  // }

  std::any ASTBuilderVisitor::visitBinary_operator_instruction(BeautifulAsmParser::Binary_operator_instructionContext *ctx) {
    ast::BinaryNode node;
    node.op = std::any_cast<ast::BinaryOperator>(visitBinary_operator(ctx->binary_operator()));
    node.lhs = std::any_cast<ast::LValueNode>(visitAny_lvalue(ctx->arg1));
    node.arg1 = std::any_cast<ast::ArgNode>(visitAny_argument(ctx->arg2));
    node.arg2 = std::any_cast<ast::ArgNode>(visitAny_argument(ctx->arg3));
    return node;
  }

  // std::any ASTBuilderVisitor::visitMemory_instruction(BeautifulAsmParser::Memory_instructionContext *ctx) {
  // }

  std::any ASTBuilderVisitor::visitBinary_operator(BeautifulAsmParser::Binary_operatorContext *ctx) {
    if (ctx->getText() == "add") return ast::BinaryOperator::kAdd;
    if (ctx->getText() == "mul") return ast::BinaryOperator::kMul;
    return ast::BinaryOperator::kAdd; // unreachable
  }

  std::any ASTBuilderVisitor::visitAny_lvalue(BeautifulAsmParser::Any_lvalueContext *ctx) {
    return ast::LValueNode();
  }

  std::any ASTBuilderVisitor::visitAny_argument(BeautifulAsmParser::Any_argumentContext *ctx) {
    if (ctx->any_rvalue())
      return ast::ArgNode(std::any_cast<ast::RValueNode>(visitAny_rvalue(ctx->any_rvalue())));
    if (ctx->any_number())
      return ast::ArgNode(std::any_cast<ast::ImmediateNode>(visitAny_rvalue(ctx->any_rvalue())));
    return 0;
  }

  std::any ASTBuilderVisitor::visitAny_rvalue(BeautifulAsmParser::Any_rvalueContext *ctx) {
    return ast::RValueNode();
  }

  std::any ASTBuilderVisitor::visitAny_number(BeautifulAsmParser::Any_numberContext *ctx) {
    if (ctx->getText().find('.'))
      return ast::ImmediateNode(std::stod(ctx->getText()));
    return ast::ImmediateNode(std::stol(ctx->getText()));
  }
}