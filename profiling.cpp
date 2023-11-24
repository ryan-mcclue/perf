// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): USE EXCEL FOR GRAPH CREATION OVER GNUPLOT!

// a course measurement like a total runtime is only useful as a final measurement
// want more granular, to identify slow parts 

// rdtsc monotonic increase from 0 at cpu boot
// it's an invariant tsc broadcast to all cores regardless of individual core frequency differences
// so, not really counting core cycles. just like a really precise wall clock

// tsc is on CPU, hpet is external on motherboard (so, would read from I/O port using 'in')
// integer divides can be done with multiply and shift, as low bits implicit divide by 2^64

// as clock_gettime is vDSO syscall, cannot step in naively.
// looking at dissassembly:
//   uses 'pause' instruction (most likely a spin lock for synchronisation)
//   kernel uses hpet over tsc as clock source if tsc considered unstable (dmesg | rg -i clocksource)
//   see granularity of rdtsc reduced, so just use rdtsc directly

// rdtsc can be used for computing program percentage time, 
// e.g. 100.0 * (section_cycle_start / cycle_total);

// x86intrin.h doubles compile times
//asm volatile("rdtsc" : "=a" (c), "=d" (d));
//uint64_t ticks = (( (u64)c ) | (( (u64)d ) << 32)); 

// simd: https://lemire.me/blog/2023/09/22/parsing-integers-quickly-with-avx-512/ 
// https://travisdowns.github.io/

// instrumentation profiling is adding timing code

// want way to modularly bring in module to see how it affects program runtime
// anything called say 1mil. times will always give a performance penalty
// so, ascertain overall profile, then remove until not affecting performance as much, e.g. nested block removals (essentially perform A/B testing)
// to time function with no nested overheads, just TIME_BLOCK("S") { func() };

// enum ARR_KEY
// {
//   ARR_KEY_TIMER,
//   ARR_KEY_BYTES,
//   ARR_KEY_COUNT
// };
// u32 arr[ARR_KEY_COUNT];

INTERNAL u64 linux_page_fault_count(void)
{
  struct rusage usage = ZERO_STRUCT;
  getrusage(RUSAGE_SELF, &usage);

  // ru_minflt  the number of page faults serviced without any I/O activity.
  // ru_majflt  the number of page faults serviced that required I/O activity.
  u64 result = usage.ru_minflt + usage.ru_majflt;

  return result;
}
// have a separate thread on startup that pre-touches memory (in a sense, prefetching), before saturating it with work


