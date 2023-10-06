// SPDX-License-Identifier: zlib-acknowledgement

typedef enum
{
  TOKEN_TYPE_NULL = 0, 
  TOKEN_TYPE_KEYWORD, // true/false/null
  TOKEN_TYPE_SEMICOLON,
} TOKEN_TYPE;

typedef struct Token Token;
struct Token
{
  TOKEN_TYPE type;
  RangeU64 range;
};

typedef struct TokenNode TokenNode;
struct TokenNode
{
  TokenNode *next;
  Token token;
};

typedef struct TokenList TokenList;
struct TokenList
{
  TokenNode *first, *last;
  u64 count;
};

typedef struct TokenArray TokenArray;
struct TokenArray
{
  Token *tokens;
  u64 count;
};

INTERNAL TokenArray
get_token_array(MemArena *arena, String8 str)
{
  MemArenaTemp temp = mem_arena_temp_begin(&arena, 1);

  TokenList tokens = ZERO_STRUCT;

  TOKEN_TYPE cur_token_type = TOKEN_TYPE_NULL;
  u64 cur_token_start = 0;

  u64 at = 0;
  u64 inc = 0;
  while (at <= str.size)
  {
    char ch = str.content[at];
    char ch_next = (at+1 < str.size ? str.content[at+1] : '\0');

    // ??
    // if(off == string.size && active_token_kind != CL_TokenKind_Null)
    // {
    //  ender_found = 1;
    //  advance = 1;
    // }

    inc = (cur_token_type != TOKEN_TYPE_NULL) ? 1 : 0;

    b32 ender_found = false;

    switch (cur_token_type)
    {
      default:
        // no token yet, so case statements 1 behind
      case TOKEN_TYPE_NULL:
      {
        if ((ch == '\r' && ch_next == '\n') || ch == '\n')
        {
          cur_token_type = TOKEN_TYPE_NEWLINE;
          cur_token_start = at;
          advance = 0; // why zero?
        }
      } break;
    }

    if (ender_found)
    {
      Token token = {active_token_type, rng1_u32(active_token_start_off, offset + advance)};
      CL_TokenChunkListPush(arena, &tokens, 1024, &token);
      // token_list_push()
      active_token_kind = CL_TokenKind_Null;
      active_token_start_off = token.range.max;
    }

    at += advance;
  }

  TokenArray result = token_array_from_list();

  mem_arena_temp_end(temp);
}
