#include <clang/AST/AST.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/Support/CommandLine.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <set>
#include <map>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using json = nlohmann::json;

// 控制台颜色
#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_RESET "\033[0m"

static llvm::cl::OptionCategory ToolCategory("field-usage-checker");
static llvm::cl::opt<std::string> StructFieldJsonPath(
    "field-json", llvm::cl::desc("Path to struct_fields.json"), llvm::cl::value_desc("filename"), llvm::cl::Required, llvm::cl::cat(ToolCategory));

class FieldUsageMatcher : public MatchFinder::MatchCallback {
public:
  std::map<std::string, std::set<std::string>> usedFields;

  void run(const MatchFinder::MatchResult &Result) override {
    if (const MemberExpr *ME = Result.Nodes.getNodeAs<MemberExpr>("memberExpr")) {
      if (const FieldDecl *FD = dyn_cast_or_null<FieldDecl>(ME->getMemberDecl())) {
        if (const RecordDecl *RD = dyn_cast_or_null<RecordDecl>(FD->getParent())) {
          std::string structName = RD->getNameAsString();
          std::string fieldName = FD->getNameAsString();
          if (!structName.empty() && !fieldName.empty()) {
            usedFields[structName].insert(fieldName);
          }
        }
      }
    }
  }
};

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolCategory);
  if (!ExpectedParser) {
    llvm::errs() << llvm::toString(ExpectedParser.takeError()) << "\n";
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  // 读取 struct_fields.json
  json structFields;
  {
    std::ifstream ifs(StructFieldJsonPath);
    if (!ifs) {
      std::cerr << "Failed to open " << StructFieldJsonPath << "\n";
      return 1;
    }
    ifs >> structFields;
  }

  FieldUsageMatcher matcher;
  MatchFinder finder;
  finder.addMatcher(memberExpr().bind("memberExpr"), &matcher);

  if (Tool.run(newFrontendActionFactory(&finder).get()) != 0) {
    std::cerr << "Tool execution failed.\n";
    return 1;
  }

  // 输出
  for (auto &[structName, fields] : structFields.items()) {
    std::cout << "Struct: " << structName << "\n";

    std::set<std::string> used = matcher.usedFields[structName];
    for (const std::string &field : fields) {
      if (used.count(field)) {
        std::cout << "  " COLOR_GREEN "✓ " << field << COLOR_RESET "\n";
      } else {
        std::cout << "  " COLOR_RED "✗ " << field << COLOR_RESET "\n";
      }
    }
    std::cout << std::endl;
  }

  return 0;
}

