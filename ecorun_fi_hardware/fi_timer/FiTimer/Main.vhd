----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    18:33:37 03/14/2015 
-- Design Name: 
-- Module Name:    Main - RTL 
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

-- spec
-- 供給クロック : 50kHz
-- 噴射精度 : 0.02ms
-- 最大噴射可能時間 : 5.1ms
-- 検出可能最低回転数 : 1700rpm

entity Main is
	port(
		clk : in std_logic;
		enable : in std_logic;
		timing_pulse : in std_logic;
		fi_pulse : out std_logic;
		sck: in std_logic;
		ssel: in std_logic;
		mosi : in std_logic;
		miso : out std_logic;
		iac_pulse : in std_logic := '0';
		iac_clockwise : in std_logic := '0';
		iac_out : out std_logic_vector(7 downto 0) := (others => '0');
		cpu_con : inout std_logic_vector(1 downto 0) := (others => '0'));
end Main;

architecture RTL of Main is
	
	component SerialReceiver
		port(
			clk : in std_logic;
			rx : in std_logic;
			data : out std_logic_vector(7 downto 0)
		);
	end component;
	
	component SerialSender
		port(
			clk : in std_logic;
			tx : out std_logic;
			data : in std_logic_vector(7 downto 0);
			send : in std_logic;
			sending : out std_logic := '0'
		);
	end component;
	
	component PulseTimer
		port(
			clk : in std_logic;
			enable : in std_logic;
			start : in std_logic;
			match : in std_logic_vector(7 downto 0);
			pulse : out std_logic
		);
	end component;
	
	component Stopwatch
		port(
			clk : in std_logic;
			stamp_and_reset : in std_logic;
			time_stamp : out std_logic_vector(7 downto 0)
		);
	end component;
	
	component SPISlave
		port(
			sck : in std_logic;
			mosi : in std_logic;
			miso : out std_logic;
			ssel : in std_logic;
			in_data : in std_logic_vector(7 downto 0);
			out_data : out std_logic_vector(7 downto 0)
		);
	end component;
	
	component Stepper
		port(
			iac_pulse : in std_logic := '0';
			iac_clockwise : in std_logic := '0';
			iac_out : out std_logic_vector(7 downto 0) := (others => '0')
		);
	end component;
	
	signal counter_match : std_logic_vector(7 downto 0) := (others => '0');
	signal time_stamp : std_logic_vector(7 downto 0);
	
begin

	--cpu_con(0) <= timing_pulse;

	st: Stepper port map (
		iac_pulse => iac_pulse,
		iac_clockwise => iac_clockwise,
		iac_out => iac_out
	);
	
	sw: Stopwatch port map (
		clk => clk,
		stamp_and_reset => timing_pulse,
		time_stamp => time_stamp
	);
	
	spi: SPISlave port map (
		sck => sck,
		mosi => mosi,
		miso => miso,
		ssel => ssel,
		out_data => counter_match,
		in_data => time_stamp
		--in_data => std_logic_vector(to_unsigned(123, 8))
	);
	
	fi_pulse_gen: PulseTimer port map (
		clk => clk,
		enable => enable,
		start => timing_pulse,
		match => counter_match,
		pulse => fi_pulse
	);
	
end RTL;

