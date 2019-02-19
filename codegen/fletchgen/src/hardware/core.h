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

#include "../../../../common/cpp/src/fletcher/common/arrow-utils.h"

#include "../utils.h"
#include "../graphs.h"

#include "schema.h"

namespace fletchgen {
namespace hardware {

/**
 * @brief A port derived from an Arrow field
 */
struct ArrowPort : public Port {
  std::shared_ptr<arrow::Field> field_;
  ArrowPort(std::string name, std::shared_ptr<arrow::Field> field, fletcher::Mode mode, Port::Dir dir);
  static std::shared_ptr<ArrowPort> Make(std::shared_ptr<arrow::Field> field, fletcher::Mode mode, Port::Dir dir);
};

/**
 * @brief The Core component to be implemented by the user
 */
struct Core : Component {
  std::shared_ptr<SchemaSet> schema_set_;

  explicit Core(std::string name, std::shared_ptr<SchemaSet> schemas);
  static std::shared_ptr<Core> Make(std::shared_ptr<SchemaSet> schemas);

  std::shared_ptr<ArrowPort> GetArrowPort(std::shared_ptr<arrow::Field> field);
  std::deque<std::shared_ptr<ArrowPort>> GetAllArrowPorts();
};


}  // namespace hardware
}  // namespace fletchgen
