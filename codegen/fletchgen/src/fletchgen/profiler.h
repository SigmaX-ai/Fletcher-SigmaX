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

/// @brief Key for the kv-metadata annotation of stream nodes.
constexpr char PROFILE_KEY[] = "fletchgen_profile";

/// @brief Returns a stream probe type.
std::shared_ptr<cerata::Type> stream_probe();

/// @brief Returns the profiler component.
std::shared_ptr<cerata::Component> Profiler(const std::shared_ptr<cerata::ClockDomain> &domain = kernel_domain());

/**
 * @brief Transforms a Cerata component graph to include stream profilers for selected nodes.
 *
 * To select a node for profiling, the node must be of type cerata::Stream and it must have the PROFILE_KEY set to true
 * in its kv-metadata.
 *
 * Currently doesn't make a deep copy, so it modifies the existing structure irreversibly.
 *
 * @param top The top-level component to apply this to.
 * @return    A transformed graph.
 */
cerata::Component *EnableStreamProfiling(cerata::Component *top);

}  // namespace fletchgen
