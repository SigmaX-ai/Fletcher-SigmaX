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

#include <memory>

#include "../src/types.h"
#include "../src/nodes.h"
#include "../src/edges.h"
#include "../src/graphs.h"

#include "../src/hardware/basic_types.h"

namespace fletchgen {

std::shared_ptr<Component> GetArrayComponent() {
  auto size = Parameter::Make("size", integer(), intl<0>());
  auto data = Vector::Make<8>();
  auto pA = ArrayPort::Make("A", data, size, Port::OUT);
  auto pB = Port::Make("B", data, Port::IN);
  auto pC = Port::Make("C", data, Port::IN);

  auto top = Component::Make("top", {}, {}, {});
  auto x_comp = Component::Make("X", {size}, {pA}, {});
  auto y_comp = Component::Make("Y", {}, {pB, pC}, {});

  auto x = Instance::Make(x_comp);
  auto y = Instance::Make(y_comp);

  top->AddChild(x)
      .AddChild(y);

  y->p("B") <<= x->ap("A");
  y->p("C") <<= x->ap("A");

  return top;
}

std::shared_ptr<Component> GetTypeConvComponent() {

  auto t_wide = Vector::Make<4>();
  auto t_narrow = Vector::Make<2>();
  // Flat indices:
  auto tA = Record::Make("rec_A", {       // 0
      RecordField::Make("q", t_wide),     // 1
      RecordField::Make("r", t_narrow),   // 2
      RecordField::Make("s", t_narrow),   // 3
      RecordField::Make("t", t_wide),     // 4
  });

  auto tB = Record::Make("rec_B", {       // 0
      RecordField::Make("u", t_wide),     // 1
      RecordField::Make("v", t_narrow),   // 2
      RecordField::Make("w", t_narrow),   // 3
      RecordField::Make("x", t_wide),     // 4
  });

  // Create a type mapping from tA to tE
  auto mapper = std::make_shared<TypeMapper>(tA.get(), tB.get());
  mapper->Add(0, 0);
  mapper->Add(1, 2).Add(1, 3);
  mapper->Add(3, 1);
  mapper->Add(2, 1);
  mapper->Add(4, 4);
  tA->AddMapper(mapper);

  // Ports
  auto pA = Port::Make("A", tA, Port::OUT);
  auto pB = Port::Make("B", tB, Port::IN);

  // Components and instantiations
  auto top = Component::Make("top", {}, {}, {});
  auto x_comp = Component::Make("X", {}, {pA}, {});
  auto y_comp = Component::Make("Y", {}, {pB}, {});
  auto x = Instance::Make(x_comp);
  auto y = Instance::Make(y_comp);
  top->AddChild(x)
      .AddChild(y);

  // Connect ports
  y->p("B") <<= x->p("A");

  return top;
}

std::shared_ptr<Component> GetAllPortTypesComponent() {
  auto r_type = Record::Make("rec", {
      RecordField::Make("a", Vector::Make<8>()),
      RecordField::Make("b", Vector::Make<32>())
  });
  auto s_type = Stream::Make("stream", Vector::Make<16>());

  auto clk_domain = ClockDomain::Make("domain0");
  auto clk_port = Port::Make(Clock::Make("clk", clk_domain));
  auto rst_port = Port::Make(Reset::Make("reset", clk_domain));
  auto b_port = Port::Make("some_bool", boolean(), Port::OUT);
  auto v_port = Port::Make("some_vector", Vector::Make<64>());
  auto r_port = Port::Make("some_record", r_type, Port::OUT);
  auto s_port = Port::Make("some_port", s_type);

  auto l = intl<16>();
  auto par = Parameter::Make("depth", integer(), l);

  auto a_type = Component::Make("a", {par}, {clk_port, rst_port, b_port, v_port, r_port, s_port}, {});

  return a_type;
}

}
