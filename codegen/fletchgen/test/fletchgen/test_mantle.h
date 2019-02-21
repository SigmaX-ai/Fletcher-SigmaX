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

#include <arrow/api.h>
#include <gtest/gtest.h>
#include <deque>
#include <memory>
#include <vector>

#include "cerata/vhdl/vhdl.h"
#include "cerata/dot/dot.h"

#include "fletchgen/mantle.h"

#include "test_schemas.h"
#include "test_bus.h"

namespace fletchgen {

TEST(Mantle, Big) {
  auto schema = fletcher::test::GetBigSchema();
  auto set = SchemaSet::Make("Big", {schema});
  auto top = Mantle::Make(set);

  auto design = cerata::vhdl::Design(top);
  std::cout << design.Generate().ToString();
  cerata::dot::Grapher dot;
  std::cout << dot.GenFile(top, "graph.dot");
}

TEST(Mantle, StringRead) {
  auto schema = fletcher::test::GetStringReadSchema();
  auto set = SchemaSet::Make("StringRead", {schema});
  auto top = Mantle::Make(set);

  auto design = cerata::vhdl::Design(top);
  std::cout << design.Generate().ToString();
  cerata::dot::Grapher dot;
  std::cout << dot.GenFile(top, "graph.dot");
}

}
