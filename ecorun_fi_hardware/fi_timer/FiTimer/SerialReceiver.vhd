----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:49:01 03/15/2015 
-- Design Name: 
-- Module Name:    SerialReceiver - RTL 
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

entity SerialReceiver is
	port(
		clk : in std_logic;
		rx : in std_logic := '1';
		data : out std_logic_vector(7 downto 0) := (others => '0')
		);
end SerialReceiver;

architecture RTL of SerialReceiver is
	type state_type is (idle, receiving, stopbit);
	signal state : state_type := idle;
	signal temp_data : std_logic_vector(7 downto 0);
	signal bit_pos : integer range 0 to 7 := 0;
begin
	process(clk) begin
		if (clk'event and clk = '1') then
			-- serial receive
			-- ストップビットでcounter_matchが更新
			if ((state = idle) and rx = '0') then
				bit_pos <= 0;
				state <= receiving;
			elsif (state = receiving) then
				temp_data <= rx & temp_data(7 downto 1);
				bit_pos <= bit_pos + 1;
				if (bit_pos = 7) then
					state <= stopbit;
				end if;
			elsif (state = stopbit) then
				if (rx = '1') then
					data <= temp_data;
					state <= idle;
				end if;
			end if;
		end if;
	end process;
end RTL;

