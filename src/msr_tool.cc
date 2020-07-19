#include "x86/msr.hpp"
#include <iostream>

int main() {
  int counter = -1;
  int cpu = -1;
  std::cout << "Enter CPU: ";
  std::cin >> cpu;
  std::cout << "Enter Fixed Counter Index: ";
  std::cin >> counter;

  PMUCtrl::program_fixed(cpu, counter, FIXED_COUNT_USER | FIXED_COUNT_KERN);
  PMUCtrl::enable_fixed(cpu, counter);
  PMUCtrl::close_pmu();
}
