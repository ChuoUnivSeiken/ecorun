--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   03:39:23 08/26/2016
-- Design Name:   
-- Module Name:   C:/Users/Yoshio/git/ecorun/ecorun_fi_hardware/fi_timer/FiTimer/TestPulseTimer.vhd
-- Project Name:  FiTimer
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: PulseTimer
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TestPulseTimer IS
END TestPulseTimer;
 
ARCHITECTURE behavior OF TestPulseTimer IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT PulseTimer
    PORT(
         clk : IN  std_logic;
         enable : IN  std_logic;
         start : IN  std_logic;
         match : IN  std_logic_vector(7 downto 0);
         pulse : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal enable : std_logic := '0';
   signal start : std_logic := '0';
   signal match : std_logic_vector(7 downto 0) := (others => '0');

 	--Outputs
   signal pulse : std_logic;

   -- Clock period definitions
   constant clk_period : time := 100 us;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: PulseTimer PORT MAP (
          clk => clk,
          enable => enable,
          start => start,
          match => match,
          pulse => pulse
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for clk_period*10;

      -- insert stimulus here 
		
		enable <= '1';
		match <= "00000100";
		
		wait for clk_period;
		
		start <= '1';
		
		wait for clk_period;
		
		start <= '0';
		
      wait for 1 ms;	

      wait;
   end process;

END;
