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

#include "fletchgen/recordbatch.h"

#include <cerata/api.h>

#include "fletchgen/array.h"
#include "fletchgen/bus.h"
#include "fletchgen/schema.h"

namespace fletchgen {

using cerata::Cast;
using cerata::Instance;
using cerata::Component;
using cerata::Port;
using cerata::Literal;
using cerata::intl;

RecordBatchReader::RecordBatchReader(const std::shared_ptr<FletcherSchema> &fletcher_schema)
    : Component(fletcher_schema->name() + "_RecordBatchReader"), fletcher_schema_(fletcher_schema) {
  // Get Arrow Schema
  auto as = fletcher_schema_->arrow_schema();
  // Get mode/direction
  auto mode = fletcher::getMode(as);
  // Add default port nodes
  AddObject(Port::Make(bus_cr()));
  AddObject(Port::Make(kernel_cr()));
  // Add and connect all array readers and resulting ports
  AddArrayReaders(as, mode);
}

void RecordBatchReader::AddArrayReaders(const std::shared_ptr<arrow::Schema> &as, const Mode &mode) {
  // Iterate over all fields and add ArrayReader data and control ports.
  for (const auto &f : as->fields()) {
    // Check if we must ignore a field
    if (!fletcher::mustIgnore(f)) {
      LOG(DEBUG, "Instantiating ArrayReader for field: " + f->name());
      // Convert to and add an Arrow port. We must invert it because it is an output of the RecordBatchReader.
      auto arrow_port = FieldPort::MakeArrowPort(f, mode, true);
      AddObject(arrow_port);
      // Add a command port for the ArrayReader
      auto command_port = FieldPort::MakeCommandPort(f);
      AddObject(command_port);
      // Add an unlock port for the ArrayReader
      auto unlock_port = FieldPort::MakeUnlockPort(f);
      AddObject(unlock_port);
      // Instantiate an ArrayReader
      auto array_reader = AddInstanceOf(
          ArrayReader(arrow_port->data_width(), ctrl_width(f)),
          f->name() + "_ArrayReader_Inst");
      // Generate a configuration string for the ArrayReader
      auto cfg_node = array_reader->GetNode(Node::PARAMETER, "CFG");
      // Set the configuration string for this field
      cfg_node <<= cerata::strl(GenerateConfigString(f));
      // Drive the clocks and resets
      array_reader->port("kcd") <<= port("kcd");
      array_reader->port("bcd") <<= port("bcd");
      // Drive the RecordBatch Arrow output port with the ArrayReader output port
      arrow_port <<= array_reader->port("out");
      // Drive the ArrayReader command port from the top-level command port
      array_reader->port("cmd") <<= command_port;
      // Drive the unlock port with the ArrayReader
      unlock_port <<= array_reader->port("unl");
      // Copy over the ArrayReader's bus channels
      auto bus = *Cast<BusPort>(array_reader->port("bus")->Copy());
      // Give the new bus port a unique name
      bus->SetName(f->name() + "_" + bus->name());
      // Add them to the RecordBatchReader
      AddObject(bus);
      // Remember the port
      bus_ports_.push_back(bus);
      // Connect them to the ArrayReader
      bus <<= array_reader->port("bus");
      // Remember where the ArrayReader is
      reader_instances_.push_back(array_reader);
    } else {
      LOG(DEBUG, "Ignoring field " + f->name());
    }
  }
}

std::shared_ptr<FieldPort> RecordBatchReader::GetArrowPort(const std::shared_ptr<arrow::Field> &field) const {
  for (const auto &n : objects_) {
    auto ap = Cast<FieldPort>(n);
    if (ap) {
      if (((*ap)->function_ == FieldPort::ARROW) && ((*ap)->field_ == field)) {
        return *ap;
      }
    }
  }
  throw std::runtime_error("Field " + field->name() + " did not generate an ArrowPort for Core " + name() + ".");
}

std::deque<std::shared_ptr<FieldPort>> RecordBatchReader::GetFieldPorts(const std::optional<FieldPort::Function> &function) const {
  std::deque<std::shared_ptr<FieldPort>> result;
  for (const auto &n : objects_) {
    auto ap = Cast<FieldPort>(n);
    if (ap) {
      if ((function && ((*ap)->function_ == *function)) || !function) {
        result.push_back(*ap);
      }
    }
  }
  return result;
}

std::shared_ptr<RecordBatchReader> RecordBatchReader::Make(const std::shared_ptr<FletcherSchema> &fletcher_schema) {
  return std::make_shared<RecordBatchReader>(fletcher_schema);
}

std::shared_ptr<FieldPort> FieldPort::MakeArrowPort(std::shared_ptr<arrow::Field> field, Mode mode, bool invert) {
  Dir dir;
  if (invert) {
    dir = Term::Invert(mode2dir(mode));
  } else {
    dir = mode2dir(mode);
  }
  return std::make_shared<FieldPort>(field->name(), ARROW, field, GetStreamType(field, mode), dir);
}

std::shared_ptr<FieldPort> FieldPort::MakeCommandPort(std::shared_ptr<arrow::Field> field) {
  return std::make_shared<FieldPort>(field->name() + "_cmd", COMMAND, field, cmd(ctrl_width(field), tag_width(field)), Dir::IN);
}

std::shared_ptr<FieldPort> FieldPort::MakeUnlockPort(std::shared_ptr<arrow::Field> field) {
  return std::make_shared<FieldPort>(field->name() + "_unl", UNLOCK, field, unlock(tag_width(field)), Dir::OUT);
}

std::shared_ptr<Node> FieldPort::data_width() {
  std::shared_ptr<Node> width = intl<0>();
  // Flatten the type
  auto flat_type = Flatten(type().get());
  for (const auto &ft : flat_type) {
    if (ft.type_->meta.count("array_data") > 0) {
      if (ft.type_->width()) {
        width = width + *ft.type_->width();
      }
    }
  }
  return width;
}

std::shared_ptr<cerata::Object> FieldPort::Copy() const {
  return std::make_shared<FieldPort>(name(), function_, field_, type(), dir());
}

} // namespace fletchgen