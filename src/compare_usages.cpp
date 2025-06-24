#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <string>

using json = nlohmann::json;

void compareStructs(const json& codeUsages, const json& msgUsages) {
  for (auto& [structName, fieldsCode] : codeUsages.items()) {
    const auto& fieldsMsg = msgUsages.contains(structName) ? msgUsages[structName] : json::array();

    std::set<std::string> codeSet(fieldsCode.begin(), fieldsCode.end());
    std::set<std::string> msgSet(fieldsMsg.begin(), fieldsMsg.end());

    std::cout << "Struct: \033[1;36m" << structName << "\033[0m\n";

    // 所有字段的并集
    std::set<std::string> allFields;
    allFields.insert(codeSet.begin(), codeSet.end());
    allFields.insert(msgSet.begin(), msgSet.end());

    for (const auto& field : allFields) {
      bool inCode = codeSet.count(field);
      bool inMsg = msgSet.count(field);

      if (inCode && inMsg) {
        std::cout << "  \033[32m✔ " << field << " (both)\033[0m\n";
      } else if (inCode) {
        std::cout << "  \033[33m← " << field << " (only in code)\033[0m\n";
      } else {
        std::cout << "  \033[31m→ " << field << " (only in msg)\033[0m\n";
      }
    }

    std::cout << std::endl;
  }

  // 查找 msgUsages 中独有的结构体
  for (auto& [structName, fieldsMsg] : msgUsages.items()) {
    if (!codeUsages.contains(structName)) {
      std::cout << "Struct: \033[1;36m" << structName << "\033[0m (only in msg)\n";
      for (const auto& field : fieldsMsg) {
        std::cout << "  \033[31m→ " << field << " (only in msg)\033[0m\n";
      }
      std::cout << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " code.json msg.json\n";
    return 1;
  }

  std::ifstream codeFile(argv[1]);
  std::ifstream msgFile(argv[2]);

  if (!codeFile.is_open() || !msgFile.is_open()) {
    std::cerr << "Failed to open input files.\n";
    return 1;
  }

  json codeUsages, msgUsages;
  try {
    codeFile >> codeUsages;
    msgFile >> msgUsages;
  } catch (const json::parse_error& e) {
    std::cerr << "Failed to parse JSON: " << e.what() << "\n";
    return 1;
  }

  compareStructs(codeUsages, msgUsages);

  return 0;
}

