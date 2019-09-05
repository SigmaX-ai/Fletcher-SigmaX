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

#include <arrow/api.h>

#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace fletcher {

/// @brief Access mode for reads / writes to recordbatches, arrays, buffers, etc. as seen from accelerator kernel.
enum class Mode {
  READ,  ///< Read mode
  WRITE  ///< Write mode
};

struct FieldMetadata {
  std::shared_ptr<arrow::DataType> type_{};
  int64_t length_ = 0;
  int64_t null_count_ = 0;
  FieldMetadata() = default;
  FieldMetadata(std::shared_ptr<arrow::DataType> type, int64_t length, int64_t null_count)
      : type_(std::move(type)), length_(length), null_count_(null_count) {}
};

struct BufferMetadata {
  const uint8_t *raw_buffer_;
  int64_t size_;
  std::vector<std::string> desc_;
  int level_ = 0;

  /// Implicit means the buffer might exists physically but is not required logically (e.g. an empty validity bitmap for
  /// non-nullable fields).
  bool implicit_ = false;

  BufferMetadata(const uint8_t *raw_buffer,
                 int64_t size,
                 std::vector<std::string> desc,
                 int level = 0,
                 bool implicit = false)
      : raw_buffer_(raw_buffer), size_(size), desc_(std::move(desc)), level_(level), implicit_(implicit) {}
};

struct RecordBatchDescription {
  std::string name;
  int64_t rows;
  std::vector<BufferMetadata> buffers;
  std::vector<FieldMetadata> fields;
  Mode mode = Mode::READ;
  // Virtual means that the RecordBatch might exist logically but is not defined physically. This is useful when
  // users supply a read schema, but no RecordBatch in simulation.
  bool is_virtual = false;
  std::string ToString() const;
};

/// @brief From the metadata of an Arrow Field, obtain the value of a specific key.
std::string GetMeta(const arrow::Field &field, const std::string &key);

/// @brief From the metadata of an Arrow Schema, obtain the value of a specific key.
std::string GetMeta(const arrow::Schema &schema, const std::string &key);

/**
 * @brief Return the schema operational mode (read or write) from the metadata, if any.
 * @param schema  The Arrow Schema to inspect.
 * @return        The schema operational mode, if any. Default = READ.
 */
Mode GetMode(const arrow::Schema &schema);

/**
 * @brief Obtain metadata and convert to integer.
 * @param field   A field
 * @return        The integer the field represents, if it exists. Returns default_to otherwise.
 */
int GetIntMeta(const arrow::Field &field, const std::string &key, int default_to);

/**
 * @brief Check if a field should be ignored in Fletcher.
 * @param field   The field to check the metadata for.
 * @return        Return true if the value for the "ignore" metadata key is set to "true", else false.
 */
bool MustIgnore(const arrow::Field &field);

/**
 * @brief Append the minimum required metadata for Fletcher to a schema. Returns a copy of the schema.
 * @param schema        The Schema to append to.
 * @param schema_name   The Schema name that will be used for hardware generation.
 * @param schema_mode   The Schema mode, i.e. whether to read or write from a RecordBatch as seen by
 *                      the accelerator kernel.
 * @return              A copy of the Schema with metadata appended.
 */
std::shared_ptr<arrow::Schema> AppendMetaRequired(const arrow::Schema &schema,
                                                  std::string schema_name,
                                                  Mode schema_mode);

/**
 * @brief Append Elements-Per-Cycle metadata to a field. Returns a copy of the field.
 *
 * This works only for primitive and list<primitive> fields.
 *
 * @param field   The field to append to.
 * @param epc     The elements-per-cycle.
 * @return        A copy of the field with metadata appended.
 */
std::shared_ptr<arrow::Field> AppendMetaEPC(const arrow::Field &field, int epc);

/**
 * @brief Append metadata to a field to signify Fletcher should ignore this field. Returns a copy of the field.
 * @param field   The field to append to.
 * @return        A copy of the field with metadata appended.
 */
std::shared_ptr<arrow::Field> AppendMetaIgnore(const arrow::Field &field);

/**
 * Write a schema to a Flatbuffer file
 * @param file_name   File to write to.
 * @param schema      The schema to write.
 * @return            The schema.
 */
void WriteSchemaToFile(const std::string &file_name, const arrow::Schema &schema);

/**
 * @brief Write the data buffers of an arrow::RecordBatch to a file.
 * @param filename    The path to the output file.
 * @param recordbatch The RecordBatch
 */
void WriteRecordBatchesToFile(const std::string &filename,
                              const std::vector<std::shared_ptr<arrow::RecordBatch>> &recordbatches);

/**
 * @brief Read one or multiple arrow::RecordBatch from a file.
 * @param file_name The path to the input file.
 * @param out       Vector to store the RecordBatches.
 */
void ReadRecordBatchesFromFile(const std::string &file_name, std::vector<std::shared_ptr<arrow::RecordBatch>> *out);

/**
 * @brief Reads a schema from a file.
 * @param file_path Path to the file to read from.
 * @param out       The schema
 */
void ReadSchemaFromFile(const std::string &file_path, std::shared_ptr<arrow::Schema> *out);

std::string ToString(const std::vector<std::string> &strvec, const std::string &sep = "_");

}  // namespace fletcher
