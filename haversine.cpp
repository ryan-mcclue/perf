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

typedef enum
{
  TOKEN_TYPE_NULL = 0, 
  TOKEN_TYPE_KEYWORD, // true/false/null
  TOKEN_TYPE_SEMICOLON,
} TOKEN_TYPE;

typedef struct
{
  TOKEN_TYPE type;
  Rng1U64 range;
} Token;

typedef struct
{
  TokenNode *next;
  Token token;
} TokenNode;

typedef struct
{
  TokenNode *first, *last;
  u64 count;
} TokenList;

typedef struct
{
  Token *tokens;
  u64 count;
} TokenArray;

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






INTERNAL TokenArray
get_token_array(MemArena *arena, String8 str)
{
  MemArenaTemp temp = mem_arena_temp_begin(arena, 1);

  TokenList tokens = ZERO_STRUCT; // ??

  TOKEN_TYPE active_token_type = TOKEN_TYPE_NULL;
  u64 active_token_start_offset = 0;

  u64 offset = 0;
  u64 advance = 0;
  char ch = str.content[offset];
  while (offset <= str.size)
  {
    advance = (active_token_type != NULL) ? 1 : 0;

    switch (active_token_type)
    {
      case TOKEN_TYPE_NULL:
      {
        if (ch == '\n')
        {
          active_token_type = TOKEN_TYPE_NEWLINE;
          active_token_start_offset = offset;
          advance = 0; // why zero?
        }
        else if (ch == ' ' || ch == '\t')
        {

        }
      } break;
      default:
      {

      }
    }

    if (ender_found != 0)
    {
      Token token = {active_token_type, rng1_u32(active_token_start_off, offset + advance)};
      CL_TokenChunkListPush(arena, &tokens, 1024, &token);
      // token_list_push()
      active_token_kind = CL_TokenKind_Null;
      active_token_start_off = token.range.max;
    }

    offset += advance;
  }

  TokenArray result = token_array_from_list();

  mem_arena_temp_end(temp);
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


// require() functions for parser?

// IMPORTANT(Ryan): For char* arrays, just end with null terminator
// char *keyword_args[] = {};
// token->type = Identifier
// if (str8_match_array(str, keyword_args)) token->type = Keyword
// https://www.youtube.com/watch?v=pKZ_p3lmHfk

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


typedef struct
{
  String8 text;  
  u64 at;
  u64 line;
  u64 bol; // beginning of line
  // b32 error;
} Lexer;

INTERNAL Lexer 
lexer_new(String8 text)
{
  Lexer lex = ZERO_STRUCT;
  lex.text = text;
  return lex;
}

INTERNAL void
lexer_consume_whitespace_left(Lexer *l)
{

}

INTERNAL Token
lexer_next(Lexer *l)
{
  lexer_consume_whitespace_left(l);

  Token token = ZERO_STRUCT;
  token.text.content = &l->text.content[l->at];

  if (l->at >= l->text.size) return token;

  if (l->text.content[l->at] == '#')
  {
    token.type = TOKEN_TYPE_HASH;
    token.text.size = 1;
    l->at += 1;
  }

  return token;
}

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
