-- Copyright 2018 Delft University of Technology
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- This file was automatically generated by FletchGen. Modify this file
-- at your own risk.

library ieee;
  use ieee.std_logic_1164.all;
  use ieee.std_logic_misc.all;

library work;
  use work.Streams.all;

entity filter_usercore is
  generic(
    TAG_WIDTH                                  : natural;
    BUS_ADDR_WIDTH                             : natural;
    INDEX_WIDTH                                : natural;
    REG_WIDTH                                  : natural
  );
  port(
    write_first_name_unlock_valid              : in std_logic;
    write_first_name_unlock_tag                : in std_logic_vector(TAG_WIDTH-1 downto 0);
    write_first_name_unlock_ready              : out std_logic;
    write_first_name_in_values_in_valid        : out std_logic;
    write_first_name_in_values_in_ready        : in std_logic;
    write_first_name_in_values_in_last         : out std_logic;
    write_first_name_in_values_in_dvalid       : out std_logic;
    write_first_name_in_values_in_data         : out std_logic_vector(7 downto 0);
    write_first_name_in_values_in_count        : out std_logic_vector(0 downto 0);
    write_first_name_in_valid                  : out std_logic;
    write_first_name_in_ready                  : in std_logic;
    write_first_name_in_length                 : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    write_first_name_in_last                   : out std_logic;
    write_first_name_cmd_write_first_name_values_addr: out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    write_first_name_cmd_write_first_name_offsets_addr: out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    write_first_name_cmd_valid                 : out std_logic;
    write_first_name_cmd_tag                   : out std_logic_vector(TAG_WIDTH-1 downto 0);
    write_first_name_cmd_ready                 : in std_logic;
    write_first_name_cmd_lastIdx               : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    write_first_name_cmd_firstIdx              : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    -------------------------------------------------------------------------
    reg_write_first_name_values_addr           : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    reg_write_first_name_offsets_addr          : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -------------------------------------------------------------------------
    reg_return1                                : out std_logic_vector(REG_WIDTH-1 downto 0);
    reg_return0                                : out std_logic_vector(REG_WIDTH-1 downto 0);
    -------------------------------------------------------------------------
    reg_read_zipcode_values_addr               : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    reg_read_last_name_values_addr             : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    reg_read_last_name_offsets_addr            : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    reg_read_first_name_values_addr            : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    reg_read_first_name_offsets_addr           : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -------------------------------------------------------------------------
    read_zipcode_unlock_valid                  : in std_logic;
    read_zipcode_unlock_tag                    : in std_logic_vector(TAG_WIDTH-1 downto 0);
    read_zipcode_unlock_ready                  : out std_logic;
    read_zipcode_out_valid                     : in std_logic;
    read_zipcode_out_ready                     : out std_logic;
    read_zipcode_out_last                      : in std_logic;
    read_zipcode_out_data                      : in std_logic_vector(31 downto 0);
    read_zipcode_cmd_valid                     : out std_logic;
    read_zipcode_cmd_tag                       : out std_logic_vector(TAG_WIDTH-1 downto 0);
    read_zipcode_cmd_ready                     : in std_logic;
    read_zipcode_cmd_read_zipcode_values_addr  : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    read_zipcode_cmd_lastIdx                   : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_zipcode_cmd_firstIdx                  : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_last_name_unlock_valid                : in std_logic;
    read_last_name_unlock_tag                  : in std_logic_vector(TAG_WIDTH-1 downto 0);
    read_last_name_unlock_ready                : out std_logic;
    read_last_name_out_values_out_valid        : in std_logic;
    read_last_name_out_values_out_ready        : out std_logic;
    read_last_name_out_values_out_last         : in std_logic;
    read_last_name_out_values_out_dvalid       : in std_logic;
    read_last_name_out_values_out_data         : in std_logic_vector(7 downto 0);
    read_last_name_out_values_out_count        : in std_logic_vector(0 downto 0);
    read_last_name_out_valid                   : in std_logic;
    read_last_name_out_ready                   : out std_logic;
    read_last_name_out_length                  : in std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_last_name_out_last                    : in std_logic;
    read_last_name_cmd_valid                   : out std_logic;
    read_last_name_cmd_tag                     : out std_logic_vector(TAG_WIDTH-1 downto 0);
    read_last_name_cmd_ready                   : in std_logic;
    read_last_name_cmd_read_last_name_values_addr: out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    read_last_name_cmd_read_last_name_offsets_addr: out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    read_last_name_cmd_lastIdx                 : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_last_name_cmd_firstIdx                : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_first_name_unlock_valid               : in std_logic;
    read_first_name_unlock_tag                 : in std_logic_vector(TAG_WIDTH-1 downto 0);
    read_first_name_unlock_ready               : out std_logic;
    read_first_name_out_values_out_valid       : in std_logic;
    read_first_name_out_values_out_ready       : out std_logic;
    read_first_name_out_values_out_last        : in std_logic;
    read_first_name_out_values_out_dvalid      : in std_logic;
    read_first_name_out_values_out_data        : in std_logic_vector(7 downto 0);
    read_first_name_out_values_out_count       : in std_logic_vector(0 downto 0);
    read_first_name_out_valid                  : in std_logic;
    read_first_name_out_ready                  : out std_logic;
    read_first_name_out_length                 : in std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_first_name_out_last                   : in std_logic;
    read_first_name_cmd_valid                  : out std_logic;
    read_first_name_cmd_tag                    : out std_logic_vector(TAG_WIDTH-1 downto 0);
    read_first_name_cmd_ready                  : in std_logic;
    read_first_name_cmd_read_first_name_values_addr: out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    read_first_name_cmd_read_first_name_offsets_addr: out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    read_first_name_cmd_lastIdx                : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    read_first_name_cmd_firstIdx               : out std_logic_vector(INDEX_WIDTH-1 downto 0);
    -------------------------------------------------------------------------
    idx_last                                   : in std_logic_vector(REG_WIDTH-1 downto 0);
    idx_first                                  : in std_logic_vector(REG_WIDTH-1 downto 0);
    -------------------------------------------------------------------------
    ctrl_stop                                  : in std_logic;
    ctrl_start                                 : in std_logic;
    ctrl_reset                                 : in std_logic;
    ctrl_idle                                  : out std_logic;
    ctrl_done                                  : out std_logic;
    ctrl_busy                                  : out std_logic;
    -------------------------------------------------------------------------
    acc_reset                                  : in std_logic;
    acc_clk                                    : in std_logic
  );
