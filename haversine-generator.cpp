// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

typedef struct Arguments
{
  u32 num_pairs;
  String8 output_file;
} Arguments;

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
      result.num_pairs = str8_to_u32(arg->next->string, 10);
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

  return 0;
}
