#include "clang/AST/AST.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include <nlohmann/json.hpp>
#include <iostream>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using json = nlohmann::json;

static llvm::cl::OptionCategory ToolCategory("extract-struct-fields");
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

class StructMatcher : public MatchFinder::MatchCallback {
public:
  json output;

  void run(const MatchFinder::MatchResult &Result) override {
    const RecordDecl *RD = Result.Nodes.getNodeAs<RecordDecl>("structDecl");
    if (RD && RD->isStruct() && RD->isThisDeclarationADefinition()) {
      const SourceManager *SM = Result.SourceManager;
      if (!SM)
        return;

      SourceLocation Loc = RD->getLocation();
      // 只保留主文件中的结构体定义
      if (Loc.isValid() && SM->isInMainFile(Loc)) {
        std::string structName = RD->getNameAsString();
        json fields = json::array();
        for (const FieldDecl *field : RD->fields()) {
          std::string fieldName = field->getNameAsString();
          if (!fieldName.empty()) {
            fields.push_back(fieldName);
          }
        }
        output[structName] = fields;
      }
    }
  }
};

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolCategory, llvm::cl::Optional, nullptr);
  if (!ExpectedParser) {
    llvm::errs() << "Failed to parse command-line arguments: "
                 << llvm::toString(ExpectedParser.takeError()) << "\n";
    return 1;
  }

  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  StructMatcher matcher;
  MatchFinder finder;
  finder.addMatcher(recordDecl(isStruct()).bind("structDecl"), &matcher);

  int result = Tool.run(newFrontendActionFactory(&finder).get());
  if (result != 0) {
    llvm::errs() << "Tool execution failed.\n";
    return result;
  }

  std::cout << matcher.output.dump(2) << std::endl;
  return 0;
}

