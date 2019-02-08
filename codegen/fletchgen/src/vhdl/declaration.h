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

#include <algorithm>
#include <string>
#include <memory>

#include "../nodes.h"
#include "../types.h"
#include "../graphs.h"

#include "./block.h"

namespace fletchgen {
namespace vhdl {

struct Decl {
  static std::string Generate(const std::shared_ptr<Type> &type);
  static Block Generate(const std::shared_ptr<Parameter> &par, int depth = 0);
  static Block Generate(const std::shared_ptr<Port> &port, int depth = 0);
  static Block Generate(const std::shared_ptr<ArrayPort> &port, int depth = 0);
  static Block Generate(const std::shared_ptr<Signal> &sig, int depth = 0);
  static MultiBlock Generate(const std::shared_ptr<Component> &comp, bool entity = false);
};

}  // namespace vhdl
}  // namespace fletchgen
