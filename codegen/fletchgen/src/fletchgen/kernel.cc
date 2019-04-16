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

#include "fletchgen/kernel.h"

#include <cerata/logging.h>
#include "fletcher/common/arrow-utils.h"

#include "fletchgen/basic_types.h"
#include "fletchgen/schema.h"

namespace fletchgen {

using cerata::Cast;

static Port::Dir mode2dir(fletcher::Mode mode) {
  if (mode == fletcher::Mode::READ) {
    return Port::Dir::IN;
  } else {
    return Port::Dir::OUT;
  }
}

ArrowPort::ArrowPort(std::string name, std::shared_ptr<arrow::Field> field, fletcher::Mode mode, Port::Dir dir)
    : Port(std::move(name), GetStreamType(field, mode), dir), field_(std::move(field)) {}

std::shared_ptr<ArrowPort> ArrowPort::Make(std::shared_ptr<arrow::Field> field, fletcher::Mode mode, Port::Dir dir) {
  return std::make_shared<ArrowPort>(field->name(), field, mode, dir);
}

Kernel::Kernel(std::string name, std::shared_ptr<SchemaSet> schema_set)
    : Component(std::move(name)), schema_set_(std::move(schema_set)) {
  LOG(DEBUG, "Construction kernel from SchemaSet: " + schema_set_->name());

  for (const auto &s : schema_set_->schema_list_) {
    // Get mode/direction
    auto mode = fletcher::getMode(s);
    auto dir = mode2dir(mode);

    // Get name, if available
    auto schema_name = fletcher::getMeta(s, "fletcher_name");
    if (schema_name.empty()) schema_name = "<Anonymous>";

    // Show some debug information about the schema
    LOG(DEBUG, "Schema " + schema_name + ", Direction: " + cerata::Term::ToString(dir));

    // Iterate over all fields
    for (const auto &f : s->fields()) {
      // Check if we must ignore a field
      if (!fletcher::mustIgnore(f)) {
        LOG(DEBUG, "Hardware-izing field: " + f->name());
        // Convert to and add an ArrowPort
        AddObject(ArrowPort::Make(f, mode, dir));
        // Add a command stream for the ArrayReader
        AddObject(Port::Make(f->name() + "_cmd", cmd(), Port::OUT));
      } else {
        LOG(DEBUG, "Ignoring field " + f->name());
      }
    }
  }
}

std::shared_ptr<Kernel> Kernel::Make(std::shared_ptr<SchemaSet> schema_set) {
  return std::make_shared<Kernel>(schema_set->name(), schema_set);
}

std::shared_ptr<ArrowPort> Kernel::GetArrowPort(const std::shared_ptr<arrow::Field> &field) {
  for (const auto &n : objects_) {
    auto ap = Cast<ArrowPort>(n);
    if (ap) {
      if ((*ap)->field_ == field) {
        return *ap;
      }
    }
  }
  throw std::runtime_error("Field " + field->name() + " did not generate an ArrowPort for Core " + name() + ".");
}
std::deque<std::shared_ptr<ArrowPort>> Kernel::GetAllArrowPorts() {
  std::deque<std::shared_ptr<ArrowPort>> result;
  for (const auto &n : objects_) {
    auto ap = Cast<ArrowPort>(n);
    if (ap) {
      result.push_back(*ap);
    }
  }
  return result;
}

}  // namespace fletchgen
