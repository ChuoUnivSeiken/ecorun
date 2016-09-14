----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    17:10:19 03/15/2015 
-- Design Name: 
-- Module Name:    PulseTimer - RTL 
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

entity PulseTimer is
	port(
		clk : in std_logic;
		enable : in std_logic;
		start : in std_logic;
		match : in std_logic_vector(7 downto 0);
		pulse : out std_logic := '0'
	);
end PulseTimer;

architecture RTL of PulseTimer is
	signal counter : std_logic_vector(7 downto 0) := (others => '0');
	signal enabled : boolean := false;
	signal match_cache : std_logic_vector(7 downto 0);
begin
	process(clk, enable, start, match) begin
		if (start = '1' and enable = '1') then
			enabled <= true;
		elsif rising_edge(clk) then
			if (enabled) then
				if (counter = "00000000") then
					match_cache <= match;
					counter <= counter + 1;
				elsif (counter = match_cache) then
					pulse <= '0';
					enabled <=  false;
					counter <= (others => '0');
				else
					pulse <= '1';
					counter <= counter + 1;
				end if;
			else
				pulse <= '0';
				counter <= (others => '0');
			end if;
		end if;
	end process;
end RTL;

