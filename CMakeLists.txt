cmake_minimum_required(VERSION 3.13)
project(ExtractFieldUsage)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 生成 extract_struct_fields 可执行文件
add_executable(extract_struct_fields
  src/extract_struct_fields.cpp
)

target_link_libraries(extract_struct_fields
  clangTooling
  clangBasic
  clangASTMatchers
)

# 生成 field_usage_checker 可执行文件
add_executable(field_usage_checker
  src/field_usage_checker.cpp
)

target_link_libraries(field_usage_checker
  clangTooling
  clangBasic
  clangASTMatchers
)

# 生成 compare_usages 可执行文件
add_executable(compare_usages
  src/compare_usages.cpp
)

target_link_libraries(compare_usages
  nlohmann_json::nlohmann_json
)

find_package(nlohmann_json REQUIRED)
target_link_libraries(compare_usages nlohmann_json::nlohmann_json)
