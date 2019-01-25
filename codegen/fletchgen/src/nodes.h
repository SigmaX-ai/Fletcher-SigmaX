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

#include <optional>
#include <string>
#include <memory>
#include <deque>
#include <utility>

#include "./types.h"

namespace fletchgen {

// Forward Declr.
struct Edge;
struct Graph;

/**
 * @brief A node.
 */
struct Node : public Named {
  /// @brief Node type IDs
  enum ID {
    PORT,
    SIGNAL,
    PARAMETER,
    LITERAL
  };
  /// @brief Node type ID
  ID id;

  /// @brief The Type of this Node.
  std::shared_ptr<Type> type;

  /// @brief All incoming Edges of this Node.
  std::deque<std::shared_ptr<Edge>> ins;

  /// @brief All outgoing Edges of this Node.
  std::deque<std::shared_ptr<Edge>> outs;

  /// @brief An optional parent Graph to which this Node belongs.
  std::optional<Graph *> parent;

  /// @brief Node constructor.
  Node(std::string name, ID id, std::shared_ptr<Type> type);
  virtual ~Node() = default;

  /// @brief Return all edges of this node, both in and out.
  std::deque<std::shared_ptr<Edge>> edges() const;

  /// @brief Return incoming Edge i
  std::shared_ptr<Edge> in(size_t i) { return ins[i]; }

  /// @brief Return outgoing Edge i
  std::shared_ptr<Edge> out(size_t i) { return outs[i]; }

  /// @brief Return whether this node is of a specific node type id.
  bool Is(ID tid) { return id == tid; }

  /// @brief Return true if this is a PORT node, false otherwise.
  bool IsPort() { return id == PORT; }

  /// @brief Return true if this is a SIGNAL node, false otherwise.
  bool IsSignal() { return id == SIGNAL; }

  /// @brief Return true if this is a PARAMETER node, false otherwise.
  bool IsParameter() { return id == PARAMETER; }

  /// @brief Return true if this is a LITERAL node, false otherwise.
  bool IsLiteral() { return id == LITERAL; }
};

/**
 * @brief A Signal Node
 *
 * Can be used to build up some Graph structure, e.g. to connect two Instance ports.
 *
 * TODO(johanpel): do we -really- need this or should an emitter figure this out? Smells like VHDL.
 */
struct Signal : public Node {
  /// @brief Signal constructor.
  Signal(std::string name, std::shared_ptr<Type> type);

  /// @brief Create a new Signal and return a smart pointer to it.
  static std::shared_ptr<Signal> Make(std::string name, const std::shared_ptr<Type> &type);

  /// @brief Create a new Signal and return a smart pointer to it. The Signal name is derived from the Type name.
  static std::shared_ptr<Signal> Make(const std::shared_ptr<Type> &type);
};

/**
 * @brief A Literal Node
 *
 * A literal node can be used to store some literal value. A literal node can, for example, be used for Vector Type
 * widths or it can be connected to a Parameter Node, to give the Parameter its value.
 */
struct Literal : public Node {
  /// @brief The actual storage type of the value.
  enum { INT, STRING, BOOL } val_storage_type;

  /// @brief The string storage.
  std::string str_val = "";

  /// @brief The integer storage.
  int int_val = 0;

  /// @brief The boolean storage.
  bool bool_val = false;

  /// @brief Construct a new Literal with a string storage type.
  Literal(std::string name, const std::shared_ptr<Type> &type, std::string value);

  /// @brief Construct a new Literal with an integer storage type.
  Literal(std::string name, const std::shared_ptr<Type> &type, int value);

  /// @brief Construct a new Literal with a boolean storage type.
  Literal(std::string name, const std::shared_ptr<Type> &type, bool value);

  /// @brief Get a smart pointer to a new Literal with string storage, where the Literal name will be the string.
  static std::shared_ptr<Literal> Make(const std::shared_ptr<Type> &type, std::string value);

