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
using cerata::intl;

/// @brief Return the width of the control data of this field.
std::shared_ptr<Node> ctrl_width(const std::shared_ptr<arrow::Field> &field);

// Default streams of ArrayReaders/ArrayWriters

///< @brief Fletcher command stream
std::shared_ptr<Type> cmd(const std::shared_ptr<Node> &ctrl_width = intl<1>(),
                          const std::shared_ptr<Node> &tag_width = intl<1>());
///< @brief Fletcher unlock stream
std::shared_ptr<Type> unlock(const std::shared_ptr<Node> &tag_width = intl<1>());
///< @brief Fletcher read data
std::shared_ptr<Type> read_data(std::shared_ptr<Node> data_width = intl<1>());
///< @brief Fletcher write data
std::shared_ptr<Type> write_data(std::shared_ptr<Node> data_width = intl<1>());

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
std::string GenerateConfigString(const std::shared_ptr<arrow::Field> &field, int level = 0);

/**
 * @brief Get a type mapper for an Arrow::Field-based stream to an ArrayReader/Writer stream.
 *
 * These type mappers can be automatically deduced based on the generic Fletcher types being used.
 *
 * @param stream_type   The stream type.
 * @param mode          Whether the stream is used for reading or writing.
 * @return              A TypeMapper that can map the Arrow data stream to an ArrayReader/Writer stream.
 */
std::shared_ptr<TypeMapper> GetStreamTypeMapper(const std::shared_ptr<Type> &stream_type, fletcher::Mode mode);

/**
 * @brief Convert an Arrow::Field into a stream type.
 * @param field The Arrow::Field to convert.
 * @param mode Whether this stream is used for reading or writing.
 * @param level Nesting level.
 * @return The Stream Type.
 */
std::shared_ptr<Type> GetStreamType(const std::shared_ptr<arrow::Field> &field, fletcher::Mode mode, int level = 0);

/// @brief Return a ArrayReader component.
std::shared_ptr<Component> ArrayReader(std::shared_ptr<Node> data_width = intl<1>(),
                                       const std::shared_ptr<Node> &ctrl_width = intl<1>(),
                                       const std::shared_ptr<Node> &tag_width = intl<1>());

}  // namespace fletchgen
