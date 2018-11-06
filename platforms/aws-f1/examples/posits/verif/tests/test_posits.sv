// Amazon FPGA Hardware Development Kit
//
// Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Amazon Software License (the "License"). You may not use
// this file except in compliance with the License. A copy of the License is
// located at
//
//    http://aws.amazon.com/asl/
//
// or in the "license" file accompanying this file. This file is distributed on
// an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or
// implied. See the License for the specific language governing permissions and
// limitations under the License.

/**
 * Fletcher Posits AWS EC2 f1 test-bench
 *
 * This example generated a small number of strings containing the word "kitten"
 * in two buffers, an offsets and data buffer, according to the Apache Arrow format
 * specification. Then, it continues to copy the buffers to the on-board DDR memory
 * and runs at most sixteen regular expression matching units and reports the
 * number of matches for each units, and in total.
 */

// Default registers:
`define REG_CONTROL                 0
`define REG_STATUS                  1
`define REG_RETURN0                 2
`define REG_RETURN1                 3
`define REG_FIRSTIDX                4
`define REG_LASTIDX			            5

`define CONTROL_START               32'h00000001
`define CONTROL_STOP                32'h00000002
`define CONTROL_RESET               32'h00000004
`define STATUS_BUSY                 32'h00000001
`define STATUS_DONE                 32'h00000002

// Address registers
`define REG_ELEMENT1_OFF_ADDR_LO    6
`define REG_ELEMENT1_OFF_ADDR_HI    7
`define REG_ELEMENT1_POSIT_ADDR_LO  8
`define REG_ELEMENT1_POSIT_ADDR_HI  9
`define REG_ELEMENT2_OFF_ADDR_LO    10
`define REG_ELEMENT2_OFF_ADDR_HI    11
`define REG_ELEMENT2_POSIT_ADDR_LO  12
`define REG_ELEMENT2_POSIT_ADDR_HI  13
`define REG_RESULT_DATA_ADDR_LO     14
`define REG_RESULT_DATA_ADDR_HI     15

// Custom registers
`define REG_OPERATION               16

`define NUM_REGISTERS               17

`define VECTOR_LEN                  4
`define NUM_ROWS                    8

`define ELEMENT1_VAL_ADDR_LO		    32'h00000000
`define ELEMENT1_VAL_ADDR_HI		    32'h00000000
`define ELEMENT1_OFF_ADDR_LO  		  32'h00010000
`define ELEMENT1_OFF_ADDR_HI  		  32'h00000000
`define ELEMENT2_VAL_ADDR_LO		    32'h00020000
`define ELEMENT2_VAL_ADDR_HI		    32'h00000000
`define ELEMENT2_OFF_ADDR_LO  		  32'h00030000
`define ELEMENT2_OFF_ADDR_HI  		  32'h00000000
`define RESULT_VAL_ADDR_LO   		    32'h00040000
`define RESULT_VAL_ADDR_HI   		    32'h00000000

`define HOST_ELEMENT1_VAL_ADDR      64'h0000000000000000
`define HOST_ELEMENT1_OFF_ADDR      64'h0000000000010000
`define HOST_ELEMENT2_VAL_ADDR      64'h0000000000020000
`define HOST_ELEMENT2_OFF_ADDR      64'h0000000000030000
`define HOST_RESULT_VAL_ADDR        64'h0000000000040000

// Currently unused:
`define ELEMENT1_POSIT_VBMP_LO      0
`define ELEMENT1_POSIT_VBMP_HI      0
`define ELEMENT2_POSIT_VBMP_LO      0
`define ELEMENT2_POSIT_VBMP_HI      0

module test_posits();

import tb_type_defines_pkg::*;

int         error_count;
int         timeout_count;
int         fail;
logic [3:0] status;
logic       ddr_ready;
int         read_data;

