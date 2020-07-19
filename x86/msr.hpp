#ifndef _MSR_CUSTOM_HPP_
#define _MSR_CUSTOM_HPP_

#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <x86intrin.h>

enum MSR {
  PMU_MSR_IA32_PEBS_ENABLE = 0x3F1,
  PMU_MSR_IA32_PERF_GLOBAL_CTRL = 0x38F,
  PMU_MSR_IA32_FIXED_CTR_CTRL = 0x38D,
  PMU_MSR_IA32_PERF_GLOBAL_INUSE = 0x392,

  PMU_MSR_IA32_FIXED_CTR0 = 0x309,
  PMU_MSR_IA32_FIXED_CTR1 = 0x30A,
  PMU_MSR_IA32_FIXED_CTR2 = 0x30B,

  PMU_MSR_PerfEvtSel0 = 0x186,
  PMU_MSR_PerfEvtSel1 = 0x187,
  PMU_MSR_PerfEvtSel2 = 0x188,
  PMU_MSR_PerfEvtSel3 = 0x189,
  PMU_MSR_PerfEvtSel4 = 0x18A,
  PMU_MSR_PerfEvtSel5 = 0x18B,
  PMU_MSR_PerfEvtSel6 = 0x18C,
  PMU_MSR_PerfEvtSel7 = 0x18D,

  PMU_MSR_IA32_PMC0 = 0xC1,
  PMU_MSR_IA32_PMC1 = 0xC2,
  PMU_MSR_IA32_PMC2 = 0xC3,
  PMU_MSR_IA32_PMC3 = 0xC4,
  PMU_MSR_IA32_PMC4 = 0xC5,
  PMU_MSR_IA32_PMC5 = 0xC6,
  PMU_MSR_IA32_PMC6 = 0xC7,
  PMU_MSR_IA32_PMC7 = 0xC8,

  PMU_MSR_OFFCORE_RSP_0 = 0x1A6,
  PMU_MSR_OFFCORE_RSP_1 = 0x1A7,

};

enum FixedCounterMask : uint32_t {
  FIXED_COUNT_USER = 0x1,
  FIXED_COUNT_KERN = 0x2,
  FIXED_COUNT_ANY = 0x4,
  FIXED_ENABLE_PMI = 0x8,
};

#include <map>

class PMUCtrl {
  static inline std::map<int, int> fd_map = {};

public:
  static inline int program_generic(int cpu, int counter_index,
                                    uint64_t event) {
    if (fd_map.find(cpu) == fd_map.end()) {
      int cpu_msr_fd = open(
          (std::string{"/dev/cpu/"} + std::to_string(cpu) + std::string{"/msr"})
              .c_str(),
          O_RDWR | O_CLOEXEC);
      if (cpu_msr_fd < 0) {
        return -1;
      }
      fd_map[cpu] = cpu_msr_fd;
    }

    if (pwrite(fd_map[cpu], &event, sizeof(uint64_t),
               MSR::PMU_MSR_PerfEvtSel0 + counter_index)) {
      return counter_index;
    } else {
      return -1;
    }
  }

  static inline int program_fixed(int cpu, int counter_index, uint64_t mask) {
    if (fd_map.find(cpu) == fd_map.end()) {
      int cpu_msr_fd = open(
          (std::string{"/dev/cpu/"} + std::to_string(cpu) + std::string{"/msr"})
              .c_str(),
          O_RDWR | O_CLOEXEC);
      if (cpu_msr_fd < 0) {
        return -1;
      }
      fd_map[cpu] = cpu_msr_fd;
    }

    uint64_t fixed_counter = 0;
    if (!pread(fd_map[cpu], &fixed_counter, sizeof(uint64_t),
               MSR::PMU_MSR_IA32_FIXED_CTR_CTRL)) {
      return -1;
    }

    fixed_counter |= (mask << (counter_index * 4));
    if (pwrite(fd_map[cpu], &fixed_counter, sizeof(uint64_t),
               MSR::PMU_MSR_IA32_FIXED_CTR_CTRL)) {
      return counter_index;
    } else {
      return -1;
    }
  }

  static inline int enable(int cpu, int counter_index) {
    if (fd_map.find(cpu) == fd_map.end()) {
      std::cerr << "Program Counter Before Enable" << std::endl;
    }

    uint64_t global_counter = 0;
    if (!pread(fd_map[cpu], &global_counter, sizeof(uint64_t),
               MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    global_counter |= (0x1UL << counter_index);

    if (!pwrite(fd_map[cpu], &global_counter, sizeof(uint64_t),
                MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    return global_counter;
  }

  static inline int enable_fixed(int cpu, int counter_index) {
    if (fd_map.find(cpu) == fd_map.end()) {
      std::cerr << "Program Counter Before Enable" << std::endl;
      return -1;
    }

    uint64_t global_counter = 0;

    if (!pread(fd_map[cpu], &global_counter, sizeof(uint64_t),
               MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    const auto shift = 32 + counter_index;
    global_counter |= (0x1UL << shift);

    if (!pwrite(fd_map[cpu], &global_counter, sizeof(uint64_t),
                MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }

    return global_counter;
  }

  static inline int disable(int cpu, int counter_index) {
    if (fd_map.find(cpu) == fd_map.end()) {
      std::cerr << "Program Counter Before Enable" << std::endl;
    }

    uint64_t global_counter = 0;
    if (!pread(fd_map[cpu], &global_counter, sizeof(uint64_t),
               MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    global_counter &= ~(0x1UL << counter_index);
    if (!pwrite(fd_map[cpu], &global_counter, sizeof(uint64_t),
                MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    return global_counter;
  }

  static inline int disable_fixed(int cpu, int counter_index) {
    if (fd_map.find(cpu) == fd_map.end()) {
      std::cerr << "Program Counter Before Enable" << std::endl;
      return -1;
    }

    uint64_t global_counter = 0;

    if (!pread(fd_map[cpu], &global_counter, sizeof(uint64_t),
               MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    global_counter &= ~(0x1UL << (counter_index + 32));

    if (!pwrite(fd_map[cpu], &global_counter, sizeof(uint64_t),
                MSR::PMU_MSR_IA32_PERF_GLOBAL_CTRL)) {
      return -1;
    }
    return global_counter;
  }

  static inline void close_pmu() {
    for (const auto &[key, val] : fd_map) {
      close(val);
    }
  }
};

#endif