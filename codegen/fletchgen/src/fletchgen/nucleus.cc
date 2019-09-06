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

#include "fletchgen/nucleus.h"

#include <utility>
#include <vector>
#include <string>

#include "fletchgen/basic_types.h"
#include "fletchgen/recordbatch.h"
#include "fletchgen/kernel.h"
#include "fletchgen/mmio.h"

namespace fletchgen {

Nucleus::Nucleus(const std::string &name,
                 const std::deque<RecordBatch *> &recordbatches,
                 const std::vector<fletcher::RecordBatchDescription> &batch_desc)
    : Component("Nucleus_" + name) {

  // Add address width
  AddObject(bus_addr_width());

  // Add clock/reset
  auto kcd = Port::Make("kcd", cr(), Port::Dir::IN, kernel_cd());
  AddObject(kcd);

  // Add MMIO port
  auto mmio = MmioPort::Make(Port::Dir::IN);
  AddObject(mmio);

  // Add VHDMMIO component
  auto vhdmmio = GenerateMmioComponent(batch_desc);
  auto vhdmmio_inst = AddInstanceOf(vhdmmio.get());

  vhdmmio_inst->port("mmio") <<= mmio;
  vhdmmio_inst->port("kcd") <<= kcd;

  // Create kernel
  kernel = Kernel::Make(name, this);

  // Add ports derived from arrow fields
  for (const auto &r : recordbatches) {
    auto field_ports = r->GetFieldPorts();
    for (const auto &fp : field_ports) {
      // Create a copy and invert for the Nucleus
      auto copied_port = std::dynamic_pointer_cast<FieldPort>(fp->Copy());
      copied_port->InvertDirection();
      AddObject(copied_port);
      // Create and add another copy for the kernel component
      kernel->AddObject(copied_port->Copy());
    }
  }

  // Add ports from mmio
  for (const auto &p : vhdmmio_inst->GetAll<Port>()) {
    if (p->meta.count(MMIO_KERNEL) > 0) {
      auto name_intended = p->meta.at(MMIO_KERNEL);
      auto kernel_mmio_port = std::dynamic_pointer_cast<Port>(p->Copy());
      kernel_mmio_port->InvertDirection();
      kernel_mmio_port->SetName(name_intended);
      kernel->AddObject(kernel_mmio_port);
    }
  }

  kernel_inst = AddInstanceOf(kernel.get());

  // Connect all kernel ports
  for (auto &p : vhdmmio_inst->GetAll<Port>()) {
    if (p->meta.count(MMIO_KERNEL) > 0) {
      auto name_intended = p->meta.at(MMIO_KERNEL);
      auto inst_port = kernel_inst->port(name_intended);
      if (p->dir() == Port::Dir::OUT) {
        Connect(inst_port, p);
      } else {
        Connect(p, inst_port);
      }
    }
  }
  // Connect field buffer addresses to command stream ctrl field
  if (recordbatches.size() != batch_desc.size()) {
    throw std::runtime_error("all of my wut");
  }
  for (size_t r = 0; r < recordbatches.size(); r++) {
    auto cmd_ports = recordbatches[r]->GetFieldPorts(FieldPort::Function::COMMAND);
    for (size_t f = 0; f < cmd_ports.size(); f++) {
      auto inst_port = port(cmd_ports[f]->name());
      std::cerr << inst_port->name() << std::endl;
      //Connect(inst_port, )
      std::cerr << cmd_ports[f]->field_->name() << std::endl;
    }
  }
}

std::shared_ptr<Nucleus> Nucleus::Make(const std::string &name,
                                       const std::deque<RecordBatch *> &recordbatches,
                                       const std::vector<fletcher::RecordBatchDescription> &batch_desc) {
  return std::make_shared<Nucleus>(name, recordbatches, batch_desc);
}

}  // namespace fletchgen
