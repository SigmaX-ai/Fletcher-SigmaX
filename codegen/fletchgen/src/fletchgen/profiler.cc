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

#include "fletchgen/profiler.h"

#include <cerata/api.h>

#include <memory>

#include "fletchgen/basic_types.h"

namespace fletchgen {

using cerata::Component;
using cerata::Parameter;
using cerata::Port;
using cerata::Stream;
using cerata::Vector;
using cerata::integer;
using cerata::bit;
using cerata::nul;

std::shared_ptr<Component> Profiler() {
  // Parameters
  auto out_count_max = Parameter::Make("OUT_COUNT_MAX", integer(), cerata::intl(1023));
  auto out_count_width = Parameter::Make("OUT_COUNT_WIDTH", integer(), cerata::intl(10));

  // Component & ports
  static auto ret = Component::Make("StreamProfiler", {
      Port::Make(bus_cr()),
      Port::Make("probe", Stream::Make("", nul()), Port::Dir::IN),
      Port::Make("enable", bit(), Port::Dir::IN),
      Port::Make("count", Vector::Make("out_count_type", out_count_width), Port::Dir::OUT)});

  // VHDL metadata
  ret->SetMeta(cerata::vhdl::metakeys::PRIMITIVE, "true");
  ret->SetMeta(cerata::vhdl::metakeys::LIBRARY, "work");
  ret->SetMeta(cerata::vhdl::metakeys::PACKAGE, "Stream_pkg");

  return ret;
}


}  // namespace fletchgen
