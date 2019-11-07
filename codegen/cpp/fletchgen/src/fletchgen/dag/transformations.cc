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

#include "fletchgen/dag/transformations.h"

#include <memory>

#include "fletchgen/dag/composer.h"

namespace fletchgen::dag {

Transform Sum(const PrimRef &type) {
  Transform result;
  result.name = "Sum";
  result += in("in", list(type));
  result += out("out", type);
  return result;
}

Transform SplitByRegex(const std::string &regex) {
  static Transform result;
  result.name = "SplitByRegex";
  result.constants.push_back(constant("expr", regex));
  result += in("in", list(utf8()));
  result += out("out", list(utf8()));
  return result;
}

Transform Zip(const std::vector<TypeRef> &inputs) {
  Transform result;
  result.name = "Zip";
  std::vector<FieldRef> fields;
  for (size_t i = 0; i < inputs.size(); i++) {
    result += in("in_" + std::to_string(i), inputs[i]);
    fields.push_back(field(std::to_string(i), inputs[i]));
  }
  result += out("out", struct_("zipped", fields));
  return result;
}

Transform Zip(const ListRef &t0, const PrimRef &t1) {
  Transform result;
  result.name = "Zip";
  result += in("in_0", t0);
  result += in("in_1", t1);
  result += out("out", list(struct_({field("out_0", t0->item->type),
                                     field("out_1", t1)})));
  return result;
}

Transform IndexOfComparison(const ListRef &list_type, const std::string &op) {
  Transform result;
  result.name = "IndexOfComparison";
  result.constants.push_back(constant("op", op));
  result += in("in", list_type);
  result += in("val", list_type->item->type);
  result += out("out", idx32());
  return result;
}

Transform Select(const ListRef &t) {
  Transform result;
  result.name = "Select";
  result += in("index", idx32());
  result += in("in", t);
  result += out("out", t);
  return result;
}

Transform Sort(const ListRef &list_type) {
  Transform result;
  result.name = "Sort";
  result += in("in", list_type);
  result += out("out", list_type);
  return result;
}

Transform SortBy(const Struct &input, size_t field_idx) {
  Transform result;
  result.name = "SortBy";
  int i = 0;
  result += constant("column", std::to_string(field_idx));
  for (const auto &f : input.fields) {
    result += in("in_" + std::to_string(i), f->type);
    result += out("out_" + std::to_string(i), f->type);
    i++;
  }
  return result;
}

}  // namespace fletchgen::dag