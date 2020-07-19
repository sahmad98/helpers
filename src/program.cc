#include <cstdlib>
#include <cstring>
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <perfmon/pfmlib.h>
#include <string>
#include <vector>

#include "msr.hpp"

static inline void list_pmu_events(pfm_pmu_t pmu, bool only_pmu = false) {
  pfm_event_info_t info;
  pfm_pmu_info_t pinfo;
  int i, ret;

  memset(&info, 0, sizeof(info));
  memset(&pinfo, 0, sizeof(pinfo));

  info.size = sizeof(info);
  pinfo.size = sizeof(pinfo);

  ret = pfm_get_pmu_info(pmu, &pinfo);
  if (ret != PFM_SUCCESS)
    printf("cannot get pmu info");

  if (only_pmu)
    return;

  for (i = pinfo.first_event; i != -1; i = pfm_get_event_next(i)) {
    ret = pfm_get_event_info(i, PFM_OS_NONE, &info);
    if (ret != PFM_SUCCESS) {
      printf("cannot get event info");
      continue;
    }

    std::cout << "Generic Counters " << pinfo.num_cntrs << ", Fixed "
              << pinfo.num_fixed_cntrs << std::endl;
    printf("%s Event: %s 0x%lx %s\n", pinfo.is_present ? "Active" : "Supported",
           pinfo.name, info.code, info.name);

    pfm_event_attr_info_t attr;
    for (int j = 0; j < info.nattrs; ++j) {
      memset(&attr, 0, sizeof(pfm_event_attr_info_t));
      attr.size = sizeof(pfm_event_attr_info_t);
      ret = pfm_get_event_attr_info(info.idx, j, PFM_OS_NONE, &attr);
      if (ret != PFM_SUCCESS) {
        printf("Error in Attributes, Total Attrs: %d,  %s\n", info.nattrs,
               pfm_strerror(ret));
        break;
      }

      if (attr.type != PFM_ATTR_UMASK)
        continue;
      printf("%s Event: %s 0x%lx %s.%s 0x%lx\n",
             pinfo.is_present ? "Active" : "Supported", pinfo.name, info.code,
             info.name, attr.name, attr.code);
    }
  }
}

struct EventCounter {
  uint64_t event;
  uint64_t index;
  uint64_t start;
  uint64_t end;
  uint64_t delta;
  uint64_t delta_tsc;
  std::string name;
};

int main(int argc, char **argv) {
  cxxopts::Options options("program", "Utility to Program CPU PMU MSRs");
  bool default_list_event = false;
  bool default_program = false;
  options.add_options()("e,event", "List PFMLib Event List",
                        cxxopts::value(default_list_event))(
      "p,program", "Program PMU Counters",
      cxxopts::value(default_program))("h,help", "Print Usage");

  auto result = options.parse(argc, argv);
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  bool list_events = result["event"].as<bool>();
  int ret = pfm_initialize();
  if (ret != PFM_SUCCESS) {
    std::cerr << "Unable to Intialize PFM Lib " << pfm_strerror(ret)
              << std::endl;
  }

  list_pmu_events(PFM_PMU_INTEL_IVB, !list_events);
  if (list_events)
    exit(0);

  bool program = result["program"].as<bool>();
  if (!program)
    exit(0);

  std::ifstream ctrs_file("counters.txt");
  std::vector<std::string> counters{};

  for (std::string line; std::getline(ctrs_file, line);) {
    counters.push_back(line);
  }

  for (const auto &ctr : counters) {
    std::cout << ctr << "\n";
  }

  pfm_pmu_encode_arg_t event;
  std::memset(&event, 0, sizeof(pfm_pmu_encode_arg_t));

  char *event_str = new char[1024];
  event.fstr = &event_str;
  std::vector<EventCounter> events{};

  int index = 0;
  for (const auto &ctr : counters) {
    ret = pfm_get_os_event_encoding(ctr.c_str(), PFM_PLM3 | PFM_PLM0,
                                    PFM_OS_NONE, &event);
    if (ret != PFM_SUCCESS) {
      std::cerr << "Unable to query event : " << ctr << " " << pfm_strerror(ret)
                << std::endl;
      exit(1);
    }

    for (int i = 0; i < event.count; ++i) {
      std::cout << event.fstr[i] << " "
                << "EVENT_NUMBER: " << std::hex << event.codes[i] << std::dec
                << std::endl;
      EventCounter evctr;
      evctr.event = event.codes[i];
      evctr.name = ctr;
      evctr.index = index++;
      events.push_back(evctr);
    }

    free(event.codes);
  }

  for (auto &ev : events) {
    if (PMUCtrl::program_generic(3, ev.index, ev.event) < 0) {
      std::cout << "Error in opening " << ev.name << std::endl;
      continue;
    }
    PMUCtrl::enable(3, ev.index);
    std::cout << "EV: " << ev.name << " ,index: " << ev.index << std::endl;
  }

  for (int i = 0; i < 3; i++) {
    if (PMUCtrl::program_fixed(3, i, FIXED_COUNT_USER | FIXED_COUNT_KERN) < 0) {
      std::cout << "Error in Fixed Counter Programming " << i << std::endl;
      continue;
    }
    PMUCtrl::enable_fixed(3, i);
    std::cout << "FixedCtr: " << i << std::endl;
  }

  int i;
  std::cin >> i;

  for (auto &ev : events) {
    PMUCtrl::disable(3, ev.index);
  }

  PMUCtrl::disable_fixed(3, 0);
  PMUCtrl::disable_fixed(3, 1);
  PMUCtrl::disable_fixed(3, 2);
  PMUCtrl::close_pmu();
  delete[] event_str;
}