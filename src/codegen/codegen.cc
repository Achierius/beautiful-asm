#include "codegen.hh"

#include <memory>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;
using namespace bytecode;

std::pair<llvm::Value *, llvm::Value *> popTwo(std::vector<llvm::Value *> &stack) {
    llvm::Value *a = *(stack.end() - 2);
    llvm::Value *b = *(stack.end() - 1);
    stack.erase(stack.end() - 2, stack.end());
    return std::make_pair(a, b);
}

class Codegen {
  private:
    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder<>> builder;
    BytecodeExecutable &bytecode;

    std::vector<Function *> functions;

    void declareFunctions();
    void defineFunctions();
    void emitFunction(Function *f, BytecodeChunk &chunk, Address &address);

  public:
    Codegen(BytecodeExecutable &bytecode);
    void generate();
    void writeObject(std::string_view outputPath);
};

void Codegen::declareFunctions() {
    if (functions.size() > 0)
        return; // functions already declared

    auto voidTy = Type::getVoidTy(*context);
    auto *fnTy = FunctionType::get(voidTy, false);
    for (size_t i = 0; i < bytecode.chunks.size(); i++) {
        auto name = "fn" + i;
        auto *f = Function::Create(fnTy, Function::ExternalLinkage, name, *module);
        functions.push_back(f);
    }
}

void Codegen::defineFunctions() {
    if (bytecode.chunks.size() != functions.size())
        return; // functions not declared yet (TODO: log)

    for (size_t i = 0; i < bytecode.chunks.size(); i++) {
        auto &[chunk, address] = bytecode.chunks[i];
        auto *f = functions[i];

        emitFunction(f, chunk, address);
    }
}

void Codegen::emitFunction(Function *f, BytecodeChunk &chunk, Address &address) {
    BasicBlock *bb = BasicBlock::Create(*context, "entry", f);
    builder->SetInsertPoint(bb);

    std::vector<llvm::Value *> stack;

    for (auto &ins : chunk.code) {
        switch (ins.opcode) {
        case kTrap:
            // TODO
            break;
        case kReturn:
            builder->CreateRetVoid();
            break;
        case kBreakpoint:
            // TODO
            break;
        case kPrintLong:
            // TODO
            break;
        case kPrintChar:
            // TODO
            break;
        case kNop:
            break;
        case kAddLong: {
            auto [a, b] = popTwo(stack);
            stack.push_back(builder->CreateAdd(a, b, ""));
            break;
        }
        case kMulLong: {
            auto [a, b] = popTwo(stack);
            stack.push_back(builder->CreateMul(a, b, ""));
            break;
        }
        case kImmByte: {
            auto *ty = Type::getInt64Ty(*context);
            auto *val = ConstantInt::get(ty, 0, true);
            stack.push_back(val);
            break;
        }
        case kDup: {
            stack.push_back(stack.back());
            break;
        }
        case kRot2:
            // TODO
            break;
        case kRot3:
            // TODO
            break;
        default:
            // TODO: unimplemented
            break;
        }
    }

    verifyFunction(*f);
}

Codegen::Codegen(BytecodeExecutable &bytecode) : bytecode{bytecode} {
    context = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("beautiful-asm-test", *context);
    builder = std::make_unique<IRBuilder<>>(*context);
}

void Codegen::generate() {
    declareFunctions();
    defineFunctions();
}

void Codegen::writeObject(std::string_view outputPath) {
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmPrinters();

    auto targetTriple = sys::getDefaultTargetTriple();
    std::string err;
    auto target = TargetRegistry::lookupTarget(targetTriple, err);

    if (!target) {
        // TODO: log `err`
        return;
    }

    // use generic cpu for now
    auto cpu = "generic";
    auto features = "";
    TargetOptions opt;
    auto rm = Optional<Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);

    std::error_code ec;
    raw_fd_ostream dest(outputPath, ec, sys::fs::OF_None);
    if (ec) {
        // TODO: error: could not open file
        return;
    }

    legacy::PassManager pass;
    auto fileType = CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        // TODO: error: failed to add emit pass
        return;
    }

    pass.run(*module);
    dest.flush();
}

namespace codegen {

void Generate(BytecodeExecutable &bytecode, std::string_view outputPath) {
    Codegen codegen(bytecode);
    codegen.generate();
    codegen.writeObject(outputPath);
}

} // end namespace codegen