  /// @brief Get a smart pointer to a new Literal with a string storage type.
  static std::shared_ptr<Literal> Make(std::string name, const std::shared_ptr<Type> &type, std::string value);

  /// @brief Get a smart pointer to a new Literal with an integer storage type.
  static std::shared_ptr<Literal> Make(std::string name, const std::shared_ptr<Type> &type, int value);

  /// @brief Get a smart pointer to a new Literal with a boolean storage type.
  static std::shared_ptr<Literal> Make(std::string name, const std::shared_ptr<Type> &type, bool value);

  /// @brief Convert the Literal value to a human-readable string.
  std::string ToValueString();
};

/**
 * @brief A Parameter node.
 *
 * Can be used to define implementation-specific characteristics of a Graph, or can be connected to e.g. Vector widths.
 */
struct Parameter : public Node {
  /// @brief An optional default value
  std::optional<std::shared_ptr<Literal>> default_value;

  /// @brief Construct a new Parameter, optionally defining a default value Literal.
  Parameter(std::string name,
            const std::shared_ptr<Type> &type,
            std::optional<std::shared_ptr<Literal>> default_value = {});

  /// @brief Get a smart pointer to a new Parameter, optionally defining a default value Literal.
  static std::shared_ptr<Parameter> Make(std::string name,
                                         std::shared_ptr<Type> type,
                                         std::optional<std::shared_ptr<Literal>> default_value = {});

  /// @brief Create a copy of this Parameter.
  std::shared_ptr<Parameter> Copy();
};

/**
 * @brief A Port node.
 *
 * Can be used to define Graph terminators. Port nodes enforce proper directionality of edges.
 */
struct Port : public Node {
  /// @brief Port direction
  enum Dir { IN, OUT } dir;

  /// @brief Construct a new Port.
  Port(std::string name, std::shared_ptr<Type> type, Dir dir);

  /// @brief Get a smart pointer to a new Port.
  static std::shared_ptr<Port> Make(std::string name, std::shared_ptr<Type> type, Dir dir = Dir::IN);;

  /// @brief Get a smart pointer to a new Port. The Port name is derived from the Type name.
  static std::shared_ptr<Port> Make(std::shared_ptr<Type> type, Dir dir = Dir::IN);;

  /// @brief Create a copy of this Port.
  std::shared_ptr<Port> Copy();

  /// @brief Return true if this Port is an input, false otherwise.
  bool IsInput() { return dir == IN; }

  /// @brief Return true if this Port is an output, false otherwise.
  bool IsOutput() { return dir == OUT; }
};

/**
 * @brief Cast a Node to some (typically) less generic Node type T
 * @tparam T    The new Node type.
 * @param obj   The Node to cast.
 * @return      Optionally, the Node casted to T, if successful.
 */
template<typename T>
std::optional<std::shared_ptr<T>> Cast(const std::shared_ptr<Node> &obj) {
  auto result = std::dynamic_pointer_cast<T>(obj);
  if (result != nullptr) {
    return result;
  } else {
    return {};
  }
}

/// @brief Convert a Node ID to a human-readable string
std::string ToString(Node::ID id);

// Some often used literals for convenience:
/**
 * @brief Create an integer Literal
 * @tparam V    The integer value
 * @return      A smart pointer to a literal node representing the value
 */
template<int V>
std::shared_ptr<Literal> litint() {
  static std::shared_ptr<Literal> result = std::make_shared<Literal>("lit_" + std::to_string(V), integer(), V);
  return result;
}

/**
 * @brief Create a string literal
 * @param str   The string value
 * @return      A smart pointer to a literal node representing the string
 */
std::shared_ptr<Literal> litstr(std::string str);

/// @brief Return a literal node representing a Boolean true
std::shared_ptr<Literal> bool_true();

/// @brief Return a literal node representing a Boolean false
std::shared_ptr<Literal> bool_false();

}  // namespace fletchgen
