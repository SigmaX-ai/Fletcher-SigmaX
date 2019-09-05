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
#include <fletcher/logging.h>
#include <fletcher/common.h>
#include <cerata/api.h>
#include <memory>
#include <string>
#include <fstream>

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
         "  name: Fletchgen\n"
         "  doc: happy bunnies\n"
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
  for (size_t r = 0; r < batches.size(); r++) {
    addr += 4;
    ofs << "  - address: " << addr << "\n";
    ofs << "    name: " << batches[r].name << "_firstidx" << "\n";
    ofs << "    behavior: control\n";
    addr += 4;
    ofs << "  - address: " << addr << "\n";
    ofs << "    name: " << batches[r].name << "_lastidx" << "\n";
    ofs << "    behavior: control\n";
    ofs << "\n";
  }

  for (const auto &r : batches) {
    for (const auto &b : r.buffers) {
      addr += 4;
      ofs << "  - address: " << addr << "\n";
      ofs << "    name: " << r.name + "_" + fletcher::ToString(b.desc_) << "_lo" << "\n";
      ofs << "    behavior: control\n";
      addr += 4;
      ofs << "  - address: " << addr << "\n";
      ofs << "    name: " << r.name + "_" + fletcher::ToString(b.desc_) << "_hi" << "\n";
      ofs << "    behavior: control\n";
    }
  }
}

}  // namespace fletchgen
