# Setup global configs
set_property CFGBVS Vcco [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]

# System clock
create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports { CLK_100M }];

# 10MHz JTAG/SWD clock
create_clock -add -name jtag_clk -period 200.00 -waveform {0 100} [get_ports { TCK_SWDCLK }];

# Create virtual clock for IO
create_clock -name slow_clk -period 1000.0

# Set transport clock as async to hclk
set_clock_groups -asynchronous -group transport_clk -group hclk -group jtag_clk
