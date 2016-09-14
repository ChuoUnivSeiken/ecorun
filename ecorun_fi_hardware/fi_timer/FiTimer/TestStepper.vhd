--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   18:02:15 09/10/2016
-- Design Name:   
-- Module Name:   C:/Users/Yoshio/git/ecorun/ecorun_fi_hardware/fi_timer/FiTimer/TestStepper.vhd
-- Project Name:  FiTimer
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Stepper
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
 
ENTITY TestStepper IS
END TestStepper;
 
ARCHITECTURE behavior OF TestStepper IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Stepper
    PORT(
         iac_pulse : IN  std_logic;
         iac_clockwise : IN  std_logic;
         iac_out : OUT  std_logic_vector(7 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal iac_pulse : std_logic := '0';
   signal iac_clockwise : std_logic := '0';

 	--Outputs
   signal iac_out : std_logic_vector(7 downto 0);
   -- No clocks detected in port list. Replace <clock> below with 
   -- appropriate port name 
 
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Stepper PORT MAP (
          iac_pulse => iac_pulse,
          iac_clockwise => iac_clockwise,
          iac_out => iac_out
        );

   -- Clock process definitions
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for clk_period*10;

      -- insert stimulus here
		
		iac_clockwise <= '1';
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;	
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;		
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;		
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;		
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;		
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;		
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;		
		
		iac_pulse <= '1';
      wait for 10 ns;	
		iac_pulse <= '0';
      wait for 10 ns;	
		

      wait;
   end process;

END;