int num_rows = `NUM_ROWS;

union {
  logic[31:0] i;
  logic[3:0][3:0] bytes;
} buf_data;

initial begin

  logic[63:0] h_elem1_val_addr;
  logic[63:0] h_elem1_off_addr;
  logic[63:0] h_elem2_val_addr;
  logic[63:0] h_elem2_off_addr;
  logic[63:0] h_result_val_addr;
  
  logic[63:0] d_elem1_val_addr;
  logic[63:0] d_elem1_off_addr;
  logic[63:0] d_elem2_val_addr;
  logic[63:0] d_elem2_off_addr;
  logic[63:0] d_result_val_addr;
  
  h_elem1_val_addr  = `HOST_ELEMENT1_VAL_ADDR;
  h_elem1_off_addr  = `HOST_ELEMENT1_OFF_ADDR;
  h_elem2_val_addr  = `HOST_ELEMENT2_VAL_ADDR;
  h_elem2_off_addr  = `HOST_ELEMENT2_OFF_ADDR;
  h_result_val_addr = `HOST_RESULT_VAL_ADDR;
  
  d_elem1_val_addr  = {`ELEMENT1_VAL_ADDR_HI, `ELEMENT1_VAL_ADDR_LO};
  d_elem1_off_addr  = {`ELEMENT1_OFF_ADDR_HI, `ELEMENT1_OFF_ADDR_LO};
    
  d_elem2_val_addr  = {`ELEMENT2_VAL_ADDR_HI, `ELEMENT2_VAL_ADDR_LO};
  d_elem2_off_addr  = {`ELEMENT2_OFF_ADDR_HI, `ELEMENT2_OFF_ADDR_LO};
  
  d_result_val_addr = {`RESULT_VAL_ADDR_HI, `RESULT_VAL_ADDR_LO};
  

  // Power up the testbench
  tb.power_up(.clk_recipe_a(ClockRecipe::A1),
              .clk_recipe_b(ClockRecipe::B0),
              .clk_recipe_c(ClockRecipe::C0));

  tb.nsec_delay(1000);

  tb.poke_stat(.addr(8'h0c), .ddr_idx(0), .data(32'h0000_0000));
  tb.poke_stat(.addr(8'h0c), .ddr_idx(1), .data(32'h0000_0000));
  tb.poke_stat(.addr(8'h0c), .ddr_idx(2), .data(32'h0000_0000));

  // Allow memory to initialize
  tb.nsec_delay(27000);

  $display("[%t] : Initializing buffers", $realtime);

  // Queue the data movement
  tb.que_buffer_to_cl(.chan(0), .src_addr(h_elem1_val_addr), .cl_addr(d_elem1_val_addr), .len(4 * `VECTOR_LEN * `NUM_ROWS));
  tb.que_buffer_to_cl(.chan(0), .src_addr(h_elem1_off_addr), .cl_addr(d_elem1_off_addr), .len(4 * (`NUM_ROWS + 1)));
  tb.que_buffer_to_cl(.chan(0), .src_addr(h_elem2_val_addr), .cl_addr(d_elem2_val_addr), .len(4 * `VECTOR_LEN * `NUM_ROWS));
  tb.que_buffer_to_cl(.chan(0), .src_addr(h_elem2_off_addr), .cl_addr(d_elem2_off_addr), .len(4 * (`NUM_ROWS + 1)));

  // Fill posit vector 1 values
  for (int i = 0; i < `NUM_ROWS * `VECTOR_LEN; i++) begin
    buf_data.i = i;
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i    ), .d(buf_data.bytes[0]));
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i + 1), .d(buf_data.bytes[1]));
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i + 2), .d(buf_data.bytes[2]));
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i + 3), .d(buf_data.bytes[3]));
  end
  
  // Fill posit vector 1 offsets
  for (int i = 0; i < `NUM_ROWS + 1; i++) begin
    buf_data.i = 4 * i;
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i    ), .d(buf_data.bytes[0]));
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i + 1), .d(buf_data.bytes[1]));
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i + 2), .d(buf_data.bytes[2]));
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i + 3), .d(buf_data.bytes[3]));
  end
  
  // Fill posit vector 2 values
  for (int i = 0; i < `NUM_ROWS * `VECTOR_LEN; i++) begin
    buf_data.i = 256 + i;
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i    ), .d(buf_data.bytes[0]));
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i + 1), .d(buf_data.bytes[1]));
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i + 2), .d(buf_data.bytes[2]));
    tb.hm_put_byte(.addr(h_elem1_val_addr + 4 * i + 3), .d(buf_data.bytes[3]));
  end
  
  // Fill posit vector 2 offsets
  for (int i = 0; i < `NUM_ROWS + 1; i++) begin
    buf_data.i = 4 * i;
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i    ), .d(buf_data.bytes[0]));
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i + 1), .d(buf_data.bytes[1]));
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i + 2), .d(buf_data.bytes[2]));
    tb.hm_put_byte(.addr(h_elem1_off_addr + 4 * i + 3), .d(buf_data.bytes[3]));
  end
  
  $display("[%t] : Starting host to CL DMA transfers ", $realtime);

  // Start transfers of data to CL DDR
  tb.start_que_to_cl(.chan(0));

  // Wait for dma transfers to complete,
  // increase the timeout if you have to transfer a lot of data
  timeout_count = 0;
  do begin
    status[0] = tb.is_dma_to_cl_done(.chan(0));
    #10ns;
    timeout_count++;
  end while ((status != 4'hf) && (timeout_count < 4000));

  if (timeout_count >= 4000) begin
    $display(
      "[%t] : *** ERROR *** Timeout waiting for dma transfers from cl",
      $realtime
    );
    error_count++;
  end

  tb.nsec_delay(1000);

  // Fletcher posits starts here
  $display("[%t] : Initializing UserCore ", $realtime);

  // Reset
  tb.poke_bar1(.addr(4*`REG_CONTROL), .data(`CONTROL_RESET));
  tb.poke_bar1(.addr(4*`REG_CONTROL), .data(0));

  // Initialize buffer addressess
  tb.poke_bar1(.addr(4*`REG_ELEMENT1_POSIT_ADDR_LO), .data(`ELEMENT1_VAL_ADDR_LO));
  tb.poke_bar1(.addr(4*`REG_ELEMENT1_POSIT_ADDR_HI), .data(`ELEMENT1_VAL_ADDR_HI));
  tb.poke_bar1(.addr(4*`REG_ELEMENT1_OFF_ADDR_LO  ), .data(`ELEMENT1_OFF_ADDR_LO));
  tb.poke_bar1(.addr(4*`REG_ELEMENT1_OFF_ADDR_HI  ), .data(`ELEMENT1_OFF_ADDR_HI));
  tb.poke_bar1(.addr(4*`REG_ELEMENT2_POSIT_ADDR_LO), .data(`ELEMENT2_VAL_ADDR_LO));
  tb.poke_bar1(.addr(4*`REG_ELEMENT2_POSIT_ADDR_HI), .data(`ELEMENT2_VAL_ADDR_HI));
  tb.poke_bar1(.addr(4*`REG_ELEMENT2_OFF_ADDR_LO  ), .data(`ELEMENT2_OFF_ADDR_LO));
  tb.poke_bar1(.addr(4*`REG_ELEMENT2_OFF_ADDR_HI  ), .data(`ELEMENT2_OFF_ADDR_HI));
  tb.poke_bar1(.addr(4*`REG_RESULT_DATA_ADDR_LO   ), .data(`RESULT_VAL_ADDR_LO  ));
  tb.poke_bar1(.addr(4*`REG_RESULT_DATA_ADDR_HI   ), .data(`RESULT_VAL_ADDR_HI  ));

  // Set first and last row index
  tb.poke_bar1(.addr(4 * (`REG_FIRSTIDX)), .data(0));
  tb.poke_bar1(.addr(4 * (`REG_LASTIDX)), .data(`NUM_ROWS));
  
  // Set the operation (vector dot product)
  tb.poke_bar1(.addr(4 * (`REG_OPERATION)), .data(1));

  for (int i=0; i<`NUM_REGISTERS; i++) begin
    tb.peek_bar1(.addr(i*4), .data(read_data));
    $display("[DEBUG] : Register %d: %H", i, read_data);
  end

  // Start UserCore
  $display("[%t] : Starting UserCore", $realtime);
  tb.poke_bar1(.addr(4*`REG_CONTROL), .data(`CONTROL_START));

  // Poll status at an interval of 1000 nsec
  // For the real thing, you should probably increase this to put
  // less stress on the PCI interface
  do
    begin
      tb.nsec_delay(1000);
      tb.peek_bar1(.addr(4*`REG_STATUS), .data(read_data));
      $display("[%t] : UserCore status: %H", $realtime, read_data);
    end
  while(read_data !== `STATUS_DONE);

  $display("[%t] : UserCore completed ", $realtime);

  // Get the return register value
  tb.peek_bar1(.addr(4*`REG_RETURN0), .data(read_data));
  $display("[t] : Return register 0: %d", read_data);
  tb.peek_bar1(.addr(4*`REG_RETURN1), .data(read_data));
  $display("[t] : Return register 1: %d", read_data);

  // Power down
  #500ns;
  tb.power_down();


  // Report pass/fail status
  $display("[%t] : Checking total error count...", $realtime);
  if (error_count > 0) begin
    fail = 1;
  end
  $display(
    "[%t] : Detected %3d errors during this test",
    $realtime, error_count
  );

  if (fail || (tb.chk_prot_err_stat())) begin
    $display("[%t] : *** TEST FAILED ***", $realtime);
  end else begin
    $display("[%t] : *** TEST PASSED ***", $realtime);
  end

  $finish;
end // initial begin

endmodule // test_posits
