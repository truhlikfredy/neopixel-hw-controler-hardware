#include <verilated.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>

#include "neopixel_simulation.h"
#include "neopixel_driver.h"
#include "neopixel_hal.h"

#define CTRL_INIT  1
#define CTRL_LIMIT 2
#define CTRL_RUN   4
#define CTRL_LOOP  8
#define CTRL_32    16

#define STATE_RESET 1
#define STATE_OFF   2

Vanton_neopixel_apb_top *uut;
VerilatedVcdC *tfp;
vluint64_t sim_time;
NeoPixelDriver *driver;

// 3 simulation steps are quired to for 100ns in simulation to pass.
// Each simulation step is 25units, so 75units means 100ns, therefore 750=1us.
// stop the simulation if it didn't ended after 3ms (3000us)
#define SIMULATION_NOT_STUCK (sim_time < (3000 * (75 * 10)))

double sc_time_stamp() {
  return sim_time*50;
}

void simulationDone() {
  // Done simulating
  uut->final();

  std::cout << "Simulation finished with " << sim_time << " timestamp." << std::endl;

#if VM_COVERAGE
  VerilatedCov::writeLcov("lcov.info");
#endif

  tfp->close();
  delete tfp;
  delete uut;
}

template <typename T> void assert_equals(std::string text, T expected, T actual) {
  if (expected != actual) {
    std::cout << "FAILED: " << text << std::endl;
    std::cout << "Expected=" << expected << " Actual=" << actual << std::endl;
    simulationDone();
    exit(1);
  }
  else {
    std::cout << "PASS: " << text << std::endl;
  }
}

void cycleClocks() {
  uut->apbPclk = 0;
  uut->eval();
  tfp->dump(sim_time += 25);

  uut->apbPclk = 1;
  uut->clk7mhz = uut->clk7mhz ? 0 : 1;
  uut->eval();
  tfp->dump(sim_time += 25);
}

#define MAX_COLORS 9

const uint8_t colors[MAX_COLORS] = {
    0xff,
    0x02,
    0x18,
    0xDE, // this shouldn't get displayed in 32bit mode
    0xCE,
    0xAD,
    0x98,
    0x01, // this shouldn't get displayed in 32bit mode
    0x00};

void populatePixelBuffer(NeoPixelDriver *driver) {
  // write color values into the buffer
  for (unsigned int i=0; i < MAX_COLORS; i++) {
    driver->writePixelByte(i, colors[i]);
  }

  // read it back and verify if they match
  for (unsigned int i = 0; i < MAX_COLORS; i++) {
    if (driver->readPixelByte(i) != colors[i]) {
      
      printf("\nPixel data @%d doesn't match actual %d != expected %d \n\n", i, 
        driver->readPixelByte(i), colors[i]);

      simulationDone();
    }
  }
}

void testHeader(std::string text) {
  std::cout << "---------------------------------------------" << std::endl;
  std::cout << text << std::endl;
}

void test1() {
  testHeader("Test 1 - write and read back registers");
  uut->anton_neopixel_apb_top__DOT__test_unit = 1;

  populatePixelBuffer(driver);

  driver->writeRegisterMax(0x1ace);
  assert_equals<uint16_t>("Large value in MAX control register", 0x1ace, driver->readRegisterMax());

  driver->writeRegisterMax(0xffff);
  assert_equals<uint16_t>("Overflowing value in MAX control register", 0x1fff, driver->readRegisterMax());

  driver->writeRegisterMax(7);
  assert_equals<uint16_t>("Small value in MAX control register", 7, driver->readRegisterMax());
}

void test2() {
  testHeader("Test 2 - run 32bit - soft limit mode with 7bytes max -> 8 bytes size (which is 2 pixels in 32bit mode)");

  driver->writeRegisterCtrl(CTRL_RUN | CTRL_32 | CTRL_LIMIT);
  uut->anton_neopixel_apb_top__DOT__test_unit = 2;
  while (driver->readRegisterState() == 0 && SIMULATION_NOT_STUCK) { // Wait to end stream and start reset
    cycleClocks();
  }

  if (driver->readRegisterState() != 1) {
    printf("ERROR: After stream phase the reset part should started. \n");
    printf("ERROR: Possibly the loop timeouted and never left from the stream phase.\n");
    simulationDone();
  }

  // Wait for the reset to finish (stream phase + reset phase = whole cycle)
  while (driver->testRegisterCtrl(CTRL_RUN) && SIMULATION_NOT_STUCK) { // Wait for the cycle to finish
    if (uut->neoData != 0)
    { // inside the reset part the output should be held low
      printf("ERROR: At the reset phase the neoData was not kept low\n");
      simulationDone();
    }
    cycleClocks();
  }

  cycleClocks();
  cycleClocks();
}

void test3() {
  testHeader("Test 3 - After one run is finished switch to 8bit with hard limit mode");

  uut->anton_neopixel_apb_top__DOT__test_unit = 3;
  driver->writeRegisterCtrl(CTRL_RUN);
  while (driver->testRegisterCtrl(CTRL_RUN) && SIMULATION_NOT_STUCK) {
    cycleClocks(); // Wait for the next cycle to finish
  }
}

void test4() {
  testHeader("Test 4 - Keep 8bit mode, but enable looping and software limit, and start it with a synch input");
  uut->anton_neopixel_apb_top__DOT__test_unit = 4;
  driver->writeRegisterCtrl(CTRL_LOOP | CTRL_LIMIT);
  driver->syncStart();

  // Iterate until simulation is finished or enough time passed.
  while (!Verilated::gotFinish() && SIMULATION_NOT_STUCK) {
    cycleClocks();
  }
}

int main(int argc, char** argv) {
  sim_time = 0;

  Verilated::commandArgs(argc, argv);
  uut = new Vanton_neopixel_apb_top;

  Verilated::traceEverOn(true);
  tfp = new VerilatedVcdC;
  // tfp->spTrace()->set_time_unit("1ns");
  // tfp->spTrace()->set_time_resolution("1ps");
  uut->trace(tfp, 99);

  std::string vcdname = argv[0];
  vcdname += ".vcd";
  std::cout << vcdname << std::endl;
  tfp->open(vcdname.c_str());
  uut->clk7mhz   = 0;
  uut->syncStart = 0;
  uut->anton_neopixel_apb_top__DOT__test_unit = 0;

  driver = new NeoPixelDriver(0, 60);

  test1();
  test2();
  test3();
  test4();

  // Proper end of the simulation, if the simulation was shutdown sooner, due
  // to test failure, then one indicators is that the coverage and/or
  // total simulated time dropped significantly.
  simulationDone();

  return 0;
}
