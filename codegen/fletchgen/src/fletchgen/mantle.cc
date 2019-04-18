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

#include "fletchgen/mantle.h"

#include <memory>
#include <deque>
#include <utility>

#include <arrow/api.h>

#include "fletcher/common/arrow-utils.h"

#include "fletchgen/basic_types.h"
#include "fletchgen/array.h"
#include "fletchgen/schema.h"

namespace fletchgen {

Mantle::Mantle(std::string name, std::shared_ptr<SchemaSet> schema_set)
    : Component(std::move(name)), schema_set_(std::move(schema_set)) {
/*
  // Create and instantiate a Kernel
  kernel_ = Kernel::Make(schema_set_);
  auto kinst = Instance::Make(kernel_);
  kernel_inst_ = kinst.get();
  AddChild(std::move(kinst));

  // Connect Fields
  auto arrow_ports = kernel_->GetAllArrowPorts();

  // Connect all Arrow data ports to the Core
  for (size_t i = 0; i < arrow_ports.size(); i++) {
    if (arrow_ports[i]->IsInput()) {
      // Get the user core instance ports
      auto uci_data_port = kernel_inst_->port(arrow_ports[i]->name());
      auto uci_cmd_port = kernel_inst_->port(arrow_ports[i]->name() + "_cmd");
      // Get the column reader ports
      auto cr_data_port = array_readers_[i]->port("out");
      auto cr_cmd_port = array_readers_[i]->port("cmd");
      // Connect the ports
      uci_data_port <<= cr_data_port;
      cr_cmd_port <<= uci_cmd_port;
    } else {
      // TODO(johanpel): ArrayWriters
      // TODO(johanpel): ArrayWriters
    }
  }

  */
}

std::shared_ptr<Mantle> Mantle::Make(std::string name, const std::shared_ptr<SchemaSet> &schema_set) {
  return std::make_shared<Mantle>(name, schema_set);
}

std::shared_ptr<Mantle> Mantle::Make(const std::shared_ptr<SchemaSet> &schema_set) {
  return std::make_shared<Mantle>(schema_set->name() + "_mantle", schema_set);
}

}  // namespace fletchgen
