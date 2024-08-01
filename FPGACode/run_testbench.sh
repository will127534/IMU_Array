iverilog -o test ./spi_slave.v ./spi_testbench.v
vvp ./test
gtkwave  ./SPI_TestBench.vcd