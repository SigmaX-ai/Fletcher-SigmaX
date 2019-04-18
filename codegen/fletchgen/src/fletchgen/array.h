// Copyright 2018 Delft University of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>

#include <cerata/api.h>

#include "fletchgen/basic_types.h"

namespace fletchgen {

using cerata::Component;

/// @brief Types for ArrayReader/Writer configuration string.
enum class ConfigType {
  ARB,        ///< Arbiter level.
  NUL,        ///< Null bitmap.
  PRIM,       ///< Primitive (fixed-width) fields.
  LIST,       ///< Variable length fields.
  LISTPRIM,   ///< List of primitives. Can have EPC > 1.
  STRUCT      ///< Structs, composed of multiple fields.
};

/**
 * @brief Return a node representing the width of a (flat) Arrow DataType.
 * @param type  The arrow::DataType.
 * @return      A Literal Node representing the width.
 */
std::shared_ptr<Node> GetWidth(const arrow::DataType *type);

/**
 * @brief Return the configuration string type version of an arrow::DataType.
 * @param type  The arrow::DataType.
 * @return      The equivalent ConfigType.
 */
ConfigType GetConfigType(const arrow::DataType *type);

/**
 * @brief Return the configuration string for a ArrayReader/Writer.
 * @param field The arrow::Field to derive the string from.
 * @param level Nesting level for recursive calls to this function.
 * @return      The string.
 */
std::string GenerateConfigString(const std::shared_ptr<arrow::Field> &field, int level=0);

/// @brief Return a ArrayReader component.
std::shared_ptr<Component> ArrayReader();

}  // namespace fletchgen
