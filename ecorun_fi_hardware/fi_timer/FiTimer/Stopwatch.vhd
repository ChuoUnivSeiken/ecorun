----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    18:47:55 03/25/2015 
-- Design Name: 
-- Module Name:    Stopwatch - RTL 
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

entity Stopwatch is
	port(
		clk : in std_logic;
		stamp_and_reset : in std_logic;
		time_stamp : out std_logic_vector(7 downto 0)
		);
end Stopwatch;

architecture RTL of Stopwatch is
	signal counter : std_logic_vector(9 downto 0) := (others => '0');
	type sw_state is (idle, should_stamp, should_reset, counting);
	signal state : sw_state := idle;
begin
	process(clk, state, counter, stamp_and_reset) begin
		if (stamp_and_reset = '1') then
			state <= should_stamp; -- パルスが立ち下がり、次のクロックでタイムスタンプ、更に次のクロックでリセット
		else
			if (rising_edge(clk)) then
				if (state = should_stamp) then
					time_stamp <= counter(9 downto 2);
					state <= should_reset;
				elsif (state = should_reset) then
					counter <= (others => '0');
					state <= counting;
				elsif (counter = "1111111111") then
					counter <= (others => '0');
					state <= idle;
				elsif (state = counting) then
					counter <= counter + 1;
				else
					counter <= counter;
				end if;
			end if;
		end if;
	end process;

end RTL;

