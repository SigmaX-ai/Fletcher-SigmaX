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

#include "fletchgen/hls/vivado.h"

#include <vector>
#include <memory>
#include <string>

#include "fletchgen/schema.h"
#include "fletchgen/kernel.h"

std::string fletchgen::hls::GenerateVivadoHLSTemplate(const Kernel &kernel) {
  std::stringstream str;

  std::vector<Argument> args;

  auto ports = kernel.GetAll<FieldPort>();

  for (const auto &p : ports) {
    Argument arg;
    arg.name = p->name();
    for (const auto &f : p->field_->Flatten()) {
      std::cout << f->ToString() << std::endl;
    }
  }

  return str.str();
}
