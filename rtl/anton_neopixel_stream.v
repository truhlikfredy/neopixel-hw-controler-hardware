`include "anton_common.vh"

//TODO: Index naming could shortened to Ix and Buf to B (or use SV and do not use buf at all)

module anton_neopixel_stream #(
  parameter  BUFFER_END  = `BUFFER_END_DEFAULT, // read anton_common.vh
  localparam BUFFER_BITS = `CLOG2(BUFFER_END+1) // minimum required amount of bits to store the BUFFER_END
)(
  input [7:0]              pixels[BUFFER_END:0],
  input                    state,
  input [BUFFER_BITS-1:0]  pixelIndex,     // index to the current pixel transmitting
  input [4:0]              pixelBitIndex,  // 0 - 23 to count whole 24bits of a RGB pixel
  input [2:0]              bitPatternIndex,
  input                    regCtrl32bit,
  input                    regCtrlRun,

  output                   neoData
);

  reg [7:0]  neoPatternLookup = 'd0;  // move to wire
  reg        neoDataBuf       = 'b0;
  reg [23:0] pixelColourValue = 'd0;  // Blue Red Green, order is from right to left and the MSB are sent first

  // Pixel values for 32bit RGB mode
  wire [7:0] pRed   = pixels[{pixelIndex[BUFFER_BITS-1: 2], 2'b00}];
  wire [7:0] pGreen = pixels[{pixelIndex[BUFFER_BITS-1: 2], 2'b01}];
  wire [7:0] pBlue  = pixels[{pixelIndex[BUFFER_BITS-1: 2], 2'b10}];

  // as combinational logic should be enough
  // https://electronics.stackexchange.com/questions/29553/how-are-verilog-always-statements-implemented-in-hardware
  always @(*) begin
    // depending on the current bit decide what pattern to push
    // patterns are ordered from right to left
    if (pixelColourValue[pixelBitIndex]) begin
      neoPatternLookup = 8'b00011111;
    end else begin
      neoPatternLookup = 8'b00000011;
    end
  end


  always @(*) begin
    `ifdef HARDCODED_PIXELS
      // hardcoded predefined colours for 3 pixels in a strip
      // TODO: use casez so bigger arrays could be auto filled with these 
      // values in tiling/overflow method
      case (pixelIndex)
        'd0:      pixelColourValue = 24'hff00d5;
        'd1:      pixelColourValue = 24'h008800;
        'd2:      pixelColourValue = 24'h000090;
        'd3:      pixelColourValue = 24'h000010;
        default:  pixelColourValue = 24'h101010; // slightly light to show there might be problem in configuration
      endcase
    `else
      if (regCtrl32bit) begin
        // In 32bit mode use 3 bytes to concatenate RGB values and reordered 
        // them to make it convenient (4th byte is dropped)
        pixelColourValue = { 
          pBlue[0],  pBlue[1],  pBlue[2],  pBlue[3],  pBlue[4],  pBlue[5],  pBlue[6],  pBlue[7], // Blue
          pRed[0],   pRed[1],   pRed[2],   pRed[3],   pRed[4],   pRed[5],   pRed[6],   pRed[7],  // Red
          pGreen[0], pGreen[1], pGreen[2], pGreen[3], pGreen[4], pGreen[5], pGreen[6], pGreen[7] // Green          
        };
      end else begin
        // 8bit mode
        // 2B, 3G, 3R = 8bit source format       => [7:6]Blue,  [5:3]Green, [2:0]Red
        // 8B, 8R, 8G = 32bit destination format =>  xxxxBxxB xxRxRxxR xxGxGxGx  high bits are sent first (so reorder them to the right)
        pixelColourValue = { 
          4'b0000, pixels[pixelIndex][6], 2'b00, pixels[pixelIndex][7],                                    // 2bits Blues
          2'b00,   pixels[pixelIndex][0], 1'b0,  pixels[pixelIndex][1], 2'b00, pixels[pixelIndex][2],      // 3bits Red
          2'b00,   pixels[pixelIndex][3], 1'b0,  pixels[pixelIndex][4], 1'b0,  pixels[pixelIndex][5], 1'b0 // 3bits Green
        };
      end
    `endif
  end


  always @(*) begin
    if (state == `ENUM_STATE_TRANSMIT && regCtrlRun) begin
      // push pattern of a single bit inside a pixel 
      neoDataBuf = neoPatternLookup[bitPatternIndex[2:0]];
    end else begin
      // reset state, stay LOW
      neoDataBuf = 'd0;
    end
  end
  
   
  assign neoData = neoDataBuf;
  
endmodule