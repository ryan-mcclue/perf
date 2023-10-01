// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

typedef struct Arguments
{
  u32 num_pairs;
  String8 output_file;
} Arguments;
#define ARGS_MAX_PAIRS MILLION(10)

INTERNAL Arguments
parse_arguments(String8List args)
{
  Arguments result = {
    10000,
    str8_lit("haversine-pairs.json")
  };

  String8Node *arg = args.first; 
  while (arg != NULL)
  {
    String8 arg_string = arg->string; 
    if (str8_match(arg_string, str8_lit("-pairs"), 0))
    {
      if (arg->next == NULL)
      {
        printf("Error: missing argument after '-pairs'\n");
        break;
      }
      s64 num_pairs_pre = str8_to_int(arg->next->string);
      u32 num_pairs = (u32)CLAMP(0, num_pairs_pre, ARGS_MAX_PAIRS);
      result.num_pairs = num_pairs; 
      arg = arg->next->next;
    }
    else
    {
      printf("Warn: unknown argument '%.*s'\n", str8_varg(arg_string));
      arg = arg->next;
    }
  }

  return result;
}

#define F64_EARTH_RADIUS 6372.8
INTERNAL f64
haversine(Vec2F64 p0, Vec2F64 p1)
{
  f64 result = 0.0;

  f64 lat_diff = F64_DEG_TO_RAD(p1.x - p0.x);
  f64 long_diff = F64_DEG_TO_RAD(p1.y - p0.y);
  f64 lat0 = F64_DEG_TO_RAD(p0.x);
  f64 lat1 = F64_DEG_TO_RAD(p1.x);

  f64 inner = SQUARE(F64_SIN(lat_diff / 2)) + \
          F64_COS(lat0) * F64_COS(lat1) * SQUARE(F64_SIN(long_diff / 2));

  result = 2.0 * F64_EARTH_RADIUS * F64_ASIN(F64_SQRT(inner));

  return result;
}

int
main(int argc, char *argv[])
{
  global_debugger_present = linux_was_launched_by_gdb();
  MemArena *perm_arena = mem_arena_allocate_default();

  ThreadContext tctx = thread_context_allocate();
  tctx.is_main_thread = 1;
  thread_context_set(&tctx);

  linux_set_cwd_to_self();

  String8List args_list = ZERO_STRUCT;
  for (s32 i = 1; i < argc; i += 1)
  {
    str8_list_push(perm_arena, &args_list, str8_cstr(argv[i]));
  }

  Arguments args = parse_arguments(args_list);

  String8List output = ZERO_STRUCT;
  str8_list_push(perm_arena, &output, str8_lit("\"{ \"pairs\": [\n"));

  u64 seed = linux_get_seed_u64();
  u64 cluster_roll = 1 + (args.num_pairs / 64);
  for (u64 i = 0; i < args.num_pairs; i += 1)
  {
     
  }

  str8_list_push(perm_arena, &output, str8_lit("]}\""));
  String8 output_merge = str8_list_join(perm_arena, output, NULL);
  str8_write_entire_file(args.output_file, output_merge);

  return 0;
}

static f64 RandomDegree(random_series *Series, f64 Center, f64 Radius, f64 MaxAllowed)
{
    f64 MinVal = Center - Radius;
    if(MinVal < -MaxAllowed)
    {
        MinVal = -MaxAllowed;
    }
    
    f64 MaxVal = Center + Radius;
    if(MaxVal > MaxAllowed)
    {
        MaxVal = MaxAllowed;
    }
    
    f64 Result = RandomInRange(Series, MinVal, MaxVal);
    return Result;
}
