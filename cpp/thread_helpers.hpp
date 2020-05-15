#ifndef _HELPERS_THREAD_HPP_
#define _HELPERS_THREAD_HPP_
#include <thread>

namespace helpers {
namespace thread {

static inline auto set_cpu_affinity(std::thread &thread, int cpu_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(2, &cpuset);
  return pthread_setaffinity_np(t1.native_handle(), sizeof(cpu_set_t), &cpuset);
}

} // namespace thread
} // namespace helpers

#endif
