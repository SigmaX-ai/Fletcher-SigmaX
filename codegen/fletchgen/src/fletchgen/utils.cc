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

#include "fletchgen/utils.h"

#include <cstdlib> // system()
#include <fletcher/common/api.h>
#include <cerata/api.h>

std::string GetProgramName(char *argv0) {
  auto arg = std::string(argv0);
  size_t pos = arg.rfind('\\');
  if (pos != std::string::npos) {
    return arg.substr(pos + 1);
  } else {
    return "fletchgen";
  }
}

cerata::Port::Dir mode2dir(fletcher::Mode mode) {
  if (mode == fletcher::Mode::READ) {
    return cerata::Port::Dir::IN;
  } else {
    return cerata::Port::Dir::OUT;
  }
}
