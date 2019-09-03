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

std::shared_ptr<cerata::Type> stream_probe() {
  static auto result = Stream::Make("", nul());
  return result;
}

std::shared_ptr<Component> Profiler(const std::shared_ptr<ClockDomain> &domain) {
  // Parameters
  auto out_count_max = Parameter::Make("OUT_COUNT_MAX", integer(), cerata::intl(1023));
  auto out_count_width = Parameter::Make("OUT_COUNT_WIDTH", integer(), cerata::intl(10));

  auto pcr = Port::Make("pcd", cr(), Port::Dir::IN, domain);
  auto probe = Port::Make("probe", stream_probe(), Port::Dir::IN);
  auto enable = Port::Make("enable", bit(), Port::Dir::IN);
  auto count = Port::Make("count", Vector::Make("out_count_type", out_count_width), Port::Dir::OUT);

  // Component & ports
  auto ret = Component::Make("StreamProfiler", {out_count_max, out_count_width, pcr, probe, enable, count});

  // VHDL metadata
  ret->SetMeta(cerata::vhdl::metakeys::PRIMITIVE, "true");
  ret->SetMeta(cerata::vhdl::metakeys::LIBRARY, "work");
  ret->SetMeta(cerata::vhdl::metakeys::PACKAGE, "Stream_pkg");

  return ret;
}

static void AttachStreamProfilers(cerata::Component *comp) {
  // Get all nodes and check if they are of a stream type, then check if they should be profiled.
  for (auto n : comp->GetNodes()) {
    if (n->type()->Is(cerata::Type::STREAM)) {
      if (n->meta.at(PROFILE_KEY) == "true") {
        // Figure out the clock domain of the stream node
        std::shared_ptr<ClockDomain> domain;
        if (n->IsPort()) {
          domain = n->AsPort().domain();
        } else if (n->IsSignal()) {
          domain = n->AsSignal().domain();
        } else {
          domain = cerata::default_domain();
        }
        auto cr_node = GetClockResetPort(comp, *domain);

        if (!cr_node) {
          throw std::runtime_error("No clock/reset port present on component for clock domain.");
        }

        // Instantiate a profiler
        auto profiler_inst = comp->AddInstanceOf(Profiler(domain).get());

        // Obtain the profiler ports
        auto p_probe = profiler_inst->port("probe");
        auto p_count = profiler_inst->port("count");
        auto p_en = profiler_inst->port("enable");

        // Copy profiler data and control to the top level port.
        auto c_count = std::dynamic_pointer_cast<Node>(p_count->Copy());
        auto c_en = std::dynamic_pointer_cast<Node>(p_en->Copy());
        c_count->SetName(n->name() + "_count");
        c_en->SetName(n->name() + "_enable");
        comp->AddObject(c_count);
        comp->AddObject(c_en);

        // Connect the probe, clock/reset, count and enable
        Connect(p_probe, n);
        Connect(profiler_inst->port("pcd"), *cr_node);
        c_count <<= p_count;
        p_en <<= c_en;
      }
    }
  }
}

cerata::Component *EnableStreamProfiling(cerata::Component *top) {
  std::deque<cerata::Graph *> graphs;
  cerata::GetAllGraphs(top, &graphs, true);
  for (auto g : graphs) {
    if (g->IsComponent()) {
      auto c = dynamic_cast<Component *>(g);
      AttachStreamProfilers(c);
    }
  }
  return top;
}

}  // namespace fletchgen
