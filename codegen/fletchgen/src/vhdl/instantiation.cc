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

#include "./instantiation.h"

#include "../edges.h"

#include "./flatnode.h"
#include "./vhdl_types.h"

namespace fletchgen {
namespace vhdl {

static std::shared_ptr<Node> GetWidth(const std::shared_ptr<Type> &type) {
  switch (type->id()) {
    case Type::VECTOR:return Cast<Vector>(type)->width();
    case Type::BIT:return litint<1>();
    case Type::CLOCK:return litint<1>();
    case Type::RESET:return litint<1>();
    case Type::BOOLEAN:return litint<1>();
    default:return litint<0>();
  }
}

static std::shared_ptr<Node> GetOffsetOf(const std::shared_ptr<Edge> &edge,
                                         const std::shared_ptr<Node> &node,
                                         size_t tuple_index) {
  // Get the siblings
  auto siblings = edge->GetAllSiblings(node);
  // Get this edge index amongst its siblings on the node in question
  auto edge_idx = Edge::GetIndexOf(edge, node);

  // Initial offset is 0
  std::shared_ptr<Node> offset = litint<0>();

  for (size_t i = 0; i < edge_idx; i++) {
    // Check the siblings other side node width at the right tuple index
    auto fn = FlatNode(siblings[i]->GetOtherNode(node));
    auto other_tuple = fn.GetTuple(i);
    auto other_type = std::get<1>(other_tuple);
    offset = offset + GetWidth(other_type);
  }

  return offset;
}

Block Inst::Generate(const std::shared_ptr<Port> &lhs) {
  Block ret;

  // Flatten left hand side node to VHDL objects
  auto fl = FlatNode(lhs);

  // Iterate over every edge of this port.
  for (const auto &e : lhs->edges()) {
    // Get the other end
    auto rhs = e->GetOtherNode(lhs);

    // Check if other end is compatible
    if (IsCompatible(lhs, rhs)) {

      // Flatten right hand side node to VHDL objects
      auto fr = FlatNode(rhs);

      std::shared_ptr<Node> lhs_off = litint<0>();
      std::shared_ptr<Node> rhs_off = litint<0>();

      // Iterate over the flattened vhdl objects
      for (size_t i = 0; i < fl.size(); i++) {
        Line line;
        // LHS properties
        auto lhs_tuple = fl.GetTuple(i);
        auto lhs_tup_id = std::get<0>(lhs_tuple).ToString();
        auto lhs_tup_width = GetWidth(std::get<1>(lhs_tuple));

        line << lhs_tup_id;

        // Check if LHS indexing is applicable
        if (e->num_siblings(lhs) > 1) {
          line += "(";
          line += (lhs_off + lhs_tup_width - litint<1>())->ToString();
          line += " downto ";
          line += lhs_off->ToString();
          line += ")";
          lhs_off = (lhs_off + lhs_tup_width);
        }

        line << " => ";

        // RHS properties
        auto rhs_tuple = fr.GetTuple(i);
        auto rhs_tup_id = std::get<0>(rhs_tuple).ToString();
        auto rhs_tup_width = GetWidth(std::get<1>(rhs_tuple));

        line << rhs_tup_id;

        // Check if RHS indexing is applicable
        if (e->num_siblings(rhs) > 1) {
          rhs_off = GetOffsetOf(e, rhs, i);
          line += "(";
          line += (rhs_off + rhs_tup_width - litint<1>())->ToString();
          line += " downto ";
          line += rhs_off->ToString();
          line += ")";
          rhs_off = (rhs_off + rhs_tup_width);
        }

        ret << line;
      }
    }
  }
  return ret;
}

MultiBlock Inst::Generate(const std::shared_ptr<Graph> &graph) {
  MultiBlock ret;

  if (!Cast<Instance>(graph)) {
    throw std::runtime_error("Graph is not an instance.");
  }

  auto inst = *Cast<Instance>(graph);

  // Instantiation header
  Block lh;
  lh << inst->name() + " : " + inst->component->name();
  ret << lh;

  // Generic map
  if (inst->CountNodes(Node::PARAMETER) > 0) {
    Block gmh(ret.indent + 1);
    Block gm(ret.indent + 2);
    Block gmf(ret.indent + 1);
    Line gh, gf;
    gh << "generic map (";
    gmh << gh;
    for (const auto &g : inst->GetAllNodesOfType<Parameter>()) {
      gm << Generate(g);
    }
    gf << ")";
    gmf << gf;
    ret << gmh << gm << gmf;
  }

  if (inst->CountNodes(Node::PORT) > 0) {
    // Port map
    Block pmh(ret.indent + 1);
    Block pmb(ret.indent + 2);
    Block pmf(ret.indent + 1);
    Line ph, pf;
    ph << "port map (";
    pmh << ph;
    for (const auto &p : inst->GetAllNodesOfType<Port>()) {
      Block pm;
      pm << Generate(p);
      pmb << pm;
    }
    pf << ")";
    pmf << pf;
    ret << pmh << pmb << pmf;
  }

  return ret;
}

Block Inst::Generate(const std::shared_ptr<Parameter> &port) {
  return Block();
}

}  // namespace vhdl
}  // namespace fletchgen