end entity;

architecture Implementation of filter_usercore is
  signal read_cmd_valid       : std_logic;
  signal read_cmd_tag         : std_logic_vector(TAG_WIDTH-1 downto 0) := (others => '0');
  signal read_cmd_ready       : std_logic;
  signal read_cmd_lastIdx     : std_logic_vector(INDEX_WIDTH-1 downto 0);
  signal read_cmd_firstIdx    : std_logic_vector(INDEX_WIDTH-1 downto 0);
  
  signal read_unlock_valid    : std_logic;
  signal read_unlock_ready    : std_logic;
  
  type state_type is (IDLE, COMMAND, BUSY, DONE);
  
  type reg_record is record
    busy                      : std_logic;
    done                      : std_logic;
    reset_start               : std_logic;
    state                     : state_type;
  end record;
  
  signal r : reg_record;
  signal d : reg_record;
  
begin
  -- Connect all addresses
  read_zipcode_cmd_read_zipcode_values_addr          <= reg_read_zipcode_values_addr;
  read_last_name_cmd_read_last_name_values_addr      <= reg_read_last_name_values_addr;
  read_last_name_cmd_read_last_name_offsets_addr     <= reg_read_last_name_offsets_addr;
  read_first_name_cmd_read_first_name_values_addr    <= reg_read_first_name_values_addr;
  read_first_name_cmd_read_first_name_offsets_addr   <= reg_read_first_name_offsets_addr;
  write_first_name_cmd_write_first_name_values_addr  <= reg_write_first_name_values_addr;
  write_first_name_cmd_write_first_name_offsets_addr <= reg_write_first_name_offsets_addr;
  
  read_cmd_firstIdx <= idx_first;
  read_cmd_lastIdx <= idx_last;
  
  -- Give each command stream the same input
  read_first_name_cmd_tag       <= read_cmd_tag;
  read_last_name_cmd_tag        <= read_cmd_tag;
  read_zipcode_cmd_tag          <= read_cmd_tag;
  
  read_first_name_cmd_firstIdx  <= read_cmd_firstIdx;
  read_last_name_cmd_firstIdx   <= read_cmd_firstIdx;
  read_zipcode_cmd_firstIdx     <= read_cmd_firstIdx;
  
  read_first_name_cmd_lastIdx   <= read_cmd_lastIdx;
  read_last_name_cmd_lastIdx    <= read_cmd_lastIdx;
  read_zipcode_cmd_lastIdx      <= read_cmd_lastIdx;
  
  -- Synchronize the three command streams from the input columns
  read_cmd_sync : StreamSync
    generic map (
      NUM_INPUTS => 1,
      NUM_OUTPUTS => 3
    )
    port map (
      clk   => acc_clk,
      reset => acc_reset,
      in_valid(0) => read_cmd_valid,
      in_ready(0) => read_cmd_ready,
      out_valid(0) => read_first_name_cmd_valid,
      out_valid(1) => read_last_name_cmd_valid, 
      out_valid(2) => read_zipcode_cmd_valid, 
      out_ready(0) => read_first_name_cmd_ready,
      out_ready(1) => read_last_name_cmd_ready,
      out_ready(2) => read_zipcode_cmd_ready
    );
    
  read_first_name_unlock_ready <= '1';
  read_last_name_unlock_ready <= '1'; 
  read_zipcode_unlock_ready <= '1';
    
  -- Synchronize the three unlock streams from the input columns
  read_unlock_sync : StreamSync
    generic map (
      NUM_INPUTS => 3,
      NUM_OUTPUTS => 1
    )
    port map (
      clk   => acc_clk,
      reset => acc_reset,
      in_valid(0) => read_first_name_unlock_valid,
      in_valid(1) => read_last_name_unlock_valid,
      in_valid(2) => read_zipcode_unlock_valid,
      in_ready(0) => read_first_name_unlock_ready,
      in_ready(1) => read_last_name_unlock_ready,
      in_ready(2) => read_zipcode_unlock_ready,
      out_valid(0) => read_unlock_valid,
      out_ready(0) => read_unlock_ready
    );
    
  
  seq_proc: process(acc_clk) is
  begin
    if rising_edge(acc_clk) then
      r <= d;

      -- Reset
      if acc_reset = '1' then
        r.state         <= IDLE;
        r.reset_start   <= '0';
        r.busy          <= '0';
        r.done          <= '0';
      end if;
    end if;
  end process;
  
  comb_proc: process(all) 
  is
    variable v : reg_record;
  begin
    -- Default registered outputs:
    v := r;
    
    -- Default combinatorial outputs:
    read_cmd_valid <= '0';
    read_unlock_ready <= '0';
    
    case r.state is
      when IDLE =>
        ctrl_idle <= '1';
        ctrl_busy <= '0';
        ctrl_done <= '0';
        
        if ctrl_start = '1' then
          v.reset_start := '1';
          v.state := COMMAND;
          v.busy := '1';
          v.done := '0';
        end if;
      when COMMAND =>
        ctrl_idle <= '0';
        ctrl_busy <= '1';
        ctrl_done <= '0';
        
        read_cmd_valid    <= '1';
        
        if read_cmd_ready = '1' then
          v.state := BUSY;
        end if;
        
      when BUSY =>
        ctrl_idle <= '0';
        ctrl_busy <= '1';
        ctrl_done <= '0';
        
        read_unlock_ready <= '1';
        
        if read_unlock_valid = '1' then
          v.state := DONE;
        end if;
        
      when DONE =>
        ctrl_idle <= '0';
        ctrl_busy <= '0';
        ctrl_done <= '1';
      
    end case;
    
    d <= v;
    
  end process;
  
  read_zipcode_out_ready <= '1';
  read_first_name_out_ready <= '1';
  read_first_name_out_values_out_ready <= '1';
  read_last_name_out_ready <= '1';
  read_last_name_out_values_out_ready <= '1';

end architecture;
