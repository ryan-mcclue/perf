// SPDX-License-Identifier: zlib-acknowledgement

#pragma once

typedef enum
{
  REPETITION_TESTER_STATE_UNINITIALISED = 0,
  REPETITION_TESTER_STATE_TESTING,
  REPETITION_TESTER_STATE_COMPLETED,
  REPETITION_TESTER_STATE_ERROR,
  REPETITION_TESTER_STATE_COUNT
} REPETITION_TESTER_STATE;

typedef struct RepetitionTester RepetitionTester;
struct RepetitionTester
{
  REPETITION_TESTER_STATE state;
  u32 test_time;
};

INTERNAL b32 
are_testing(RepetitionTester *tester)
{
  if (tester->state == REPEAT_STATE_TESTING)
  {
    u64 current_time = read_cpu_timer();

    if (tester->bytes_accumulated_on_this_test != tester->target_bytes_processed) test_error();
    
    if (tester->state == REPEAT_STATE_TESTING)
    {
      u32 elapsed_time = tester->time_accumulated_on_this_test;
      tester->test_count += 1;
      tester->total_time += tester->time_accumulated_on_this_test;
      if (tester->max_time < elapsed_time)
      {
        tester->max_time = elapsed_time;
      }
      if (tester->min_time > elapsed_time)
      {
        tester->min_time = elapsed_time;

        // so, test_time is from most recent minimum
        tester->tests_started_at = current_time;
      }
      tester->time_accumulated_on_this_test = 0;
      tester->bytes_accumulated_on_this_test = 0;
    }

    if ((current_time - tester->test_started_at) > tester->test_time)
    {
      tester->state = COMPLETED;
      print_results();
    }

  }
}

BEGIN_TIME()
{
  state->open_block_count++;
  state->repeat_time -= read_cpu_timer();
}
END_TIME()
{
  state->close_block_count++;
  state->repeat_time += read_cpu_timer();
}

INTERNAL void
repeat_read_via_fread(RepeatState *state, ReadParams *params)
{
  while (true)
  {
     FILE *file = fopen(params->name, "rb"); 
     if (file != NULL)
     {
       BEGIN_TIME(tester);
       u32 res = fread(params->buffer);
       END_TIME(tester);

       if (res == 1) repetition_count_bytes(tester, params->buffer.count);
       // TODO(Ryan): How are incomplete file reads handled?
     }
     else
     {
       repetition_error(tester, "fopen() failed");
       break;
     }

     // update
  }
}

INTERNAL void
repeat(RepeatState *state, u64 target_processed_byte_count, u64 cpu_timer_freq, u32 seconds_to_try)
{

}

static void NewTestWave(repetition_tester *Tester, u64 TargetProcessedByteCount, u64 CPUTimerFreq, u32 SecondsToTry = 10)
{
    if(Tester->Mode == TestMode_Uninitialized)
    {
        Tester->Mode = TestMode_Testing;
        Tester->TargetProcessedByteCount = TargetProcessedByteCount;
        Tester->CPUTimerFreq = CPUTimerFreq;
        Tester->PrintNewMinimums = true;
        Tester->Results.MinTime = (u64)-1;
    }
    else if(Tester->Mode == TestMode_Completed)
    {
        Tester->Mode = TestMode_Testing;
        
        if(Tester->TargetProcessedByteCount != TargetProcessedByteCount)
        {
            Error(Tester, "TargetProcessedByteCount changed");
        }
        
        if(Tester->CPUTimerFreq != CPUTimerFreq)
        {
            Error(Tester, "CPU frequency changed");
        }
    }

    Tester->TryForTime = SecondsToTry*CPUTimerFreq;
    Tester->TestsStartedAt = ReadCPUTimer();
}

TestFunc func = {"fread", read_via_fread};

INTERNAL void
repeat_reads(void)
{
  RepetitionTester testers[ARRAY_COUNT(test_functions)]; 

  ReadParams params = ZERO_STRUCT;
  params.name = "file.bin";
  params.buffer = malloc();

  while (true) 
  {
    for (u32 i = 0; i < ARRAY_COUNT(test_functions); i += 1)
    {
      RepetitionTester *tester = testers + i;
      TestFunc *func = tester->func;
      printf("\n--- %s ---\n", func.name);

      init_repeat_state(tester);
      //NewTestWave(Tester, Params.Dest.Count, CPUTimerFreq);

      func.func(tester, &params);
    }
  }
}
