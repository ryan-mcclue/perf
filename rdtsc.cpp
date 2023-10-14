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
// to time function with no nested overheads, just TIME_BLOCK("S") { func() };

#if defined(PROFILER)
  #define PROFILER_END_OF_COMPILATION_UNIT \
    STATIC_ASSERT(__COUNTER__ <= ARRAY_COUNT(global_profiler.slots))
  #define PROFILE_BLOCK(name) \
    for (struct {ProfileEphemeral e; u32 i;} UNIQUE_NAME(l) = {profile_block_start(name, __COUNTER__ + 1), 0}; \
         UNIQUE_NAME(l).i == 0; \
         profile_block_end(&(UNIQUE_NAME(l)).e), UNIQUE_NAME(l).i++)
  #define PROFILE_FUNCTION() \
    PROFILE_BLOCK(__func__)

  typedef struct ProfileSlot ProfileSlot;
  struct ProfileSlot
  {
    u64 elapsed_exclusive; // no children
    u64 elapsed_inclusive; // children
    u64 hit_count;
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
  profile_block_start(char *label, u32 slot_index)
  {
    ProfileEphemeral ephemeral = ZERO_STRUCT;
    ephemeral.slot_index = slot_index;
    ephemeral.label = label;
    ephemeral.parent_slot_index = global_profiler_parent_slot_index;
  
    ProfileSlot *slot = global_profiler.slots + slot_index;
    ephemeral.old_elapsed_inclusive = slot->elapsed_inclusive;
  
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
      printf(")\n");
    }
  }
#else
  #define PROFILER_END_OF_COMPILATION_UNIT
  #define PROFILE_FUNCTION()
  #define PROFILE_BLOCK(name)

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

