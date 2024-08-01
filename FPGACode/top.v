`default_nettype none

module main (
    //LEDs
    output LEDR,
    output LEDG,
    output LEDB,

    //SPI to PICO
    input  MOSI,
    input  CS,
    input  SCK,
    output MISO,

    input IMU_MISO1,
    input IMU_MISO2,
    input IMU_MISO3,
    input IMU_MISO4,
    input IMU_MISO5,
    input IMU_MISO6,
    input IMU_MISO7,
    input IMU_MISO8,
    input IMU_MISO9,
    input IMU_MISO10,
    input IMU_MISO11,
    input IMU_MISO12,
    input IMU_MISO13,
    input IMU_MISO14,
    input IMU_MISO15,
    input IMU_MISO16,
    input IMU_MISO17,
    input IMU_MISO18,
    input IMU_MISO19,
    input IMU_MISO20,
    input IMU_MISO21,
    input IMU_MISO22,
    input IMU_MISO23,
    input IMU_MISO24,
    input IMU_MISO25,
    input IMU_MISO26,
    input IMU_MISO27,
    input IMU_MISO28,
    input IMU_MISO29,
    input IMU_MISO30,
    input IMU_MISO31,
    input IMU_MISO32


);
      
  wire R,G,B;

  spi_slave spi_dev(
    .sclk(SCK),
    .mosi(MOSI),
    .miso(MISO),
    .cs_n(CS),

    .IMU_MISO1(IMU_MISO1),
    .IMU_MISO2(IMU_MISO2),
    .IMU_MISO3(IMU_MISO3),
    .IMU_MISO4(IMU_MISO4),
    .IMU_MISO5(IMU_MISO5),
    .IMU_MISO6(IMU_MISO6),
    .IMU_MISO7(IMU_MISO7),
    .IMU_MISO8(IMU_MISO8),
    .IMU_MISO9(IMU_MISO9),
    .IMU_MISO10(IMU_MISO10),
    .IMU_MISO11(IMU_MISO11),
    .IMU_MISO12(IMU_MISO12),
    .IMU_MISO13(IMU_MISO13),
    .IMU_MISO14(IMU_MISO14),
    .IMU_MISO15(IMU_MISO15),
    .IMU_MISO16(IMU_MISO16),
    .IMU_MISO17(IMU_MISO17),
    .IMU_MISO18(IMU_MISO18),
    .IMU_MISO19(IMU_MISO19),
    .IMU_MISO20(IMU_MISO20),
    .IMU_MISO21(IMU_MISO21),
    .IMU_MISO22(IMU_MISO22),
    .IMU_MISO23(IMU_MISO23),
    .IMU_MISO24(IMU_MISO24),
    .IMU_MISO25(IMU_MISO25),
    .IMU_MISO26(IMU_MISO26),
    .IMU_MISO27(IMU_MISO27),
    .IMU_MISO28(IMU_MISO28),
    .IMU_MISO29(IMU_MISO29),
    .IMU_MISO30(IMU_MISO30),
    .IMU_MISO31(IMU_MISO31),
    .IMU_MISO32(IMU_MISO32),

    .LED_CMD(R),         // LED for command processing state
    .LED_LOAD(G),        // LED for data loading state
    .LED_SEND(B)         // LED for data sending state
  );


  SB_IO_OD #(
    .PIN_TYPE(6'b011001),
    .NEG_TRIGGER(1'b0)
  ) pin_out_driverB (
    .PACKAGEPIN(LEDB),
    .DOUT0(~B)
  );
  SB_IO_OD #(
    .PIN_TYPE(6'b011001),
    .NEG_TRIGGER(1'b0)
  ) pin_out_driverR (
    .PACKAGEPIN(LEDR),
    .DOUT0(~R)
  );
  SB_IO_OD #(
    .PIN_TYPE(6'b011001),
    .NEG_TRIGGER(1'b0)
  ) pin_out_driverG (
    .PACKAGEPIN(LEDG),
    .DOUT0(~G)
  );

endmodule