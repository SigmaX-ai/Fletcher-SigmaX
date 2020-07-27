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

#include "fletchgen/array.h"

#include <cerata/api.h>
#include <fletcher/common.h>
#include <utility>
#include <memory>
#include <cmath>
#include <vector>
#include <string>

#include "fletchgen/bus.h"
#include "fletchgen/basic_types.h"

namespace fletchgen {

using cerata::vector;
using cerata::record;
using cerata::string;
using cerata::strl;
using cerata::boolean;
using cerata::booll;
using cerata::record;
using cerata::stream;
using cerata::integer;
using cerata::parameter;

using fletcher::Mode;

PARAM_FACTORY(index_width)
PARAM_FACTORY(tag_width)

// The width of the indices or Arrow.
// TODO(johanpel): support LARGE_LIST, LARGE_BINARY, LARGE_STRING, etc...
static const int ARROW_OFFSET_WIDTH = 32;

size_t GetCtrlBufferCount(const arrow::Field &field) {
  fletcher::FieldMetadata field_meta;
  fletcher::FieldAnalyzer fa(&field_meta);
  fa.Analyze(field);
  return field_meta.buffers.size();
}

uint32_t GetTagWidth(const arrow::Field &field) {
  return fletcher::GetUIntMeta(field, fletcher::meta::TAG_WIDTH, 1);
}

std::shared_ptr<Type> cmd_type(const std::shared_ptr<Node> &index_width,
                               const std::shared_ptr<Node> &tag_width,
                               const std::optional<std::shared_ptr<Node>> &ctrl_width) {
  // Create record
  auto data = record({field("firstIdx", vector(index_width)),
                      field("lastIdx", vector(index_width)),
                      field("tag", vector(tag_width))});
  // If we want the ctrl field to be visible on this type, create that as well. This field is used to pass addresses.
  // Depending on how advanced the developer is, we want to expose this or leave it out through the Nucleus layer.
  if (ctrl_width) {
    auto ctrl = field(vector("ctrl", *ctrl_width));
    data->AddField(ctrl, 2);
  }
  // Create the stream type.
  auto result = stream(data);
  return result;
}

std::shared_ptr<Type> unlock_type(const std::shared_ptr<Node> &tag_width) {
  std::shared_ptr<Type> unlock_stream = stream("tag", vector(tag_width));
  return unlock_stream;
}

std::shared_ptr<Type> array_reader_out(uint32_t num_streams, uint32_t full_width) {
  auto data_stream = stream("ar_out", "", record({field(data(full_width)),
                                                  field(dvalid(num_streams, true)),
                                                  field(last(num_streams, true))}),
                            {field("valid", vector(num_streams)),
                             field("ready", vector(num_streams))->Reverse()});
  return data_stream;
}

std::shared_ptr<Type> array_writer_in(uint32_t num_streams, uint32_t full_width) {
  auto data_stream = stream("aw_in", "", record({field(data(full_width)),
                                                 field(dvalid(num_streams, true)),
                                                 field(last(num_streams, true))}),
                            {field("valid", vector(num_streams)),
                             field("ready", vector(num_streams))->Reverse()});
  return data_stream;
}

std::shared_ptr<Type> array_reader_out(std::pair<uint32_t, uint32_t> spec) {
  return array_reader_out(spec.first,
                          spec.second);
}

std::shared_ptr<Type> array_writer_in(std::pair<uint32_t, uint32_t> spec) {
  return array_writer_in(spec.first,
                         spec.second);
}

static std::string ArrayName(fletcher::Mode mode) {
  return mode == Mode::READ ? "ArrayReader" : "ArrayWriter";
}

static std::string DataName(fletcher::Mode mode) {
  return mode == Mode::READ ? "out" : "in";
}

/**
 * @brief Return a Cerata component model of an Array(Reader/Writer).
 *
 * This model corresponds to either:
 *    [`hardware/arrays/ArrayReader.vhd`](https://github.com/johanpel/fletcher/blob/develop/hardware/arrays/ArrayReader.vhd)
 * or [`hardware/arrays/ArrayWriter.vhd`](https://github.com/johanpel/fletcher/blob/develop/hardware/arrays/ArrayWriter.vhd)
 * depending on the mode parameter.
 *
 * Changes to the implementation of this component in the HDL source must be reflected in the implementation of this
 * function.
 *
 * WARNING: Binding of the input/output data stream width generics is more arcane than what is good for most.
 *  As such, most widths are just bound to some integer literals rather than parameters.
 *  Any code instantiating this component should rebind the type themselves after figuring out their true width.
 *
 * @param mode        Whether this Array component must Read or Write.
 * @return            The component model.
 */
Component *array(Mode mode) {
  // Check if the component already exists.
  auto optional_existing = cerata::default_component_pool()->Get(ArrayName(mode));
  if (optional_existing) {
    return *optional_existing;
  }
  // Create anew component.
  auto result = cerata::component(ArrayName(mode));

  BusDimParams params(result);
  auto func = mode == Mode::READ ? BusFunction::READ : BusFunction::WRITE;
  BusSpecParams spec{params, func};

  auto iw = index_width();
  auto tw = tag_width();
  tw->SetName("CMD_TAG_WIDTH");

  result->Add({iw,
               parameter("CFG", std::string("")),
               parameter("CMD_TAG_ENABLE", true),
               tw});

  // Clocks and resets.
  auto bcd = port("bcd", cr(), Port::Dir::IN, bus_cd());
  auto kcd = port("kcd", cr(), Port::Dir::IN, kernel_cd());

  // Command port.
  auto cmd = port("cmd", cmd_type(iw, tw, strl("arcfg_ctrlWidth(CFG, BUS_ADDR_WIDTH)")), Port::Dir::IN, kernel_cd());

  // Unlock port.
  auto unlock = port("unl", unlock_type(tw), Port::Dir::OUT, kernel_cd());

  // Bus port.
  auto bus = bus_port("bus", Port::Dir::OUT, spec);

  // Arrow data port.
  auto type = mode == Mode::READ ? array_reader_out() : array_writer_in();
  auto dir = mode == Mode::READ ? Port::Dir::OUT : Port::Dir::IN;
  auto data = port(DataName(mode), type, dir, kernel_cd());

  // Insert ports
  result->Add({bcd, kcd, cmd, unlock, bus, data});

  result->SetMeta(cerata::vhdl::meta::PRIMITIVE, "true");
  result->SetMeta(cerata::vhdl::meta::LIBRARY, "work");
  result->SetMeta(cerata::vhdl::meta::PACKAGE, "Array_pkg");
  return result.get();
}

ConfigType GetConfigType(const arrow::DataType &type) {
  if (type.id() == arrow::Type::LIST) {
    // Detect listprim:
    // Elements must be non-nullable.
    if (!type.field(0)->nullable() && (GetConfigType(*type.field(0)->type()) == ConfigType::PRIM)) {
      return ConfigType::LIST_PRIM;
    } else { // otherwise it's just a normal list
      return ConfigType::LIST;
    }
  }
  // listprim(8) types:
  if (type.id() == arrow::Type::BINARY) return ConfigType::LIST_PRIM;
  if (type.id() == arrow::Type::STRING) return ConfigType::LIST_PRIM;

  // Structs
  if (type.id() == arrow::Type::STRUCT) return ConfigType::STRUCT;

  // Anything else should be a primitive.
  return ConfigType::PRIM;
}

std::shared_ptr<Node> GetWidthNode(const arrow::DataType &type) {
  switch (type.id()) {
    // Fixed-width:
    case arrow::Type::BOOL: return intl(1);
    case arrow::Type::DATE32: return intl(32);
    case arrow::Type::DATE64: return intl(64);
    case arrow::Type::DOUBLE: return intl(64);
    case arrow::Type::FLOAT: return intl(32);
    case arrow::Type::HALF_FLOAT: return intl(16);
    case arrow::Type::INT8: return intl(8);
    case arrow::Type::INT16: return intl(16);
    case arrow::Type::INT32: return intl(32);
    case arrow::Type::INT64: return intl(64);
    case arrow::Type::TIME32: return intl(32);
    case arrow::Type::TIME64: return intl(64);
    case arrow::Type::TIMESTAMP: return intl(64);
    case arrow::Type::UINT8: return intl(8);
    case arrow::Type::UINT16: return intl(16);
    case arrow::Type::UINT32: return intl(32);
    case arrow::Type::UINT64: return intl(64);
    case arrow::Type::DECIMAL: return intl(128);

      // Lists:
    case arrow::Type::LIST: return strl("OFFSET_WIDTH");
    case arrow::Type::BINARY: return strl("OFFSET_WIDTH");
    case arrow::Type::STRING: return strl("OFFSET_WIDTH");

      // Others:
    default:
      // case arrow::Type::INTERVAL: return 0;
      // case arrow::Type::MAP: return 0;
      // case arrow::Type::NA: return 0;
      // case arrow::Type::DICTIONARY: return 0;
      // case arrow::Type::UNION: return 0;
      throw std::domain_error("Arrow type " + type.ToString() + " not supported.");

      // Structs have no width
    case arrow::Type::STRUCT: return intl(0);

      // Other width types:
    case arrow::Type::FIXED_SIZE_BINARY: {
      const auto *t = dynamic_cast<const arrow::FixedSizeBinaryType *>(&type);
      return intl(t->bit_width());
    }
  }
}

std::string GenerateConfigString(const arrow::Field &field, int level) {
  std::string ret;
  ConfigType ct = GetConfigType(*field.type());

  if (field.nullable()) {
    ret += "null(";
    level++;
  }

  int epc = fletcher::GetUIntMeta(field, fletcher::meta::VALUE_EPC, 1);
  int lepc = fletcher::GetUIntMeta(field, fletcher::meta::LIST_EPC, 1);

  bool has_children = false;

  if (ct == ConfigType::PRIM) {
    auto w = GetWidthNode(*field.type());
    ret += "prim(" + w->ToString();
    level++;
  } else if (ct == ConfigType::LIST_PRIM) {
    ret += "listprim(";
    level++;
    // Binary and string have no child, so we can't inspect it for the width, which is always 8.
    if ((field.type()->id() == arrow::Type::BINARY) || (field.type()->id() == arrow::Type::STRING)) {
      ret += "8";
    } else {
      // Other list of non-nullable primitives:
      ret += std::to_string(GetFixedWidthTypeBitWidth(*field.type()->field(0)->type()));
    }
  } else if (ct == ConfigType::LIST) {
    has_children = true;
    ret += "list(";
    level++;
  } else if (ct == ConfigType::STRUCT) {
    has_children = true;
    ret += "struct(";
    level++;
  }

  if (epc > 1 || lepc > 1) {
    ret += ";";
  }
  if (epc > 1) {
    ret += "epc=" + std::to_string(epc);
    if (lepc > 1) {
      ret += ",";
    }
  }
  if (lepc > 1) {
    ret += "lepc=" + std::to_string(epc);
  }

  if (has_children) {
    // Append children
    for (int c = 0; c < field.type()->num_fields(); c++) {
      auto child = field.type()->field(c);
      ret += GenerateConfigString(*child);
      if (c != field.type()->num_fields() - 1)
        ret += ",";
    }
  }

  for (; level > 0; level--)
    ret += ")";

  return ret;
}

std::shared_ptr<TypeMapper> GetStreamTypeMapper(Type *stream_type, Type *other) {
  auto result = TypeMapper::Make(stream_type, other);

  constexpr size_t idx_valid = 1;
  constexpr size_t idx_ready = 2;
  constexpr size_t idx_data = 4;
  constexpr size_t idx_dvalid = 5;
  constexpr size_t idx_last = 6;

  auto flat_stream = result->flat_a();
  for (size_t i = 0; i < flat_stream.size(); i++) {
    auto t = flat_stream[i].type_;
    if (t->Is(Type::RECORD)) {
    } else if (t == cerata::Stream::valid().get()) {
      result->Add(i, idx_valid);
    } else if (t == cerata::Stream::ready().get()) {
      result->Add(i, idx_ready);
    } else if (t->name() == dvalid()->name()) {
      result->Add(i, idx_dvalid);
    } else if (t->name() == last()->name()) {
      result->Add(i, idx_last);
    } else {
      // If it's not any of the default control signals on the stream, it must be data.
      result->Add(i, idx_data);
    }
  }
  return result;
}

std::shared_ptr<Type> ListPrimType(int val_epc, int list_epc, int val_width, int idx_width, const std::string &name) {
  auto data_width = val_epc * val_width;
  auto lengths_width = list_epc * idx_width;

  auto e_count_width = static_cast<int>(ceil(log2(val_epc + 1)));
  auto l_count_width = static_cast<int>(ceil(log2(list_epc + 1)));

  return record({field("", stream(record({field("dvalid", dvalid()),
                                          field("last", last()),
                                          field("length", length(lengths_width)),
                                          field("count", count(l_count_width))}))),
                 field(name, stream(record({field("dvalid", dvalid()),
                                            field("last", last()),
                                            field("", data(data_width)),
                                            field("count", count(e_count_width))})))});
}

std::shared_ptr<Type> GetStreamType(const arrow::Field &arrow_field, fletcher::Mode mode, int level) {
  // The ordering of the record fields in this function determines the order in which a nested stream is type converted
  // automatically using GetStreamTypeConverter. This corresponds to how the hardware is implemented.
  // More specifically, this is how the data, count and validity fields are currently concatenated onto one big data
  // field of the output and input streams of ArrayReaders/Writers.
  //
  // WARNING: Modifications to this function must be reflected in the manual hardware implementation of Fletcher
  //  components! See: hardware/arrays/ArrayConfig_pkg.vhd

  // Get the EPC values
  int epc = fletcher::GetUIntMeta(arrow_field, fletcher::meta::VALUE_EPC, 1);
  int lepc = fletcher::GetUIntMeta(arrow_field, fletcher::meta::LIST_EPC, 1);

  // Get their ceiled log2 to determine the count width
  auto e_count_width = static_cast<int>(ceil(log2(epc + 1)));
  auto l_count_width = static_cast<int>(ceil(log2(lepc + 1)));

  // Placeholder for the returning type.
  std::shared_ptr<Type> type;

  // Determine what Cerata type to generate from the Arrow field type.
  switch (arrow_field.type()->id()) {
    // Special case: binary type has a length stream and non-nullable byte stream.
    // The EPC is assumed to relate to the list values.
    // The LEPC can be used for the length stream.
    case arrow::Type::BINARY: return ListPrimType(epc, lepc, 8, ARROW_OFFSET_WIDTH, "bytes");
      // Special case: string type has a length stream and non-nullable utf8 character stream.
      // The EPC is assumed to relate to the list values.
      // The LEPC can be used for the length stream.
      // TODO(johanpel): reconsider the name of the chars stream.
    case arrow::Type::STRING: return ListPrimType(epc, lepc, 8, ARROW_OFFSET_WIDTH, "chars");

      // Lists could be either lists of non-nullable primitives, or of something else.
      // If the values are non-nullable primitives, we can use the "listprim" configuration, which has some additional
      // options.
    case arrow::Type::LIST: {
      // Sanity check, a list should only have one child field.
      if (arrow_field.type()->num_fields() != 1) {
        FLETCHER_LOG(FATAL, "Encountered Arrow list type with other than 1 child.");
      }
      // Inspect the child field.
      auto child_field = arrow_field.type()->field(0);
      // First handle the special case where this can be made a listprim
      if (GetConfigType(*child_field->type()) == ConfigType::PRIM) {
        auto w = GetFixedWidthTypeBitWidth(*child_field->type());
        FLETCHER_LOG(DEBUG, "Using \"listprim\" configuration for list of non-nullable primitives of width " << w);
        auto values_type = ConvertFixedWidthType(arrow_field.type()->field(0)->type(), epc);
        return ListPrimType(epc, lepc, w, ARROW_OFFSET_WIDTH, child_field->name());
      } else {
        // Lists of non-primitive types or nullable primitive types.
        // EPC or LEPC are not supported.
        if ((epc > 1) || (lepc > 1)) {
          FLETCHER_LOG(FATAL, "(Length)-elements-per-cycle > 1 on non-primitive list is not supported.");
        }
        auto values_type = GetStreamType(*child_field, mode, level + 1);
        auto child = stream(record({field("dvalid", dvalid()),
                                    field("last", last()),
                                    field("data", values_type),
                                    field("count", count(e_count_width))}));
        type = record({field("length", length(ARROW_OFFSET_WIDTH)),
                       field(child_field->name(), child)});
        e_count_width = l_count_width;
      }
      break;
    }

      // Structs
    case arrow::Type::STRUCT: {
      if (arrow_field.type()->num_fields() < 1) {
        FLETCHER_LOG(FATAL, "Encountered Arrow struct type without any children.");
      }
      std::vector<std::shared_ptr<cerata::Field>> children;
      for (const auto &f : arrow_field.type()->fields()) {
        auto child_type = GetStreamType(*f, mode, level + 1);
        children.push_back(field(f->name(), child_type));
      }
      type = record(arrow_field.name() + "_rec", children);
      break;
    }

      // Non-nested types
    default: {
      type = ConvertFixedWidthType(arrow_field.type(), epc);
      break;
    }
  }

  // If this is a top level field, create a stream out of it
  if (level == 0) {
    // Create the stream record
    auto rec = record({field("dvalid", dvalid()),
                       field("last", last())});
    if (arrow_field.nullable()) {
      rec->AddField(field("validity", validity()));
    }

    rec->AddField(field("", type));

    if (epc > 1) {
      rec->AddField(field("count", count(e_count_width)));
    }
    return stream(rec);
  } else {
    // Otherwise just return the type
    return type;
  }
}

// TODO(johanpel): move this into GetStreamType
std::pair<uint32_t, uint32_t> GetArrayDataSpec(const arrow::Field &arrow_field) {

  uint32_t epc = fletcher::GetUIntMeta(arrow_field, fletcher::meta::VALUE_EPC, 1);
  uint32_t lepc = fletcher::GetUIntMeta(arrow_field, fletcher::meta::LIST_EPC, 1);

  auto e_count_width = static_cast<int>(ceil(log2(epc + 1)));
  auto l_count_width = static_cast<int>(ceil(log2(lepc + 1)));

  uint32_t validity_bit = arrow_field.nullable() ? 1 : 0;

  switch (arrow_field.type()->id()) {
    case arrow::Type::BINARY: {
      auto data_width = epc * 8;
      auto length_width = lepc * 32;
      return {2, e_count_width + l_count_width + data_width + length_width + validity_bit};
    }

    case arrow::Type::STRING: {
      auto data_width = epc * 8;
      auto length_width = lepc * 32;
      return {2, e_count_width + l_count_width + data_width + length_width + validity_bit};
    }

      // Lists
    case arrow::Type::LIST: {
      auto child_field = arrow_field.type()->field(0);
      if (GetConfigType(*child_field->type()) == ConfigType::PRIM) {
        auto data_width = GetFixedWidthTypeBitWidth(*child_field->type());
        return {2, e_count_width + l_count_width + data_width * epc + ARROW_OFFSET_WIDTH * lepc + validity_bit};
      } else {
        auto arrow_child = arrow_field.type()->field(0);
        auto elem_spec = GetArrayDataSpec(*arrow_child);
        // Add a length stream to number of streams, and length width to data width.
        return {elem_spec.first + 1, elem_spec.second + ARROW_OFFSET_WIDTH + validity_bit};
      }
    }

      // Structs
    case arrow::Type::STRUCT: {
      if (epc > 1) {
        FLETCHER_LOG(ERROR, "Multi-elements-per-cycle at struct-level is unsupported."
                            "Try to set EPC > 1 at struct field level.");
      }
      if (lepc > 1) {
        FLETCHER_LOG(ERROR, "Struct delivers no length stream.");
      }
      if (arrow_field.type()->num_fields() < 1) {
        FLETCHER_LOG(ERROR, "Encountered Arrow struct type without any children.");
      }
      auto spec = std::pair<int, int>{0, 0};
      for (const auto &f : arrow_field.type()->fields()) {
        auto child_spec = GetArrayDataSpec(*f);
        spec.first += child_spec.first;
        spec.second += child_spec.second;
      }
      return spec;
    }

      // Non-nested types or unsupported types.
    default: {
      auto fwt = std::dynamic_pointer_cast<arrow::FixedWidthType>(arrow_field.type());
      if (fwt == nullptr) {
        FLETCHER_LOG(ERROR, "Unsupported Arrow type: " + arrow_field.type()->ToString());
      }
      return {1, (epc > 1 ? e_count_width : 0) + epc * (fwt->bit_width() + validity_bit)};
    }
  }
}

}  // namespace fletchgen
