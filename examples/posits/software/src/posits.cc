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

/**
 * Example for doing some posit basic linear algebra operations using Fletcher
 *
 */

#include <stdlib.h>
#include <memory>

// Apache Arrow
#include <arrow/api.h>

// Fletcher
#include <fletcher/api.h>
#include <fletcher/common/timer.h>
#include <fletcher/common/arrow-utils.h>

std::shared_ptr<arrow::Array> createPositsArray(size_t num_vecs, size_t vec_size) {
  // Posits array
  std::shared_ptr<arrow::Array> pa;

  // Posits vector builder
  auto pb = std::make_shared<arrow::UInt32Builder>();

  // Posits vector list builder
  arrow::ListBuilder plb(arrow::default_memory_pool(), pb);

  for (size_t j = 0; j < num_vecs; j++) {
    if (!plb.Append().ok()) {
      throw std::runtime_error("Could not append to list builder.");
    }
    for (size_t i = 0; i < vec_size; i++) {
      if (!pb->Append(1).ok()) {
        throw std::runtime_error("Could not append to posits builder.");
      }
    }
  }

  if (!plb.Finish(&pa).ok()) {
    throw std::runtime_error("Could not finish list builder.");
  }

  return pa;
}

int main(int argc, char **argv) {
  fletcher::Timer t;

  constexpr int num_rows = 100;
  constexpr int vec_size = 4;

  // Create input in_schema:
  auto in_fp = std::make_shared<arrow::Field>("a", arrow::uint32(), false);
  auto in_flpa = std::make_shared<arrow::Field>("posits_a", arrow::list(in_fp), false);
  auto in_flpb = std::make_shared<arrow::Field>("posits_b", arrow::list(in_fp), false);
  auto in_fields = std::vector<std::shared_ptr<arrow::Field>>({in_flpa, in_flpb});
  auto in_schema = std::make_shared<arrow::Schema>(in_fields, fletcher::metaMode(fletcher::Mode::READ));

  // Create input arrays and recordbatch
  auto in_pa = createPositsArray(num_rows, vec_size);
  auto in_pb = createPositsArray(num_rows, vec_size);
  auto in_rb = arrow::RecordBatch::Make(in_schema, num_rows, {in_pa, in_pb});

  // Create output in_schema:
  auto out_fpc = std::make_shared<arrow::Field>("posits_a", arrow::uint32(), false);
  auto out_fields = std::vector<std::shared_ptr<arrow::Field>>({out_fpc});
  auto out_schema = std::make_shared<arrow::Schema>(out_fields,
                                                    fletcher::metaMode(fletcher::Mode::WRITE));

  // Create output array and recordbatch
  uint8_t *out_buf = nullptr;
  size_t out_buf_size = 4096 * (((sizeof(uint32_t) * vec_size) / 4096) + 1);
  posix_memalign(reinterpret_cast<void **>(&out_buf), 4096, out_buf_size);
  auto out_pb = std::make_shared<arrow::Buffer>(out_buf, out_buf_size);
  auto out_pc = std::make_shared<arrow::PrimitiveArray>(arrow::uint32(), vec_size, out_pb);
  auto out_rb = arrow::RecordBatch::Make(out_schema, num_rows, {out_pc});

  // Create Fletcher objects
  std::shared_ptr<fletcher::Platform> platform;
  std::shared_ptr<fletcher::Context> context;
  std::shared_ptr<fletcher::UserCore> usercore;

  // Create and initialize platform and context
  fletcher::Platform::Make(&platform);
  fletcher::Context::Make(&context, platform);
  usercore = std::make_shared<fletcher::UserCore>(context);

  platform->init();

  usercore->ctrl_start = 0x00000001;
  usercore->ctrl_reset = 0x00000002;
  usercore->done_status = 0x00000002;
  usercore->done_status_mask = 0x3;

  usercore->reset();

  // Queue the data and enable the context
  context->queueRecordBatch(in_rb);
  context->queueRecordBatch(out_rb);
  context->enable();

  // Write posit operation
  platform->writeMMIO(16, 1);

  usercore->start();
  usercore->waitForFinish(100000);

  return EXIT_SUCCESS;
}
