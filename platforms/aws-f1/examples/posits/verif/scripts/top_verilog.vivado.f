# Amazon FPGA Hardware Development Kit
#
# Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Amazon Software License (the "License"). You may not use
# this file except in compliance with the License. A copy of the License is
# located at
#
#    http://aws.amazon.com/asl/
#
# or in the "license" file accompanying this file. This file is distributed on
# an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or
# implied. See the License for the specific language governing permissions and
# limitations under the License.

--define VIVADO_SIM

--sourcelibext .v
--sourcelibext .sv
--sourcelibext .svh

--sourcelibdir ${CL_ROOT}/design
--sourcelibdir ${SH_LIB_DIR}
--sourcelibdir ${SH_INF_DIR}
--sourcelibdir ${HDK_SHELL_DESIGN_DIR}/sh_ddr/sim

--include ${CL_ROOT}/../common/design
--include ${CL_ROOT}/verif/sv
--include ${SH_LIB_DIR}
--include ${SH_INF_DIR}
--include ${HDK_COMMON_DIR}/verif/include
--include ${SH_LIB_DIR}/../ip/cl_axi_interconnect/ipshared/7e3a/hdl
--include ${HDK_SHELL_DESIGN_DIR}/sh_ddr/sim
--include ${HDK_SHELL_DESIGN_DIR}/ip/axi_register_slice_light/hdl

${SH_LIB_DIR}/../ip/axi_clock_converter_0/sim/axi_clock_converter_0.v
${SH_LIB_DIR}/../ip/axi_register_slice/sim/axi_register_slice.v
${SH_LIB_DIR}/../ip/axi_register_slice_light/sim/axi_register_slice_light.v
${HDK_SHELL_DESIGN_DIR}/ip/axi_register_slice_light/hdl/axi_register_slice_v2_1_vl_rfs.v
${HDK_SHELL_DESIGN_DIR}/ip/axi_clock_converter_0/hdl/axi_clock_converter_v2_1_vl_rfs.v

# Top level interconnect between PCI Slave and DDR C
${CL_ROOT}/design/ip/axi_interconnect_top/sim/axi_interconnect_top.v

# Custom IP
${CL_ROOT}/design/ip/accum_fifo_es3/sim/accum_fifo_es3.v
${CL_ROOT}/design/ip/element_fifo/sim/element_fifo.v
${CL_ROOT}/design/ip/output_fifo/sim/output_fifo.v
${CL_ROOT}/design/ip/posits_clock/posits_clock.v
${CL_ROOT}/design/ip/posits_clock/posits_clock_clk_wiz.v

--define DISABLE_VJTAG_DEBUG
${CL_ROOT}/design/cl_arrow_defines.vh
${CL_ROOT}/design/cl_arrow_pkg.sv
${CL_ROOT}/design/cl_arrow.sv

-f ${HDK_COMMON_DIR}/verif/tb/filelists/tb.${SIMULATOR}.f

${TEST_NAME}

${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_defines_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/lib/LOD_N.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/lib/LZD_N.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/lib/shift_left.v
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/lib/shift_right.v
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positaccum_16_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positaccum_16_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positaccum_accum_16_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positaccum_accumprod_16_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positaccum_prod_16_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_4_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_4_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_4_truncated_prodsum_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_4_truncated_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_8_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_8_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_prod_4_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positadd_prod_8_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_extract_accum_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_extract_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_extract_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positmult_4_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positmult_4_raw_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positmult_4_raw_sumval_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positmult_4_truncated_raw_prodsumsum_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/positmult_4_truncated_raw_sumval_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_accum_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_accum_prod_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_prod_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_prod_sum_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_product_prod_sum_sum_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_normalize_sum_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_prod_to_value_es3.sv
${FLETCHER_EXAMPLES_DIR}/posits/hardware/posit_arith_hdl/es3/posit_sum_to_value_es3.sv
