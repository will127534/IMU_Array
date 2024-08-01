module spi_slave (
    input wire sclk,            // SPI clock
    input wire mosi,            // Master Out Slave In
    output wire miso,           // Master In Slave Out

    input wire IMU_MISO1,
    input wire IMU_MISO2,
    input wire IMU_MISO3,
    input wire IMU_MISO4,
    input wire IMU_MISO5,
    input wire IMU_MISO6,
    input wire IMU_MISO7,
    input wire IMU_MISO8,
    input wire IMU_MISO9,
    input wire IMU_MISO10,
    input wire IMU_MISO11,
    input wire IMU_MISO12,
    input wire IMU_MISO13,
    input wire IMU_MISO14,
    input wire IMU_MISO15,
    input wire IMU_MISO16,
    input wire IMU_MISO17,
    input wire IMU_MISO18,
    input wire IMU_MISO19,
    input wire IMU_MISO20,
    input wire IMU_MISO21,
    input wire IMU_MISO22,
    input wire IMU_MISO23,
    input wire IMU_MISO24,
    input wire IMU_MISO25,
    input wire IMU_MISO26,
    input wire IMU_MISO27,
    input wire IMU_MISO28,
    input wire IMU_MISO29,
    input wire IMU_MISO30,
    input wire IMU_MISO31,
    input wire IMU_MISO32,

    input wire cs_n,            // Chip select, active low
    output reg LED_CMD,         // LED for command processing state
    output reg LED_LOAD,        // LED for data loading state
    output reg LED_SEND         // LED for data sending state
);

    // State encoding
    parameter IDLE = 2'b00;
    parameter CMD = 2'b01;
    parameter LOAD = 2'b10;
    parameter SEND = 2'b11;

    reg [1:0] current_state, next_state;

    reg [7:0] cmd_byte;
    reg [3:0] bit_counter;
    reg [9:0] byte_counter;
    reg [7:0] shift_register;

    reg [4095:0] miso_shift_register;
    reg sclk_reg, sclk_prev;

    assign miso = miso_shift_register[4095];

    // State transition and LED control on system clock
    always @(negedge sclk or posedge cs_n) begin
        if (cs_n) begin
            current_state <= IDLE;
            LED_CMD <= 1'b0;
            LED_LOAD <= 1'b0;
            LED_SEND <= 1'b0;
        end else begin
            current_state <= next_state;
            case (next_state)
                IDLE: begin
                    LED_CMD <= 1'b0;
                    LED_LOAD <= 1'b0;
                    LED_SEND <= 1'b0;
                end
                CMD: begin
                    LED_CMD <= 1'b1;
                    LED_LOAD <= 1'b0;
                    LED_SEND <= 1'b0;
                end
                LOAD: begin
                    LED_CMD <= 1'b0;
                    LED_LOAD <= 1'b1;
                    LED_SEND <= 1'b0;
                end
                SEND: begin
                    LED_CMD <= 1'b0;
                    LED_LOAD <= 1'b0;
                    LED_SEND <= 1'b1;
                end
            endcase
        end
    end

    // Next state logic
    always @(*) begin
        // Default values
        next_state = current_state;

        case (current_state)
            IDLE: begin
                if (!cs_n) begin
                    next_state = CMD;
                end
            end
            CMD: begin
                if (cmd_get == 1'b1) begin
                    if (cmd_byte == 8'hB0) begin
                        next_state = LOAD;
                    end else begin
                        next_state = SEND;
                    end
                end
            end
            LOAD: begin
                if (byte_counter == 5'b10000) begin
                    next_state = IDLE;
                end
            end
            SEND: begin
                if (byte_counter == 10'd512) begin
                    next_state = IDLE;
                end
            end
        endcase
    end

    reg start_send;
    reg cmd_get;
    reg first_mosi_bit;

    // Data processing on SPI clock edge
    always @(posedge sclk or posedge cs_n) begin
        if (cs_n) begin
            cmd_byte <= 8'b0;
            bit_counter <= 4'b0;
            byte_counter <= 5'b0;
            shift_register <= 8'b0;
            start_send <= 1'b0;
            cmd_get <= 1'b0;
            first_mosi_bit <= 1'b0;
        end 
        else begin
            if (sclk) begin
                case (current_state)
                    IDLE: begin
                        first_mosi_bit <= mosi;
                    end
                    CMD: begin
                        if (bit_counter == 4'b0111) begin
                            bit_counter <= 4'b0;
                            cmd_get <= 1'b1;
                            cmd_byte <= {shift_register[6:0], mosi};
                        end else if (bit_counter == 4'b0) begin
                            bit_counter <= bit_counter + 2;
                            shift_register <= {shift_register[5:0], first_mosi_bit, mosi};
                        end else begin
                            bit_counter <= bit_counter + 1;
                            shift_register <= {shift_register[6:0], mosi};
                        end
                    end
                    LOAD: begin
                        miso_shift_register[127:0] <= {miso_shift_register[126:0], IMU_MISO1};
                        miso_shift_register[255:128] <= {miso_shift_register[254:128], IMU_MISO2};
                        miso_shift_register[383:256] <= {miso_shift_register[382:256], IMU_MISO3};
                        miso_shift_register[511:384] <= {miso_shift_register[510:384], IMU_MISO4};
                        miso_shift_register[639:512] <= {miso_shift_register[638:512], IMU_MISO5};
                        miso_shift_register[767:640] <= {miso_shift_register[766:640], IMU_MISO6};
                        miso_shift_register[895:768] <= {miso_shift_register[894:768], IMU_MISO7};
                        miso_shift_register[1023:896] <= {miso_shift_register[1022:896], IMU_MISO8};
                        miso_shift_register[1151:1024] <= {miso_shift_register[1150:1024], IMU_MISO9};
                        miso_shift_register[1279:1152] <= {miso_shift_register[1278:1152], IMU_MISO10};
                        miso_shift_register[1407:1280] <= {miso_shift_register[1406:1280], IMU_MISO11};
                        miso_shift_register[1535:1408] <= {miso_shift_register[1534:1408], IMU_MISO12};
                        miso_shift_register[1663:1536] <= {miso_shift_register[1662:1536], IMU_MISO13};
                        miso_shift_register[1791:1664] <= {miso_shift_register[1790:1664], IMU_MISO14};
                        miso_shift_register[1919:1792] <= {miso_shift_register[1918:1792], IMU_MISO15};
                        miso_shift_register[2047:1920] <= {miso_shift_register[2046:1920], IMU_MISO16};
                        miso_shift_register[2175:2048] <= {miso_shift_register[2174:2048], IMU_MISO17};
                        miso_shift_register[2303:2176] <= {miso_shift_register[2302:2176], IMU_MISO18};
                        miso_shift_register[2431:2304] <= {miso_shift_register[2430:2304], IMU_MISO19};
                        miso_shift_register[2559:2432] <= {miso_shift_register[2558:2432], IMU_MISO20};
                        miso_shift_register[2687:2560] <= {miso_shift_register[2686:2560], IMU_MISO21};
                        miso_shift_register[2815:2688] <= {miso_shift_register[2814:2688], IMU_MISO22};
                        miso_shift_register[2943:2816] <= {miso_shift_register[2942:2816], IMU_MISO23};
                        miso_shift_register[3071:2944] <= {miso_shift_register[3070:2944], IMU_MISO24};
                        miso_shift_register[3199:3072] <= {miso_shift_register[3198:3072], IMU_MISO25};
                        miso_shift_register[3327:3200] <= {miso_shift_register[3326:3200], IMU_MISO26};
                        miso_shift_register[3455:3328] <= {miso_shift_register[3454:3328], IMU_MISO27};
                        miso_shift_register[3583:3456] <= {miso_shift_register[3582:3456], IMU_MISO28};
                        miso_shift_register[3711:3584] <= {miso_shift_register[3710:3584], IMU_MISO29};
                        miso_shift_register[3839:3712] <= {miso_shift_register[3838:3712], IMU_MISO30};
                        miso_shift_register[3967:3840] <= {miso_shift_register[3966:3840], IMU_MISO31};
                        miso_shift_register[4095:3968] <= {miso_shift_register[4094:3968], IMU_MISO32};

                    end
                    SEND: begin
                        if (start_send == 1'b1) begin
                            miso_shift_register <= {miso_shift_register[4094:0], 1'b0}; // Shift left
                        end else begin
                            start_send <=  1'b1;
                        end
                    end
                endcase
            end
        end 
    end
endmodule


