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
haversine(f64 x0, f64 y0, f64 x1, f64 y1)
{
  f64 result = 0.0;

  f64 lat_diff = F64_DEG_TO_RAD(x1 - x0);
  f64 long_diff = F64_DEG_TO_RAD(y1 - y0);
  f64 lat0 = F64_DEG_TO_RAD(x0);
  f64 lat1 = F64_DEG_TO_RAD(x1);

  f64 inner = SQUARE(F64_SIN(lat_diff / 2)) + \
          F64_COS(lat0) * F64_COS(lat1) * SQUARE(F64_SIN(long_diff / 2));

  result = 2.0 * F64_EARTH_RADIUS * F64_ASIN(F64_SQRT(inner));

  return result;
}

INTERNAL f64
rand_degree(u64 *seed, f64 centre, f64 radius, f64 max_allowed)
{
  f64 min = centre - radius; 
  if (min < -max_allowed) min = -max_allowed;

  f64 max = centre + radius;
  if (max > max_allowed) max = max_allowed;

  return f64_rand_range(seed, min, max);
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
  str8_list_push(perm_arena, &output, str8_lit("{ \"pairs\": [\n"));

  u64 seed = linux_get_seed();
  f64 max_lat = 90.0;
  f64 max_long = 180.0;
  f64 cluster_lat = 0.0, cluster_long = 0.0, cluster_r = 0.0;
  u64 cluster_roll = 0;
  u64 cluster_size = 1 + (args.num_pairs / 64);

  MemArenaTemp temp = mem_arena_temp_begin(NULL, 0);

  f64 avg_coeff = 1.0 / args.num_pairs;
  f64 avg = 0.0;
  for (u64 i = 0; i < args.num_pairs; i += 1)
  {
    if (cluster_roll == 0) 
    {
      cluster_lat = f64_rand_range(&seed, -max_lat, max_lat);
      cluster_long = f64_rand_range(&seed, -max_long, max_long);
      cluster_r = f64_rand_range(&seed, 0, max_lat);
    }

    f64 rand_lat0 = rand_degree(&seed, cluster_lat, cluster_r, max_lat);
    f64 rand_lat1 = rand_degree(&seed, cluster_lat, cluster_r, max_lat);
    f64 rand_long0 = rand_degree(&seed, cluster_long, cluster_r, max_long);
    f64 rand_long1 = rand_degree(&seed, cluster_long, cluster_r, max_long);

    f64 d = haversine(rand_lat0, rand_long0, rand_lat1, rand_long1);
    avg += avg_coeff * d;

    String8 entry = str8_fmt(temp.arena, "{ \"x0\": %.16f, \"y0\": %.16f, \"x1\": %.16f, \"y1\": %.16f}%c\n", 
                             rand_lat0, rand_long0, rand_lat1, rand_long1, (i == args.num_pairs - 1) ? ' ' : ',');
    str8_list_push(perm_arena, &output, entry);

    cluster_roll += 1;
    if (cluster_roll == cluster_size) cluster_roll = 0;
  }

  mem_arena_temp_end(temp);

  str8_list_push(perm_arena, &output, str8_lit("]}"));
  String8 output_merge = str8_list_join(perm_arena, output, NULL);
  str8_write_entire_file(args.output_file, output_merge);

  printf("pairs: %d, avg: %.16f\n", args.num_pairs, avg);

  return 0;
}
