----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    19:49:32 06/15/2015 
-- Design Name: 
-- Module Name:    Stepper - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity Stepper is
	port(
		iac_pulse : in std_logic := '0';
		iac_clockwise : in std_logic := '0';
		iac_out : out std_logic_vector(7 downto 0) := (others => '0')
		);
end Stepper;

architecture Behavioral of Stepper is
	signal phase : integer range 0 to 3 := 0;
begin
	process(iac_pulse, phase) begin
		if (falling_edge(iac_pulse)) then
			if (iac_clockwise = '1') then
				phase <= phase + 1;
			else
				phase <= phase - 1;
			end if;
		end if;
	
		case phase is
         when 0  => iac_out <= "10010000" and (7 downto 0 => iac_pulse);
         when 1  => iac_out <= "00001001" and (7 downto 0 => iac_pulse);
         when 2  => iac_out <= "01100000" and (7 downto 0 => iac_pulse);
         when 3  => iac_out <= "00000110" and (7 downto 0 => iac_pulse);
			when others => iac_out <= (others => '0');
      end case;
	end process;
end Behavioral;

