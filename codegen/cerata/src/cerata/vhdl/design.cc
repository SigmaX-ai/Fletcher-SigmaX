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

#include "cerata/vhdl/design.h"

#include <string>
#include <memory>

#include "cerata/logging.h"
#include "cerata/graphs.h"
#include "cerata/vhdl/resolve.h"
#include "cerata/vhdl/declaration.h"
#include "cerata/vhdl/architecture.h"

namespace cerata::vhdl {

MultiBlock Design::Generate() {
  MultiBlock ret;

  // TODO(johanpel): when proper copy is in place, make a copy of the whole structure before sanitizing,
  // in case multiple back ends are processing the graph. This currently modifies the original structure.

  // Resolve VHDL specific problems
  LOG(DEBUG, "VHDL: Transforming Cerata graph to VHDL-compatible.");
  Resolve::ResolvePortToPort(comp);
  Resolve::ExpandStreams(comp);

  // Place header
  if (!head.empty()) {
    Block h;
    h << Line(head);
    ret << h;
  }

  // Figure out potential libs and packages used for primitive components.
  std::unordered_map<std::string, std::vector<std::string>> libs_and_packages;
  for (const auto &c: GetAllUniqueComponents(comp.get())) {
    if (c->meta.count("primitive") > 0) {
      if (c->meta.at("primitive") == "true") {
        std::string lib = c->meta.at("library");
        std::string pkg = c->meta.at("package");
        // Create the kv-pair if there is none yet
        if (libs_and_packages.count(lib) == 0) {
          libs_and_packages[lib] = std::vector<std::string>({pkg});
        } else {
          // If the package has not been added already, add it
          if (std::find(std::begin(libs_and_packages[lib]), std::end(libs_and_packages[lib]), pkg)
              != std::end(libs_and_packages[lib])) {
            libs_and_packages[lib].push_back(pkg);
          }
        }
      }
    }
  }

  Block incl;
  for (const auto &kv : libs_and_packages) {
    incl << Line("library " + kv.first + ";");
    for (const auto &pkg : kv.second) {
      // TODO(johanpel): consider also allowing non-all.
      incl << Line("use " + kv.first + "." + pkg + ".all;");
    }
  }

  ret << incl;

  // Place any libraries from subcomponents

  auto decl = Decl::Generate(comp.get(), true);
  auto arch = Arch::Generate(comp);

  ret << decl;
  ret << arch;

  return ret;
}

}  // namespace cerata::vhdl
