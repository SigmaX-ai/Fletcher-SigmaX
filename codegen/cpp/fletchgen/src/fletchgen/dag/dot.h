// Copyright 2018-2019 Delft University of Technology
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

#include <string>

#include "fletchgen/dag/composer.h"

namespace fletchgen::dag {

std::string Name(const Vertex &v);
std::string Name(const Edge &e);
std::string Name(const Transform &t);

std::string Label(const Vertex &v);
std::string Label(const Edge &e);
std::string Label(const Transform &t);

std::string Style(const Vertex &v);
std::string Style(const Edge &e);
std::string Style(const Transform &t);

std::string AsDotGraph(const Graph &g);

}  // namespace fletchgen::dag
