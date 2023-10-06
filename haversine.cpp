// SPDX-License-Identifier: zlib-acknowledgement

/* https://www.youtube.com/watch?v=ddfSDIzIbdE&t=591s 
 * Put arena here as want all memory here to be contiguous
 * struct something {
 *   MemArena *a;
 * }
 */

#include "base-inc.h"

#include "parser.cpp"

typedef struct Arguments Arguments;
struct Arguments
{
  u32 num_pairs;
  String8 output_file;
};
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


#if 0
void parse(void)
{
  if (recognise_token(TOKEN_BRACE_OPEN)) advance_token(); parse_array();

  for (; consume_token(TOKEN_COMMA))
  {
    if (recognise_token(TOKEN_BRACE_CLOSE)) break;
    else if (recognise_token(TOKEN_INTEGER)) advance_token();
  }
  require_token(TOKEN_SEMICOLON);
}

INTERNAL void
parse(FileData *data)
{
  String8 text = data->text; 
  TokenArray token_array = data->token_array;

  for (u32 i = 0; i < token_array.count; i += 1)
  {
    Token *token = &token_array.tokens[i];
    String8 token_str = str8_substr(text, token->start, token->end);
  }
}

INTERNAL void
bool expect_token(TokenKind kind) {
  if (is_token(kind)) {
    next_token();
    return true;
  } else {
    fatal_error_here("Expected token %s, got %s", token_kind_name(kind), token_info());
    return false;
  }
}

INTERNAL TokenArray
token_list_to_array(MemArena *arena, TokenList *list)
{
  TokenArray array = ZERO_STRUCT;
  array.count = list->count;
  array.tokens = MEM_ARENA_PUSH_ARRAY(arena, Token, array.count);

  u64 i = 0;
  for (TokenNode *n = list->first; n != NULL; n = n->next)
  {
    MEMORY_COPY(array.tokens + i, n->token, sizeof(Token));
    i += 1;
  }

  return array;
}
#endif




// require() functions for parser?

// IMPORTANT(Ryan): For char* arrays, just end with null terminator
// char *keyword_args[] = {};
// token->type = Identifier
// if (str8_match_array(str, keyword_args)) token->type = Keyword
// https://www.youtube.com/watch?v=pKZ_p3lmHfk


// IMPORTANT(Ryan): Index checking! str8_in_bounds(str, i);
// ProfScope("%.*s", Str8VArg(path)) {}
// ProfScope("load & lex all C files") {}

// c_file->tokens = CL_TokenArrayFromString(mg_arena, c_file->data);

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