#if defined(PROFILER)
  #define PROFILER_END_OF_COMPILATION_UNIT \
    STATIC_ASSERT(__COUNTER__ <= ARRAY_COUNT(global_profiler.slots))
  #define PROFILE_BLOCK(name) \
    for (struct {ProfileEphemeral e; u32 i;} UNIQUE_NAME(l) = {profile_block_start(name, __COUNTER__ + 1, 0), 0}; \
         UNIQUE_NAME(l).i == 0; \
         profile_block_end(&(UNIQUE_NAME(l)).e), UNIQUE_NAME(l).i++)
  #define PROFILE_FUNCTION() \
    PROFILE_BLOCK(__func__)
  #define PROFILE_BANDWIDTH(name, byte_count) \
    for (struct {ProfileEphemeral e; u32 i;} UNIQUE_NAME(l) = {profile_block_start(name, __COUNTER__ + 1, byte_count), 0}; \
         UNIQUE_NAME(l).i == 0; \
         profile_block_end(&(UNIQUE_NAME(l)).e), UNIQUE_NAME(l).i++)
  // PROFILE_BANDWIDTH("str8_read_entire_file", result.count);

  // TODO(Ryan): This block profiler not appropriate for measuring small amounts of code?
  typedef struct ProfileSlot ProfileSlot;
  struct ProfileSlot
  {
    u64 elapsed_exclusive; // no children
    u64 elapsed_inclusive; // children
    u64 hit_count;
    u64 byte_count;
    char *label;
  };
  
  typedef struct Profiler Profiler;
  struct Profiler
  {
    ProfileSlot slots[4096];
    u64 start;
    u64 end;
  };
  
  typedef struct ProfileEphemeral ProfileEphemeral;
  struct ProfileEphemeral
  {
    char *label;
    u64 old_elapsed_inclusive;
    u64 start;
    u32 parent_slot_index;
    u32 slot_index; 
  };
  
  GLOBAL Profiler global_profiler;
  GLOBAL u32 global_profiler_parent_slot_index;
  
  INTERNAL void
  profiler_init(void)
  {
    global_profiler.start = __rdtsc();
  }
  
  INTERNAL ProfileEphemeral
  profile_block_start(char *label, u32 slot_index, u64 byte_count)
  {
    ProfileEphemeral ephemeral = ZERO_STRUCT;
    ephemeral.slot_index = slot_index;
    ephemeral.label = label;
    ephemeral.parent_slot_index = global_profiler_parent_slot_index;
  
    ProfileSlot *slot = global_profiler.slots + slot_index;
    ephemeral.old_elapsed_inclusive = slot->elapsed_inclusive;

    slot->byte_count += byte_count;
  
    ephemeral.start = __rdtsc();
  
    return ephemeral;
  }
  
  INTERNAL u32
  profile_block_end(ProfileEphemeral *ephemeral)
  {
    u64 elapsed = __rdtsc() - ephemeral->start; 
  
    global_profiler_parent_slot_index = ephemeral->parent_slot_index;
  
    ProfileSlot *parent_slot = global_profiler.slots + ephemeral->parent_slot_index;
    parent_slot->elapsed_exclusive -= elapsed;
  
    ProfileSlot *slot = global_profiler.slots + ephemeral->slot_index;
    slot->elapsed_exclusive += elapsed;
    slot->elapsed_inclusive = ephemeral->old_elapsed_inclusive + elapsed; // handle recursion; just overwrite what children wrote
    slot->hit_count++;
    slot->label = ephemeral->label;
  
    return 0;
  }
  
  INTERNAL void
  profiler_end_and_print(void)
  {
    u64 cpu_freq = linux_estimate_cpu_timer_freq();
    u64 total = __rdtsc() - global_profiler.start;
    if (cpu_freq)
    {
      printf("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)total/(f64)cpu_freq, cpu_freq);
    }
  
    for (u32 i = 1; i < ARRAY_COUNT(global_profiler.slots); i += 1)
    {
      ProfileSlot *slot = global_profiler.slots + i;
      if (slot->hit_count == 0) break;
  
      f64 percent = 100.0 * ((f64)slot->elapsed_exclusive / (f64)total);
      printf("  %s(%lu): %lu (%0.2f%%", slot->label, slot->hit_count, slot->elapsed_exclusive, percent);
  
      if (slot->elapsed_inclusive != slot->elapsed_exclusive)
      {
        f64 percent_with_children = 100.0 * ((f64)slot->elapsed_inclusive / (f64)total);
        printf(", %.2f%% w/children", percent_with_children); 
      }
      printf(")");

      if (slot->byte_count != 0)
      {
        f64 seconds = (f64)slot->elapsed_inclusive/(f64)cpu_freq;
        f64 bps = (f64)slot->byte_count / seconds; 
        // TODO(Ryan): gb/s often what striving for? 0.3-0.5gb/s for modern CPUs?
        printf(" %.3fmb at %.2fgb/s", (f64)slot->byte_count / (f64)MB(1), bps / (f64)GB(1));
      }
      printf("\n");
    }
  }
#else
  #define PROFILER_END_OF_COMPILATION_UNIT
  #define PROFILE_FUNCTION()
  #define PROFILE_BLOCK(name)
  #define PROFILE_BANDWIDTH(name, byte_count)

  typedef struct Profiler Profiler;
  struct Profiler
  {
    u64 start;
    u64 end;
  };
  
  GLOBAL Profiler global_profiler;
  
  INTERNAL void
  profiler_init(void)
  {
    global_profiler.start = __rdtsc();
  }

  INTERNAL void
  profiler_end_and_print(void)
  {
    u64 cpu_freq = linux_estimate_cpu_timer_freq();
    u64 total = __rdtsc() - global_profiler.start;
    if (cpu_freq)
    {
      printf("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)total/(f64)cpu_freq, cpu_freq);
    }
  }
#endif

struct AddressArray
{
  void **addresses;
  u64 count;
  u64 page_size;
};

struct TrackedBuffer
{
  String8 base;
  AddressArray results;
};

TrackedBuffer allocate(u64 size)
{
  TrackedBuffer buf;
  buf.base.content = malloc(size); // want to read dirty bits of this stuff
  buf.base.size = size;

  buf.results.count = size / page_size;
  buf.results.addresses = malloc(page_count * sizeof(void**));
}

// so, track pages written to across calls
AddressArray get_and_clear_written_pages(buf)
{
  // https://lwn.net/Articles/940704/
  // https://docs.kernel.org/admin-guide/mm/soft-dirty.html
  // read and then reset dirty bits
  // get say page 10, 11, 20 written to
}

// paging get circular buffer, sparseness and memory recording for free 
struct CircularBuffer
{
  String8 buf;
  u32 rep_count;
};

INTERNAL CircularBuffer
create(void)
{
  u64 data_size = ALIGN_POW2_UP(size, sysconf(_SC_PAGESIZE));
  u64 total_size = rep_count * data_size;

  // don't commit, just want this much virtual address space
  u8 *addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (addr == MAP_FAILED);

  // shrink to actual size to get usable handles
  u8 *first = mremap(addr, total_size, data_size, 0);
  for ()

  u8 *second = mremap(addr, 0, data_size, MREMAP_MAYMOVE, addr+data_size);
  u8 *third = mremap(addr, 0, data_size, MREMAP_MAYMOVE, addr+data_size*2);

  // return second; so second[0] = second[-size] = second[size]
  // so can now just do memcpys without splitting

  return str8(mapping, data_size);

  // free
  munmap(addr, size);
}

test_func test_functions[] = {
  {"fread", read_via_fread},
  {"read", read_via_read}
};

RepetitionTester testers[ARRAY_COUNT(test_functions)] = {};

for (u32 i = 0; i < ARRAY_COUNT(test_functions); i += 1)
{
  new_test_wave(testers[i]);
  test_functions[i].func(testers[i]);
}

while (is_testing())
{
  if (tester->open_block_count > 0) // if any begin_time() recorded
  // test here
}

