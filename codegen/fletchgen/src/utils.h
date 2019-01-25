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

#include <string>
#include <memory>
#include <utility>
#include <deque>
#include <algorithm>

namespace fletchgen {

struct Named {
 public:
  explicit Named(std::string name)
      : name_(std::move(name)) {}
  std::string name() const { return name_; }
  void SetName(std::string name) { name_ = std::move(name); }
 private:
  std::string name_;
};

template<typename T>
bool contains(const std::deque<std::shared_ptr<T>>& list, const std::shared_ptr<T>& item) {
  return std::find(std::begin(list), std::end(list), item) != std::end(list);
}

template<typename T>
bool contains(const std::deque<T*>& list, T* item) {
  return std::find(std::begin(list), std::end(list), item) != std::end(list);
}

template<typename T>
std::deque<std::shared_ptr<T>>* remove(std::deque<std::shared_ptr<T>>* list, const std::shared_ptr<T>& item) {
  auto it = std::find(std::begin(*list), std::end(*list), item);
  if (it != std::end(*list)) {
    list->erase(it);
  }
  return list;
}

}  // namespace fletchgen
