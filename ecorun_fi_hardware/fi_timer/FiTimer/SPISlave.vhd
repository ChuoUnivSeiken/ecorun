----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    01:21:33 05/14/2015 
-- Design Name: 
-- Module Name:    SPISlave - Behavioral 
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

entity SPISlave is
port(
		sck : in std_logic := '0';
		mosi : in std_logic;
		miso : out std_logic := 'Z';
		ssel : in std_logic;
		in_data : in std_logic_vector(7 downto 0);
		out_data : out std_logic_vector(7 downto 0)
		);
end SPISlave;

architecture Behavioral of SPISlave is
signal first: boolean := true;
signal reg: std_logic_vector(6 downto 0) := (others => '0');
signal mosi_cache: std_logic := '0';
begin
process(sck, ssel, reg, mosi_cache, in_data) begin
	if (rising_edge(ssel)) then
		out_data <= reg(6 downto 0) & mosi_cache;
	end if;
	
	if (ssel = '1') then
		first <= true;
		miso <= 'Z';
	else
		if (rising_edge(sck)) then
			mosi_cache <= mosi;
		end if;
		
		if (falling_edge(sck)) then
			if (first) then
				first <= false;
				miso <= in_data(7);
				reg <= in_data(6 downto 0);
			else
				miso <= reg(6);
				reg <= reg(5 downto 0) & mosi_cache;
			end if;
		end if;
	end if;
end process;
end Behavioral;

