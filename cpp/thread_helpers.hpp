#ifndef _HELPERS_THREAD_HPP_
#define _HELPERS_THREAD_HPP_
#include <thread>

namespace helpers {
namespace thread {

static inline auto set_cpu_affinity(std::thread &t, int cpu_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_id, &cpuset);
  return pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
}

} // namespace thread
} // namespace helpers

#endif
