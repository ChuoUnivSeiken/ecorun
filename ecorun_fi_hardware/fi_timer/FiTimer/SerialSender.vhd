----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    17:02:23 03/15/2015 
-- Design Name: 
-- Module Name:    SerialSender - RTL 
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
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity SerialSender is
	port(
		clk : in std_logic;
		tx : out std_logic := '1';
		data : in std_logic_vector(7 downto 0);
		send : in std_logic;
		sending : out std_logic := '0'
		);
end SerialSender;

architecture RTL of SerialSender is
	signal temp_data : std_logic_vector(7 downto 0);
	signal bit_pos : integer range 0 to 9 := 0;
	signal sending : std_logic := '0';
begin
	process(clk) begin
		if (clk'event and clk = '1') then
			-- serial send
			if (send = '1') then
				sending <= send;
			end if;
			if (sending = '1') then
				case bit_pos is
					when 0 =>
						tx <= '0';
						bit_pos <= bit_pos + 1;
						temp_data <= data;
					when 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 =>
						bit_pos <= bit_pos + 1;
						tx <= temp_data(0);
						temp_data <= '1' & temp_data(7 downto 1);
					when 9 =>
						tx <= '1';
						bit_pos <= 0;
						sending <= '0';
				end case;
			end if;
			out_sending <= sending;
		end if;
	end process;

end RTL;

