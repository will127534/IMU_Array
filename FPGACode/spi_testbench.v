`timescale 1ns/1ns

`define SECOND 1000000000
`define MS 1000000



module SPI_TestBench();
    wire [7:0] data_in;
    reg [7:0] data_out;
    initial data_out = 0;

    reg [7:0]testdata;
    initial testdata = 8'h1275;

    reg clock;
    initial clock = 0;

    always #(1) clock <= ~clock;
    reg rst;

    reg SCK,MO,CS;
    wire MI;
    initial SCK = 0;
    initial MO = 0;
    initial CS = 1;

    reg [31:0] IMU_MISO;
    initial IMU_MISO = 32'b0;

    spi_slave rpi_spi_dev(
        .sclk(SCK),            // SPI clock
        .mosi(MO),            // Master Out Slave In
        .miso(MI),           // Master In Slave Out

        .IMU_MISO1(IMU_MISO[0]),
        .IMU_MISO2(IMU_MISO[1]),
        .IMU_MISO3(IMU_MISO[2]),
        .IMU_MISO4(IMU_MISO[3]),
        .IMU_MISO5(IMU_MISO[4]),
        .IMU_MISO6(IMU_MISO[5]),
        .IMU_MISO7(IMU_MISO[6]),
        .IMU_MISO8(IMU_MISO[7]),
        .IMU_MISO9(IMU_MISO[8]),
        .IMU_MISO10(IMU_MISO[9]),
        .IMU_MISO11(IMU_MISO[10]),
        .IMU_MISO12(IMU_MISO[11]),
        .IMU_MISO13(IMU_MISO[12]),
        .IMU_MISO14(IMU_MISO[13]),
        .IMU_MISO15(IMU_MISO[14]),
        .IMU_MISO16(IMU_MISO[15]),
        .IMU_MISO17(IMU_MISO[16]),
        .IMU_MISO18(IMU_MISO[17]),
        .IMU_MISO19(IMU_MISO[18]),
        .IMU_MISO20(IMU_MISO[19]),
        .IMU_MISO21(IMU_MISO[20]),
        .IMU_MISO22(IMU_MISO[21]),
        .IMU_MISO23(IMU_MISO[22]),
        .IMU_MISO24(IMU_MISO[23]),
        .IMU_MISO25(IMU_MISO[24]),
        .IMU_MISO26(IMU_MISO[25]),
        .IMU_MISO27(IMU_MISO[26]),
        .IMU_MISO28(IMU_MISO[27]),
        .IMU_MISO29(IMU_MISO[28]),
        .IMU_MISO30(IMU_MISO[29]),
        .IMU_MISO31(IMU_MISO[30]),
        .IMU_MISO32(IMU_MISO[31]),

        .cs_n(CS),        
        .LED_CMD(),         // LED for command processing state
        .LED_LOAD(),        // LED for data loading state
        .LED_SEND()        // LED for data sending state
    );

    initial begin
    $dumpfile("SPI_TestBench.vcd"); 
    $dumpvars(0, rpi_spi_dev);

    rst <= 1;
    #1
    rst <= 0;
    #1
    rst <= 1;

    #20
    CS <= 0;
    #1
    SCK <= 0;

    #3
    MO <= 1;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 1;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 1;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;


    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;


    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    IMU_MISO <= 32'b11111111111111111111111111111111;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    CS <= 1;
    #3
    SCK <= 0;


    #200


    #20
    CS <= 0;
    #1
    SCK <= 0;

    #3
    MO <= 1;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 1;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    MO <= 0;
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;

    #200

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;

    #3
    SCK <= 0;
    #3
    SCK <= 1;


    #3
    CS <= 1;
    #3
    SCK <= 0;


    #200
    $finish;
    end
 
endmodule