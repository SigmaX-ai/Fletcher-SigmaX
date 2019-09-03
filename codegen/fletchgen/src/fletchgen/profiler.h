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

#include <cerata/api.h>

#include "fletchgen/basic_types.h"

#include <utility>
#include <string>
#include <deque>
#include <memory>

namespace fletchgen {

std::shared_ptr<cerata::Type> stream_probe();
std::shared_ptr<cerata::Component> Profiler(const std::shared_ptr<cerata::ClockDomain> &domain = kernel_domain());

constexpr char PROFILE_KEY[] = "fletchgen_profile";

void AttachStreamProfilers(cerata::Component *comp, cerata::Port *cr, const std::shared_ptr<cerata::ClockDomain>& cd);

}  // namespace fletchgen
