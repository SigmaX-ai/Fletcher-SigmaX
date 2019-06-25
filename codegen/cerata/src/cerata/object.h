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

#include <utility>
#include <string>
#include <memory>
#include <unordered_map>

#include "cerata/utils.h"

namespace cerata {

class Node;
class NodeArray;
class PortArray;
struct Port;
class Graph;

class Object : public Named {
 public:
  enum ID {
    NODE,
    ARRAY
  };
  explicit Object(std::string name, ID id) : Named(std::move(name)), obj_id_(id) {}
  virtual ~Object() = default;

  /// @brief Return the object ID of this object.
  ID obj_id() const { return obj_id_; }
  /// @brief Return true if this object is a node.
  bool IsNode() const { return obj_id_ == NODE; }
  /// @brief Return true if this object is an array.
  bool IsArray() const { return obj_id_ == ARRAY; }

  virtual void SetParent(Graph *parent);
  virtual std::optional<Graph *> parent() const;
  virtual std::shared_ptr<Object> Copy() const = 0;

  /// @brief KV storage for metadata of tools or specific backend implementations
  std::unordered_map<std::string, std::string> meta;

 protected:
  ID obj_id_;
  /// An optional parent Graph to which this Object belongs. Initially no value.
  std::optional<Graph *> parent_ = {};
};

}  // namespace cerata
