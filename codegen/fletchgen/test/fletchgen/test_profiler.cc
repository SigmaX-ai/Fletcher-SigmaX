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

#include <gtest/gtest.h>
#include <cerata/api.h>
#include <string>

#include "fletchgen/utils.h"
#include "fletchgen/profiler.h"
#include "fletchgen/test_utils.h"

namespace fletchgen {

TEST(Profiler, Connect) {
  cerata::logger().enable(fletchgen::LogCerata);
  cerata::default_component_pool()->Clear();

  auto stream_type = cerata::Stream::Make("test_stream", cerata::Vector::Make(8), "data");
  auto stream_port = cerata::Port::Make(stream_type);
  auto top = cerata::Component::Make("top", {stream_port});

  auto profiler = Profiler();
  auto profiler_inst = top->AddInstanceOf(profiler.get());
  auto probe = profiler_inst->port("probe");

  cerata::Connect(probe, stream_port.get());

  auto vhdl = cerata::vhdl::Design(top);
  auto top_code = vhdl.Generate().ToString();
  std::cerr.flush();
  std::cout << top_code << std::endl;
  VHDL_DUMP_TEST(top_code);
}

}  // namespace fletchgen
