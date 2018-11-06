# Script to generate the Xilinx IP used in Amazon EC2 F1 example.

source $::env(HDK_DIR)/common/shell_stable/build/scripts/device_type.tcl

create_project -in_memory -part [DEVICE_TYPE] -force

# Read Fletcher example IP
read_ip [ list \
  $::env(CL_DIR)/design/ip/axi_interconnect_top/axi_interconnect_top.xci \
  $::env(CL_DIR)/design/ip/accum_fifo_es2/accum_fifo_es2.xci \
  $::env(CL_DIR)/design/ip/accum_fifo_es3/accum_fifo_es3.xci \
  $::env(CL_DIR)/design/ip/ADDSUB151_8/ADDSUB151_8.xci \
  $::env(CL_DIR)/design/ip/ADDSUB256_8/ADDSUB256_8.xci \
  $::env(CL_DIR)/design/ip/element_fifo/element_fifo.xci \
  $::env(CL_DIR)/design/ip/output_fifo/output_fifo.xci \
  $::env(CL_DIR)/design/ip/posits_clock/posits_clock.xci 
]

# Generate the IP used in cl_arrow
generate_target all [get_files $::env(CL_DIR)/design/ip/axi_interconnect_top/axi_interconnect_top.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/axi_interconnect_top/axi_interconnect_top.xci]

# Generate the IP used in posits implementation
generate_target all [get_files $::env(CL_DIR)/design/ip/accum_fifo_es2/accum_fifo_es2.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/accum_fifo_es2/accum_fifo_es2.xci]

generate_target all [get_files $::env(CL_DIR)/design/ip/accum_fifo_es3/accum_fifo_es3.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/accum_fifo_es3/accum_fifo_es3.xci]

generate_target all [get_files $::env(CL_DIR)/design/ip/ADDSUB151_8/ADDSUB151_8.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/ADDSUB151_8/ADDSUB151_8.xci]

generate_target all [get_files $::env(CL_DIR)/design/ip/ADDSUB256_8/ADDSUB256_8.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/ADDSUB256_8/ADDSUB256_8.xci]

generate_target all [get_files $::env(CL_DIR)/design/ip/element_fifo/element_fifo.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/element_fifo/element_fifo.xci]

generate_target all [get_files $::env(CL_DIR)/design/ip/output_fifo/output_fifo.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/output_fifo/output_fifo.xci]

generate_target all [get_files $::env(CL_DIR)/design/ip/posits_clock/posits_clock.xci]
synth_ip [get_files $::env(CL_DIR)/design/ip/output_fifo/posits_clock.xci]
