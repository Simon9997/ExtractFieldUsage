#!/bin/bash

# 检查参数数量
if [ "$#" -ne 3 ]; then
  echo "Usage: $0 <build_dir> <header_file> <source_file>"
  echo "Example:"
  echo "  $0 /root/TDengine/debug /root/TDengine/include/libs/nodes/plannodes.h /root/TDengine/source/libs/nodes/src/nodesCodeFuncs.c"
  exit 1
fi

# 参数定义
BUILD_DIR=$1
HEADER_FILE=$2
SOURCE_FILE=$3
STRUCT_JSON="struct_fields.json"

# 步骤 1: 生成 struct_fields.json
echo ">> Generating struct field list from $HEADER_FILE ..."
./build/extract_struct_fields -p "$BUILD_DIR" --extra-arg=-Wno-error=unknown-warning-option "$HEADER_FILE" > "$STRUCT_JSON"

if [ $? -ne 0 ]; then
  echo "❌ Failed to generate struct fields."
  exit 2
fi

echo "✅ Generated $STRUCT_JSON"

# 步骤 2: 检查字段使用情况
echo ">> Running field usage checker on $SOURCE_FILE ..."
./build/field_usage_checker -p "$BUILD_DIR" \
  --extra-arg=-Wno-error=unknown-warning-option \
  --extra-arg=-Wno-error \
  --field-json="$STRUCT_JSON" \
  "$SOURCE_FILE"

if [ $? -ne 0 ]; then
  echo "❌ Field usage check failed."
  exit 3
fi

echo "✅ Field usage check completed."

