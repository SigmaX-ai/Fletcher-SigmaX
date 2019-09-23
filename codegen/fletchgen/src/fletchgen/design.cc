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

#include <cerata/api.h>

#include <string>
#include <deque>

#include "fletcher/common.h"
#include "fletchgen/design.h"
#include "fletchgen/recordbatch.h"
#include "fletchgen/profiler.h"
#include "fletchgen/mmio.h"

namespace fletchgen {

using cerata::OutputSpec;

/// Short-hand for vector of RecordBatches.
using RBVector = std::vector<std::shared_ptr<arrow::RecordBatch>>;

static std::optional<std::shared_ptr<arrow::RecordBatch>> GetRecordBatchWithName(const RBVector &batches,
                                                                                 const std::string &name) {
  for (const auto &b :  batches) {
    if (fletcher::GetMeta(*b->schema(), "fletcher_name") == name) {
      return b;
    }
  }
  return std::nullopt;
}

void Design::AnalyzeSchemas() {
  // Attempt to create a SchemaSet from all schemas that can be detected in the options.
  FLETCHER_LOG(INFO, "Creating SchemaSet.");
  schema_set = SchemaSet::Make(options->kernel_name);
  // Add all schemas from the list of schema files
  for (const auto &arrow_schema : options->schemas) {
    schema_set->AppendSchema(arrow_schema);
  }
  // Add all schemas from the recordbatches and add all recordbatches.
  for (const auto &recordbatch : options->recordbatches) {
    schema_set->AppendSchema(recordbatch->schema());
  }

  // Sort the schema set according to the recordbatch ordering specification.
  // Important for the control flow through MMIO / buffer addresses.
  // First we sort recordbatches by name, then by mode.
  schema_set->Sort();
}

void Design::AnalyzeRecordBatches() {
  // Now that we have every Schema, for every Schema, figure out if there is a RecordBatch in the input options.
  // If there is, add a description of the RecordBatch to this design.
  // If there isn't, create a virtual RecordBatch based on the schema.
  for (const auto &fletcher_schema : schema_set->schemas()) {
    auto rb = GetRecordBatchWithName(options->recordbatches, fletcher_schema->name());
    fletcher::RecordBatchDescription rbd;
    if (rb) {
      fletcher::RecordBatchAnalyzer rba(&rbd);
      rba.Analyze(**rb);
    } else {
      fletcher::SchemaAnalyzer sa(&rbd);
      sa.Analyze(*fletcher_schema->arrow_schema());
    }
    batch_desc.push_back(rbd);
  }
}

fletchgen::Design fletchgen::Design::GenerateFrom(const std::shared_ptr<Options> &opts) {
  Design ret;
  ret.options = opts;

  // Analyze schemas and recordbatches to get schema_set and batch_desc
  ret.AnalyzeSchemas();
  ret.AnalyzeRecordBatches();

  // Generate the hardware structure through Mantle and extract all subcomponents.
  FLETCHER_LOG(INFO, "Generating Mantle...");
  ret.mantle = Mantle::Make(*ret.schema_set, ret.batch_desc);
  ret.kernel = ret.mantle->nucleus()->kernel;
  ret.nucleus = ret.mantle->nucleus();
  for (const auto &recordbatch_component : ret.mantle->recordbatch_components()) {
    ret.recordbatches.push_back(recordbatch_component);
  }

  // Generate a Yaml file for vhd mmio based on the recordbatch description
  GenerateVhdmmioYaml(ret.batch_desc);

  // ret.mmio = GenerateVhdmmioComponent(ret.batch_desc);
  // ret.mantle->AddInstanceOf(ret.mmio.get());

  // TODO(johanpel): fix this:
  system("vhdmmio -V vhdl -H -P vhdl");

  // EnableStreamProfiling(ret.mantle.get());

  return ret;
}

std::deque<cerata::OutputSpec> Design::GetOutputSpec() {
  std::deque<OutputSpec> result;
  OutputSpec omantle, okernel, onucleus;

  // Mantle
  omantle.comp = mantle;
  // Always overwrite mantle, as users should not modify.
  omantle.meta[cerata::vhdl::metakeys::OVERWRITE_FILE] = "true";
  result.push_back(omantle);

  // Nucleus
  onucleus.comp = nucleus;
  // Check the force flag if kernel should be overwritten
  onucleus.meta[cerata::vhdl::metakeys::OVERWRITE_FILE] = "true";
  result.push_back(onucleus);

  // Kernel
  okernel.comp = kernel;
  // Check the force flag if kernel should be overwritten
  okernel.meta[cerata::vhdl::metakeys::OVERWRITE_FILE] = options->overwrite ? "true" : "false";
  result.push_back(okernel);

  // RecordBatchReaders/Writers
  for (const auto &recbatch : recordbatches) {
    OutputSpec orecbatch;
    orecbatch.comp = recbatch;
    // Always overwrite readers/writers, as users should not modify.
    orecbatch.meta[cerata::vhdl::metakeys::OVERWRITE_FILE] = "true";
    result.push_back(orecbatch);
  }

  return result;
}

}  // namespace fletchgen
