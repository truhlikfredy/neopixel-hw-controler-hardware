#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "neopixel_driver.h"
#include "neopixel_hal.h"
#include "neopixel_simulation.h"
#include "test_helper.h"

Vanton_neopixel_apb_top* uut;
VerilatedVcdC* tfp;
vluint64_t sim_time;
NeoPixelDriver* driver;


double sc_time_stamp() {
  return sim_time * 50;
}


void simulationDone() {
  // Done simulating
  uut->final();

  std::cout << "Simulation finished with " << sim_time << " timestamp."
            << std::endl;

#if VM_COVERAGE
  VerilatedCov::writeLcov("lcov.info");
#endif

  tfp->close();
  delete tfp;
  delete uut;
}


void simulationHardLimitReached() {
  std::cout << "ERROR: Simulation time reached hard limit, probably stuck at something. Terminating the simulation." << std::endl;
    
  simulationDone();
  exit(1);
}


void cycleClocks() {
  if (sim_time > SIMULATION_HARD_LIMIT) {
    simulationHardLimitReached();
  }

  uut->apbPclk = !uut->apbPclk;
  uut->eval();
  tfp->dump(sim_time += 25);

  uut->apbPclk = !uut->apbPclk;
  uut->eval();
  tfp->dump(sim_time += 25);

  uut->apbPclk = !uut->apbPclk;
  uut->eval();
  tfp->dump(sim_time += 25);

  uut->clk6_4mhz = uut->clk6_4mhz ? 0 : 1;
  uut->apbPclk = !uut->apbPclk;
  uut->eval();
  tfp->dump(sim_time += 25);

  uut->apbPclk = !uut->apbPclk;
  uut->eval();
  tfp->dump(sim_time += 25);

  uut->apbPclk = !uut->apbPclk;
  uut->eval();
  tfp->dump(sim_time += 25);
}


void testHeader(std::string text) {
  std::cout << "---------------------------------------------" << std::endl;
  std::cout << text << std::endl;
}


void test1() {
  testHeader("Test 1 - populate buffer with values");
  //TODO: make sure they are registers and save int the DOTs
  uut->anton_neopixel_apb_top__DOT__testUnit = 1;

  driver->selfTestPopulatePixelBuffer();
}


void test2() {
  testHeader("Test 2 - double buffering - skipped as its implementation got removed");
  uut->anton_neopixel_apb_top__DOT__testUnit = 2;
}


void test3() {
  testHeader("Test 3 - write and read back MAX register");
  uut->anton_neopixel_apb_top__DOT__testUnit = 3;

  driver->selfTestLowHighRegisters();
}


void test4() {
  testHeader(
      "Test 4 - run 32bit - soft limit mode with 7bytes max -> 8 bytes size "
      "(which is 2 pixels in 32bit mode)");
  uut->anton_neopixel_apb_top__DOT__testUnit = 4;

  driver->selfTestSoftLimit32bit();
}


void test5() {
  testHeader(
      "Test 5 - After one run is finished switch to 8bit with hard limit mode");
  uut->anton_neopixel_apb_top__DOT__testUnit = 5;

  driver->selfTestHardLimit8bit();
}

void test6() {
  testHeader(
      "Test 6 - Delta/Virtual writes");
  uut->anton_neopixel_apb_top__DOT__testUnit = 6;

  driver->selfTestVirtualWrites();
}


void test7() {
  testHeader(
      "Test 7 - Keep 8bit mode, but enable looping and software limit, and "
      "start it with a synch input");
  uut->anton_neopixel_apb_top__DOT__testUnit = 7;

  driver->selfTestSoftLimit8bitLoop();
}


void testDelay() {
  // just add delay so it's more visible on gtk wave
  for (uint32_t i = 0; i < 150; i++) {
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
  uut->clk6_4mhz = 0;
  uut->syncStart = 0;
  uut->anton_neopixel_apb_top__DOT__testUnit = 0;

  driver = new NeoPixelDriver(0x0, 16);

  testStart();
  test1();
  testDelay();

  test2();
  test3();
  testDelay();

  test4();
  testDelay();

  test5();
  testDelay();

  test6();
  testDelay();

  test7();
  testHeader("Tests finished without a failure");

  // Proper end of the simulation, if the simulation was shutdown sooner, due
  // to test failure, then one indicators is that the coverage and/or
  // total simulated time dropped significantly.
  simulationDone();

  return 0;
}
