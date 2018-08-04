lappend signals "TOP.anton_neopixel_apb.apbPclk"
lappend signals "TOP.clk7mhz"
lappend signals "TOP.neoData"
lappend signals "TOP.neoState"
lappend signals "TOP.pixelsSync"

lappend signals "TOP.anton_neopixel_apb.neopixel.reset_delay"
lappend signals "TOP.anton_neopixel_apb.neopixel.reset_delay_count"
lappend signals "TOP.anton_neopixel_apb.neopixel.bit_pattern_index"
lappend signals "TOP.anton_neopixel_apb.neopixel.neo_pattern_lookup"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixel_bit_index"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixel_index"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixel_index_equiv"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixel_value_8bit"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixel_value_32bit"

lappend signals "TOP.anton_neopixel_apb.wr_enable"
lappend signals "TOP.anton_neopixel_apb.apbPwData"
lappend signals "TOP.anton_neopixel_apb.apbPaddr"
lappend signals "TOP.anton_neopixel_apb.address"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixels(0)"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixels(1)"
lappend signals "TOP.anton_neopixel_apb.neopixel.pixels(2)"
lappend signals "TOP.anton_neopixel_apb.pixelsSynch"

lappend signals "TOP.anton_neopixel_apb.neopixel.registers(0)"
lappend signals "TOP.anton_neopixel_apb.neopixel.registers(1)"

lappend signals "TOP.anton_neopixel_apb.neopixel.reg_ctrl_init"
lappend signals "TOP.anton_neopixel_apb.neopixel.reg_ctrl_run"
lappend signals "TOP.anton_neopixel_apb.neopixel.reg_ctrl_loop"
lappend signals "TOP.anton_neopixel_apb.neopixel.reg_ctrl_32bit"
lappend signals "TOP.anton_neopixel_apb.neopixel.reg_state_reset"
lappend signals "TOP.anton_neopixel_apb.neopixel.reg_state_off"

lappend signals "TOP.anton_neopixel_apb.neopixel.reset_reg_ctrl_run"

#lappend signals "TOP.anton_neopixel_apb.neopixel.pixels"

set num_added [ gtkwave::addSignalsFromList $signals ]
