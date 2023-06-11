#include "compile.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/SourceMgr.h>

#include <map>

#include "ast.h"
#include "common.h"

typedef union _as {
  int64_t integer;
  double decimal;
} as;

static std::map<std::string, int64_t> gSymbolMap;

llvm::LLVMContext context;
using namespace llvm;

Compiler::Compiler(std::string s) : mfilename(s) {
  pmodule = new Module(mfilename, context);
  pbuilder = new IRBuilder<>(context);
  SMDiagnostic error;
  originalModule = parseIRFile("trt.ll", error, context);
  if (!originalModule) {
    error.print("llvm-ir-example", errs());
    exit(1);
  }
  irnode = StructType::create(context, "IRNode");
  irnode->setBody(pbuilder->getInt8Ty(), pbuilder->getInt64Ty());
  FunctionType* mainFunType = FunctionType::get(pbuilder->getInt32Ty(), false);
  Function* mainFun = Function::Create(mainFunType, Function::ExternalLinkage,
                                       "main", *pmodule);
  BasicBlock* mainEntry = BasicBlock::Create(context, "entry", mainFun);
  pbuilder->SetInsertPoint(mainEntry);
}

int64_t convert_sym(SymbolNode* node, bool create = false) {
  static int64_t gNextValue = 0;
  if (gSymbolMap.find(node->symbol) == gSymbolMap.end()) {
    if (create)
      gSymbolMap[node->symbol] = gNextValue++;
    else
      throw std::runtime_error("Symbol definition doesn't exist : " +
                               node->symbol);
  }

  return gSymbolMap[node->symbol];
}

Value* Compiler::generate_irnode(uint32_t type, int64_t value) {
  AllocaInst* nodeobj = pbuilder->CreateAlloca(irnode, nullptr, "node");
  Value* typeidx[] = {pbuilder->getInt32(0), pbuilder->getInt32(0)};
  Value* fieldtype =
      pbuilder->CreateGEP(irnode, nodeobj, typeidx, "field-type");
  Value* valueidx[] = {pbuilder->getInt32(0), pbuilder->getInt32(1)};
  Value* fieldvalue =
      pbuilder->CreateGEP(irnode, nodeobj, valueidx, "field-value");

  pbuilder->CreateStore(pbuilder->getInt8(type), fieldtype);
  pbuilder->CreateStore(pbuilder->getInt64(value), fieldvalue);
  return nodeobj;
}

Value* Compiler::generate_irnode(ASTNode* node) {
  as xformer;
  Value* nodeobj;
  switch (node->type()) {
    case ASTNodeType::Bool:
      nodeobj =
          generate_irnode((uint32_t)node->type(),
                          (dynamic_cast<BoolNode*>(node)->getBool() ? 1 : 0));
      break;
    case ASTNodeType::Integer:
      nodeobj = generate_irnode((uint32_t)node->type(),
                                dynamic_cast<IntegerNode*>(node)->value);
      break;
    case ASTNodeType::Decimal:
      xformer.decimal = dynamic_cast<DecimalNode*>(node)->value;
      nodeobj = generate_irnode((uint32_t)node->type(), xformer.integer);
      break;
    case ASTNodeType::Symbol: {
      auto symInt = convert_sym(dynamic_cast<SymbolNode*>(node));
      nodeobj = generate_irnode((uint32_t)node->type(), symInt);
      FunctionType* operationType = FunctionType::get(
          PointerType::get(irnode, 0), {PointerType::get(irnode, 0)}, false);
      Function* operation = Function::Create(
          operationType, Function::ExternalLinkage, "_Z8retrieveP7_IRNode");
      return pbuilder->CreateCall(operation, {nodeobj});
    }
    default:
      throw std::runtime_error("Not implemented");
  }
  return nodeobj;
}

Value* Compiler::generate_println(ASTNode* node) {
  Value* operand = generate_code(node);
  FunctionType* operationType = FunctionType::get(
      pbuilder->getVoidTy(), {PointerType::get(irnode, 0)}, false);
  Function* operation = Function::Create(
      operationType, Function::ExternalLinkage, "_Z9printNodeP7_IRNode");
  return pbuilder->CreateCall(operation, {operand});
}

Value* Compiler::generate_arithmetic(char opchar, ListNode* listnode) {
  auto it = std::next(listnode->list.begin());  // skip func name
  auto itend = listnode->list.end();
  std::vector<Value*> operands;
  operands.push_back(pbuilder->getInt8(opchar));
  operands.push_back(pbuilder->getInt32(listnode->list.size() - 1));
  while (it != itend) {
    operands.push_back(generate_code(*it));
    it++;
  }

  FunctionType* operationType =
      FunctionType::get(PointerType::get(irnode, 0),
                        {pbuilder->getInt8Ty(), pbuilder->getInt32Ty()}, true);
  Function* operation = Function::Create(
      operationType, Function::ExternalLinkage, "_Z10arithmeticcjz");
  return pbuilder->CreateCall(operation, operands);
}

Value* Compiler::generate_define(SymbolNode* symnode, ASTNode* valuenode) {
  auto symoperand =
      generate_irnode(symnode->type(), convert_sym(symnode, true));
  auto valueoperand = generate_code(valuenode);
  FunctionType* funType = FunctionType::get(
      PointerType::get(irnode, 0),
      {PointerType::get(irnode, 0), PointerType::get(irnode, 0)}, false);
  Function* fun = Function::Create(funType, Function::ExternalLinkage,
                                   "_Z6defineP7_IRNodeS0_");
  return pbuilder->CreateCall(fun, {symoperand, valueoperand});
}

Value* Compiler::generate_code(ASTNode* node) {
  switch (node->type()) {
    case ASTNodeType::List: {
      auto listnode = dynamic_cast<ListNode*>(node);
      // TODO Currently we assume +. Handle all cases
      if (listnode->list.front()->type() == ASTNodeType::Symbol) {
        auto fun = dynamic_cast<SymbolNode*>(listnode->list.front())->symbol;
        if (fun == "+" || fun == "-" || fun == "*" || fun == "/")
          return generate_arithmetic(fun[0], listnode);
        if (fun == "println") return generate_println(listnode->list.back());
        if (fun == "def") {
          auto symnode =
              dynamic_cast<SymbolNode*>(*std::next(listnode->list.begin()));
          return generate_define(symnode, listnode->list.back());
        }
      }

      throw std::runtime_error("Not implemented");
    }
    case ASTNodeType::Lambda:
    case ASTNodeType::String:
      throw std::runtime_error("Not implemented");
    case ASTNodeType::Symbol:
    case ASTNodeType::Bool:
    case ASTNodeType::Integer:
    case ASTNodeType::Decimal:
      return generate_irnode(node);
  }
}

std::string Compiler::handle_line(std::string line) {
  std::string success = "";
  if (is_comment(line)) return success;
  auto tokens = tokenizer(line);
  auto ast = create_ast(tokens);

  if (ast.size() == 0) return success;
  if (ast.size() != 1)
    throw TodaluException("Contains more than one node at the base");

  mret = generate_code(ast.front());

  return success;
}

Compiler::~Compiler() {
  pbuilder->CreateRet(pbuilder->getInt32(0));
  if (Linker::linkModules(*pmodule, std::move(originalModule))) {
    errs() << "Error linking modules";
    exit(1);
  }
  // verifyModule(*pmodule, &outs());
  pmodule->print(outs(), nullptr);
}
