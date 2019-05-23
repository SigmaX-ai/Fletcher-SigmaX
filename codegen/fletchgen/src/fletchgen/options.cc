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

#include "fletchgen/options.h"

#include <fletcher/common.h>
#include <CLI/CLI11.hpp>

namespace fletchgen {
int Options::Parse(Options *options, int argc, char **argv) {

  CLI::App app{"Fletchgen - The Fletcher Wrapper Generator"};

  app.get_formatter()->column_width(40);

  // Required options:
  app.add_option("-i,--input", options->schema_paths,
                 "List of Flatbuffer files with Arrow Schemas to base wrapper on, comma seperated. "
                 "Example: --input file1.fbs file2.fbs file3.fbs")
      ->required()
      ->check(CLI::ExistingFile);

  // Naming options:
  app.add_option("-n,--kernel_name", options->kernel_name,
                 "Name of the accelerator kernel.");

  // Simulation options:
  app.add_option("-r,--recordbatch_input", options->recordbatch_paths,
                 "List of Flatbuffer files with Arrow RecordBatches to convert to SREC format for simulation. "
                 "RecordBatches must adhere to and be in the same order as Arrow Schemas set with option --input.");
  app.add_option("-s,--recordbatch_output", options->srec_out_path,
                 "SREC simulation output file.");
  app.add_option("-t,--srec_dump", options->srec_sim_dump,
                 "File to dump memory contents to in SREC format after simulation.");

  // Output options:
  app.add_option("-o,--output_path", options->output_dir,
                 "Path to the output directory to place the generated files. (Default: . )")
      ->check(CLI::ExistingDirectory);

  app.add_option("-l,--language", options->languages,
                 "Select the output languages for your design. Each type of output will be stored in a "
                 "seperate subfolder (e.g. <output folder>/vhdl/...). \n"
                 "                                        Available languages:\n"
                 "                                          vhdl: Export as VHDL files (default).\n"
                 "                                          dot : Export as DOT graphs.");

  app.add_flag("-f,--force", options->override_kernels,
               "Force overwriting kernel component if it exists already. If this flag is not used and the kernel "
               "source exists already in the specified path, the output filename will be <kernel>.vhdt and is always "
               "overwritten.");

  app.add_flag("--axi", options->axi_top,
               "Generate AXI top-level template (VHDL only).");
  app.add_flag("--sim", options->sim_top,
               "Generate simulation top-level template (VHDL only).");
  app.add_flag("--vivado_hls", options->vivado_hls,
               "Generate a Vivado HLS kernel template.");

  // Other options:
  // TODO(johanpel): implement the quiet and verbose options
  /*
  app.add_flag("-q,--quiet", options->quiet,
               "Surpress all stdout.");
  app.add_flag("-v,--verbose", options->verbose,
               "More detailed information on stdout.");
  */

  CLI11_PARSE(app, argc, argv)

  return 0;
}

bool Options::MustGenerateSREC() {
  if (!srec_out_path.empty()) {
    if (schema_paths.size() == recordbatch_paths.size()) {
      return true;
    } else {
      FLETCHER_LOG(WARNING, "SREC output flag set, but number of RecordBatches inputs is not equal to number of Schemas.");
      return true;
    }
  }
  return false;
}

static bool HasLanguage(const std::vector<std::string> &languages, const std::string &lang) {
  for (const auto &l : languages) {
    if (l == lang) {
      return true;
    }
  }
  return false;
}

bool Options::MustGenerateVHDL() {
  return HasLanguage(languages, "vhdl");
}

bool Options::MustGenerateDOT() {
  return HasLanguage(languages, "dot");
}

bool Options::MustGenerateDesign() {
  return !schema_paths.empty();
}

} // namespace fletchgen
