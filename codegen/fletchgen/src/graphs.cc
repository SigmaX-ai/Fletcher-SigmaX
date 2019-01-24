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

#include "graphs.h"
#include "./edges.h"
#include "graphs.h"

namespace fletchgen {

std::shared_ptr<Component> Component::Make(std::string name,
                                           std::initializer_list<std::shared_ptr<Parameter>> parameters,
                                           std::initializer_list<std::shared_ptr<Port>> ports,
                                           std::initializer_list<std::shared_ptr<Signal>> signals) {
  auto ret = std::make_shared<Component>(name);
  for (const auto &param : parameters) {
    ret->Add(param);
    // If the parameter node has edges
    if (!param->ins.empty()) {
      // It has been assigned
      std::shared_ptr<Node> val = param->in(0)->src;
      // Add the value to the node list
      ret->Add(val);
    } else if (param->default_value != nullptr) {
      // Otherwise assign default value if any
      param <<= param->default_value;
      ret->Add(param->default_value);
    }
  }
  for (const auto &port : ports) {
    ret->Add(port);
  }
  for (const auto &signal : signals) {
    ret->Add(signal);
  }
  return ret;
}

Graph &Graph::Add(std::shared_ptr<Node> node) {
  nodes.push_back(node);
  node->parent = this;
  return *this;
}

std::shared_ptr<Node> Graph::Get(Node::ID id, std::string name) const {
  for (const auto &n : nodes) {
    if ((n->name() == name) && (n->id == id)) {
      return n;
    }
  }
  return nullptr;
}

Graph &Graph::Add(const std::shared_ptr<Graph> &child) {
  children.push_back(child);
  return *this;
}

size_t Graph::CountNodes(Node::ID id) const {
  size_t count = 0;
  for (const auto &n : nodes) {
    if (n->id == id) {
      count++;
    }
  }
  return count;
}

std::shared_ptr<Instance> Instance::Make(std::string name, std::shared_ptr<Component> component) {
  return std::make_shared<Instance>(name, component);
}

std::shared_ptr<Instance> Instance::Make(std::shared_ptr<Component> component) {
  return std::make_shared<Instance>(component->name() + "_inst", component);
}

Instance::Instance(std::string name, std::shared_ptr<Component> comp)
    : Graph(std::move(name)), component(std::move(comp)) {
  // Make copies of ports and parameters
  for (const auto& port : component->GetAll<Port>()) {
    Add(port->Copy());
  }
  for (const auto& par : component->GetAll<Parameter>()) {
    Add(par->Copy());
  }
}

Graph &Instance::Add(std::shared_ptr<Node> node) {
  if (!node->Is(Node::SIGNAL)) {
    nodes.push_back(node);
    node->parent = this;
    return *this;
  } else {
    throw std::runtime_error("Cannot add signal nodes to Instance graph " + name());
  }
}

}  // namespace fletchgen
