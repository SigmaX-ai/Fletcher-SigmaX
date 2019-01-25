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

#include "./vhdl.h"

#include <algorithm>
#include <vector>
#include <string>
#include <memory>

#include "../stream.h"
#include "../nodes.h"
#include "../types.h"
#include "../graphs.h"

namespace fletchgen {
namespace vhdl {

using fletchgen::Port;
using fletchgen::Type;
using fletchgen::Vector;
using fletchgen::Record;
using fletchgen::Parameter;
using fletchgen::Component;
using fletchgen::Literal;
using fletchgen::Stream;

using std::to_string;

std::shared_ptr<Type> valid() {
  static std::shared_ptr<Type> result = std::make_shared<Bit>("valid");
  return result;
}

std::shared_ptr<Type> ready() {
  static std::shared_ptr<Type> result = std::make_shared<Bit>("ready");
  return result;
}

std::string ToString(Port::Dir dir) {
  if (dir == Port::Dir::IN) {
    return "in";
  } else {
    return "out";
  }
}

Port::Dir Reverse(Port::Dir dir) {
  if (dir == Port::Dir::IN) {
    return Port::Dir::OUT;
  } else {
    return Port::Dir::IN;
  }
}

std::string ToString(std::vector<Block> blocks) {
  std::stringstream ret;
  for (const auto &b : blocks) {
    ret << b.str();
  }
  return ret.str();
}

std::string ToString(std::shared_ptr<Node> node) {
  if (node->IsLiteral()) {
    auto x = *Cast<Literal>(node);
    return x->ToValueString();
  } else if (node->IsParameter()) {
    auto x = *Cast<Parameter>(node);
    return x->name();
  } else {
    return node->name();
  }
}

std::string Declarator::Generate(const std::shared_ptr<Type> &type) {
  if (type->Is(Type::CLOCK)) {
    return "std_logic";
  } else if (type->Is(Type::RESET)) {
    return "std_logic";
  } else if (type->Is(Type::BIT)) {
    return "std_logic";
  } else if (type->Is(Type::VECTOR)) {
    auto vec = Cast<Vector>(type);
    return "std_logic_vector(" + ToString(vec->width()) + "-1 downto 0)";
  } else if (type->Is(Type::RECORD)) {
    auto r = Cast<Record>(type);
    return r->name();
  } else if (type->Is(Type::NATURAL)) {
    return "natural";
  } else if (type->Is(Type::STREAM)) {
    return Generate(Cast<Stream>(type)->element_type());
  } else if (type->Is(Type::STRING)) {
    return "string";
  } else if (type->Is(Type::BOOLEAN)) {
    return "boolean";
  } else {
    return "FLETCHGEN_INVALID_TYPE";
  }
}

Block Declarator::Generate(const std::shared_ptr<Parameter> &par, int depth) {
  Block ret(depth);
  Line l;
  l << par->name() << " : " << Generate(par->type);
  ret << l;
  return ret;
}

Block Declarator::Generate(const std::shared_ptr<Type> &type, const std::shared_ptr<Node> &parent, int depth) {
  if (type->Is(Type::RECORD)) {
    Block ret(depth);
    ret << Generate(Cast<Record>(type), parent, depth);
    return ret;
  } else if (type->Is(Type::STREAM)) {
    Block ret(depth);
    ret << Generate(Cast<Stream>(type), parent, depth);
    return ret;
  } else {
    Block ret(depth);
    Line d;
    d << " : ";
    auto port = Cast<Port>(parent);
    if (port) {
      d << ToString((*port)->dir);
    }
    d << " " + Generate(type);
    ret << d;
    return ret;
  }
}

Block Declarator::Generate(const std::shared_ptr<Stream> &stream, const std::shared_ptr<Node> &parent, int depth) {
  Block ret;
  Block d;
  Line v, r;
  auto port = Cast<Port>(parent);

  v << "valid" << " : ";
  r << "ready" << " : ";
  if (port) {
    v << ToString((*port)->dir);
    r << ToString(Reverse((*port)->dir));
  }
  v << " " + Generate(valid());
  r << " " + Generate(ready());

  d << Generate(stream->element_type(), parent, depth);
  ret << v << r;
  ret << Prepend(stream->element_name(), &d, "");
  return ret;
}

Block Declarator::Generate(const std::shared_ptr<Record> &rec, const std::shared_ptr<Node> &parent, int depth) {
  Block ret;
  for (const auto &f : rec->fields()) {
    Block fb = Generate(f->type(), parent, depth);
    ret << Prepend(f->name(), &fb);
  }
  return ret;
}

Block Declarator::Generate(const std::shared_ptr<Node> &port, int depth) {
  Block p = Generate(port->type, port, depth);
  return Prepend(port->name(), &p);
}

MultiBlock Declarator::Generate(const std::shared_ptr<Graph> &graph, bool entity) {
  MultiBlock ret;

  // Header
  Block h(ret.indent), f(ret.indent);
  Line hl, fl;
  if (entity) {
    hl << "entity " + graph->name();
  } else {
    hl << "component " + graph->name();
  }
  h << hl;
  ret << h;

  // Generics
  auto generics = graph->GetAllNodesOfType<Parameter>();
  if (!generics.empty()) {
    Block gdh(ret.indent + 1);
    Block gd(ret.indent + 2);
    Block gdf(ret.indent + 1);
    Line gh, gf;
    gh << "generic (";
    gdh << gh;
    for (const auto &gen : generics) {
      auto g = Declarator::Generate(gen, ret.indent + 2);
      if (gen != generics.back()) {
        g << ";";
      } else {
        g <<= ";";
      }
      gd << g;
    }
    gf << ");";
    gdf << gf;
    ret << gdh << gd << gdf;
  }
  auto ports = graph->GetAllNodesOfType<Port>();
  if (!ports.empty()) {
    Block pdh(ret.indent + 1);
    Block pd(ret.indent + 2);
    Block pdf(ret.indent + 1);
    Line ph, pf;
    ph << "port (";
    pdh << ph;
    for (const auto &port : ports) {
      auto g = Declarator::Generate(port, ret.indent + 2);
      if (port != ports.back()) {
        g << ";";
      } else {
        g <<= ";";
      }
      pd << g;
    }
    pf << ");";
    pdf << pf;
    ret << pdh << pd << pdf;
  }

  fl << "end component;";
  f << fl;

  ret << f;

  return ret;

}

Block Instantiator::GenConcatenatedStream(std::shared_ptr<Node> left,
                                          std::shared_ptr<Node> right,
                                          std::shared_ptr<Edge> edge) {
  Block block(1);
  Line d;
  Line v;
  Line r;

  auto leftidx = Edge::GetIndexOf(edge, left);
  auto rightidx = Edge::GetIndexOf(edge, right);

  auto leftoff = Edge::GetVectorOffsetOf(edge, left);
  auto rightoff = Edge::GetVectorOffsetOf(edge, right);

  size_t right_width = 0;

  auto rstream = Cast<Stream>(right->type);
  if (rstream->element_type()->Is(Type::VECTOR)) {
    auto vec = Cast<Vector>(rstream->element_type());
    //right_width = vec->width();
  }

  if (edge->HasSiblings(left)) {
    d << left->name() + "(" + to_string(leftoff + right_width - 1) + " downto " + to_string(leftoff) + ")";
    v << left->name() + "_valid(" + to_string(leftidx) + ")";
    r << left->name() + "_ready(" + to_string(leftidx) + ")";
  } else {
    d << left->name();
    v << left->name() + "_valid";
    r << left->name() + "_ready";
  }

  d << " => ";
  v << " => ";
  r << " => ";

  if (edge->HasSiblings(right)) {
    d << right->name() + "(" + to_string(rightoff + right_width - 1) + " downto " + to_string(rightoff) + "),";
    v << right->name() + "_valid(" + to_string(rightidx) + "),";
    r << right->name() + "_ready(" + to_string(rightidx) + "),";
  } else {
    d << right->name() + ",";
    v << right->name() + "_valid,";
    r << right->name() + "_ready,";
  }

  d << " -- Edge: " + edge->name();

  block << v << r << d;
  return block;
}

Block Instantiator::Generate(std::shared_ptr<Node> left,
                             std::shared_ptr<Node> right,
                             std::shared_ptr<Edge> edge) {
  Block ret;

  if (edge->GetOtherNode(left)) {
    if (left->type->Is(Type::STREAM) && right->type->Is(Type::STREAM)) {
      ret << GenConcatenatedStream(left, right, edge);
    }
  } else {
    Block b;
    Line c;
    c << left->name() << " => " << right->name();
    b << c;
    ret << b;
  }
  return ret;
}

MultiBlock Instantiator::Generate(const std::shared_ptr<Graph> &graph) {
  MultiBlock ret;

  // Instantiation header
  Block lh;
  lh << graph->name() + " : " + graph->name();
  ret << lh;

  // Generic map
  if (graph->CountNodes(Node::PARAMETER) > 0) {
    Block gmh(ret.indent + 1);
    Block gm(ret.indent + 2);
    Block gmf(ret.indent + 1);
    Line gh, gf;
    gh << "generic map (";
    gmh << gh;
    for (const auto &n : graph->nodes) {
      Block g;
      if (n->IsParameter()) {
        g << Generate(n);
      }
      gm << g;
    }
    gf << ")";
    gmf << gf;
    ret << gmh << gm << gmf;
  }

  if (graph->CountNodes(Node::PORT) > 0) {
    // Port map
    Block pmh(ret.indent + 1);
    Block pm(ret.indent + 2);
    Block pmf(ret.indent + 1);
    Line ph, pf;
    ph << "port map (";
    pmh << ph;
    for (const auto &n : graph->nodes) {
      Block p;
      if (n->IsPort()) {
        p << Generate(n);
      }
      pm << p;
    }
    pf << ")";
    pmf << pf;
    ret << pmh << pm << pmf;
  }

  return ret;
}

Block Instantiator::Generate(const std::shared_ptr<Node> &node) {
  Block ret;

  if (node->id == Node::PARAMETER) {
    // Parameter nodes must be connected to a literal
    Line l;
    l << node->name();
    if (!node->edges().empty()) {
      for (const auto &e : node->edges()) {
        auto other = e->GetOtherNode(node);
        if (other->IsLiteral()) {
          l << " => " << std::dynamic_pointer_cast<Literal>(other)->ToValueString() << ",";
        } else {
          throw std::runtime_error("Parameter node should be attached to literal node.");
        }
      }
    }
    ret << l;
  } else if (node->IsPort()) {
    // Port nodes must be connected to signals or other port nodes
    if (!node->edges().empty()) {
      for (const auto &e : node->edges()) {
        auto other = e->GetOtherNode(node);
        ret << Generate(node, other, e);
      }
    }
  }

  return ret;
}

}
}
