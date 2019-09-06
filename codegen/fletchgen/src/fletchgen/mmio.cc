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

#include "fletchgen/mmio.h"

#include <fletcher/fletcher.h>
#include <fletcher/common.h>
#include <cerata/api.h>
#include <memory>
#include <string>
#include <fstream>

#include "fletchgen/basic_types.h"
#include "fletchgen/kernel.h"

namespace fletchgen {

using cerata::Parameter;
using cerata::Port;
using cerata::PortArray;
using cerata::intl;
using cerata::integer;
using cerata::string;
using cerata::strl;
using cerata::boolean;
using cerata::integer;
using cerata::bit;
using cerata::RecField;
using cerata::Vector;
using cerata::Record;
using cerata::Stream;

std::shared_ptr<Type> mmio(MmioSpec spec) {
  auto mmio_typename = spec.ToMMIOTypeName();
  auto optional_existing_mmio_type = cerata::default_type_pool()->Get(mmio_typename);
  if (optional_existing_mmio_type) {
    FLETCHER_LOG(DEBUG, "MMIO Type already exists in default pool.");
    return optional_existing_mmio_type.value()->shared_from_this();
  } else {
    auto new_mmio_type = Record::Make(mmio_typename, {
        NoSep(RecField::Make("aw", Stream::Make(Record::Make("aw", {
            RecField::Make("addr", Vector::Make(spec.addr_width))})))),
        NoSep(RecField::Make("w", Stream::Make(Record::Make("w", {
            RecField::Make("data", Vector::Make(spec.data_width)),
            RecField::Make("strb", Vector::Make(spec.data_width / 8))
        })))),
        NoSep(RecField::Make("b", Stream::Make(Record::Make("b", {
            RecField::Make("resp", Vector::Make(2))})), true)),
        NoSep(RecField::Make("ar", Stream::Make(Record::Make("ar", {
            RecField::Make("addr", Vector::Make(spec.addr_width))
        })))),
        NoSep(RecField::Make("r", Stream::Make(Record::Make("r", {
            RecField::Make("data", Vector::Make(spec.data_width)),
            RecField::Make("resp", Vector::Make(2))})), true)),
    });
    cerata::default_type_pool()->Add(new_mmio_type);
    return new_mmio_type;
  }
}

std::string MmioSpec::ToString() const {
  std::stringstream str;
  str << "MmioSpec[";
  str << "addr:" << addr_width;
  str << ", dat:" << data_width;
  str << "]";
  return str.str();
}

std::string MmioSpec::ToMMIOTypeName() const {
  std::stringstream str;
  str << "MMIO";
  str << "_A" << addr_width;
  str << "_D" << data_width;
  return str.str();
}

std::shared_ptr<MmioPort> MmioPort::Make(Port::Dir dir, MmioSpec spec, const std::shared_ptr<ClockDomain> &domain) {
  return std::make_shared<MmioPort>(dir, spec, "mmio", domain);
}

void GenerateVhdmmioYaml(const std::vector<fletcher::RecordBatchDescription> &batches) {
  auto ofs = std::ofstream("fletchgen.mmio.yaml");
  ofs << "metadata:\n"
         "  name: mmio\n"
         "  doc: Fletchgen generated MMIO configuration.\n"
         "  \n"
         "entity:\n"
         "  bus-flatten:  yes\n"
         "  bus-prefix:   mmio_\n"
         "  clock-name:   bcd_clk\n"
         "  reset-name:   bcd_reset\n"
         "\n"
         "features:\n"
         "  bus-width:    32\n"
         "  optimize:     yes\n"
         "\n"
         "interface:\n"
         "  flatten:      yes\n"
         "\n"
         "fields: \n"
         "  - address: 0\n"
         "    register-name: control\n"
         "    bitrange: 0\n"
         "    name: start\n"
         "    behavior: strobe\n"
         "\n"
         "  - address: 0\n"
         "    register-name: control\n"
         "    bitrange: 1\n"
         "    name: stop\n"
         "    behavior: strobe\n"
         "\n"
         "  - address: 0\n"
         "    register-name: control\n"
         "    bitrange: 2\n"
         "    name: reset\n"
         "    behavior: strobe\n"
         "\n"
         "  - address: 4\n"
         "    register-name: status\n"
         "    bitrange: 0\n"
         "    name: idle\n"
         "    behavior: status\n"
         "\n"
         "  - address: 4\n"
         "    register-name: status\n"
         "    bitrange: 1\n"
         "    name: busy\n"
         "    behavior: status\n"
         "\n"
         "  - address: 4\n"
         "    register-name: status\n"
         "    bitrange: 2\n"
         "    name: done\n"
         "    behavior: status\n"
         "\n"
         "  - address: 8\n"
         "    bitrange: 63..0\n"
         "    name: result\n"
         "    behavior: status\n"
         "\n";

  int addr = 4 * FLETCHER_REG_SCHEMA;
  for (const auto &r : batches) {
    ofs << "  - address: " << addr << "\n";
    ofs << "    doc: " << "First index to process from Arrow RecordBatch " << r.name << "\n";
    ofs << "    name: " << r.name << "_firstidx" << "\n";
    ofs << "    behavior: control\n";
    addr += 4;
    ofs << "  - address: " << addr << "\n";
    ofs << "    doc: " << "Last index to process from Arrow RecordBatch " << r.name << "\n";
    ofs << "    name: " << r.name << "_lastidx" << "\n";
    ofs << "    behavior: control\n";
    addr += 4;
    ofs << "\n";
  }

  for (const auto &r : batches) {
    for (const auto &f : r.fields) {
      for (const auto &b : f.buffers) {
        ofs << "  - address: " << addr << "\n";
        ofs << "    doc: " << "RecordBatch " + r.name + " field " + b.desc_[0] + "\n";
        ofs << "    name: " << r.name + "_" + fletcher::ToString(b.desc_) << "\n";
        ofs << "    bitrange: 63..0\n";
        ofs << "    behavior: control\n";
        ofs << "\n";
        addr += 8;
        // addr += 4;
        // ofs << "  - address: " << addr << "\n";
        // ofs << "    name: " << r.name + "_" + fletcher::ToString(b.desc_) << "_hi" << "\n";
        // ofs << "    behavior: control\n";
      }
    }
  }
}

/// @brief Create a port, but rename it to the vhdmmio naming convention.
static std::shared_ptr<Port> MmioPort(const std::string &name,
                                      const std::shared_ptr<Type> &type,
                                      Port::Dir dir = Port::Dir::IN) {
  std::string suffix = std::string(dir == Port::Dir::IN ? "_write" : "") + "_data";
  auto result = Port::Make("f_" + name + suffix, type, dir, kernel_cd());
  return result;
}

/// @brief Create a port, but rename it to the vhdmmio naming convention, and place the intended name in metadata.
static std::shared_ptr<Port> MmioKernelPort(const std::string &name,
                                            const std::shared_ptr<Type> &type,
                                            Port::Dir dir = Port::Dir::IN) {
  auto result = MmioPort(name, type, dir);
  result->meta[MMIO_KERNEL] = name;
  return result;
}
/// @brief Create a port, but rename it to the vhdmmio naming convention, and annotate with buffer address annotation
static std::shared_ptr<Port> MmioBufferPort(const std::string &name,
                                            const std::shared_ptr<Type> &type,
                                            Port::Dir dir = Port::Dir::IN) {
  auto result = MmioPort(name, type, dir);
  result->meta[MMIO_BUFFER] = name;
  return result;
}


std::shared_ptr<Component> GenerateMmioComponent(const std::vector<fletcher::RecordBatchDescription> &batches) {
  auto reg32 = Vector::Make(32);
  auto reg64 = Vector::Make(64);

  auto kcd = Port::Make("kcd", cr(), Port::Dir::IN, kernel_cd());

  // Kernel ports, i.e. ports that should be exposed to the kernel
  auto comp = Component::Make("mmio",
                              {kcd,
                               MmioKernelPort("start", bit(), Port::Dir::OUT),
                               MmioKernelPort("stop", bit(), Port::Dir::OUT),
                               MmioKernelPort("reset", bit(), Port::Dir::OUT),
                               MmioKernelPort("idle", bit()),
                               MmioKernelPort("busy", bit()),
                               MmioKernelPort("done", bit()),
                               MmioKernelPort("result", reg64)});

  for (const auto &b : batches) {
    comp->AddObject(MmioKernelPort(b.name + "_firstidx", reg32, Port::Dir::OUT));
    comp->AddObject(MmioKernelPort(b.name + "_lastidx", reg32, Port::Dir::OUT));
  }

  for (const auto &r : batches) {
    for (const auto &f : r.fields) {
      for (const auto &b : f.buffers) {
        comp->AddObject(MmioBufferPort(r.name + "_" + fletcher::ToString(b.desc_), reg64, Port::Dir::OUT));
      }
    }
  }

  auto bus = Port::Make("mmio", mmio(), Port::Dir::IN, kernel_cd());

  comp->AddObject(bus);

  // comp->SetMeta(cerata::vhdl::metakeys::PRIMITIVE, "true");
  // comp->SetMeta(cerata::vhdl::metakeys::LIBRARY, "work");
  // comp->SetMeta(cerata::vhdl::metakeys::PACKAGE, "mmio_pkg");

  return comp;
}

}  // namespace fletchgen
