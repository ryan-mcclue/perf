// SPDX-License-Identifier: zlib-acknowledgement

typedef enum
{
  TOKEN_TYPE_NULL = 0, 
  TOKEN_TYPE_NEWLINE,
  TOKEN_TYPE_ERROR,
  TOKEN_TYPE_WHITESPACE,
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_STRING_LITERAL,
  TOKEN_TYPE_NUMERIC_LITERAL,

  TOKEN_TYPE_COUNT,
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
token_array_from_list(MemArena *arena, TokenList *list)
{
  TokenArray result = ZERO_STRUCT;
  result.tokens = MEM_ARENA_PUSH_ARRAY(arena, Token, list->count);

  u64 i = 0;
  for (TokenNode *n = list->first; n != NULL; n = n->next)
  {
    MEMORY_COPY(result.tokens + i, &n->token, sizeof(Token));
    i += 1;
  }

  result.count = list->count;

  return result;
}

INTERNAL TokenArray
get_token_array(MemArena *arena, String8 str)
{
  MemArenaTemp temp = mem_arena_temp_begin(&arena, 1);

  TokenList token_list = ZERO_STRUCT;

  TOKEN_TYPE cur_token_type = TOKEN_TYPE_NULL;
  u64 cur_token_start = 0;

  u64 off = 0;
  u64 inc = 0;
  while (off <= str.size)
  {
    char ch = str.content[off];
    char ch_next = (off+1 < str.size ? str.content[off+1] : '\0');

    b32 ender = false;

    inc = (cur_token_type != TOKEN_TYPE_NULL) ? 1 : 0;

    if (off == str.size && cur_token_type != TOKEN_TYPE_NULL)
    {
      ender = true;
      inc = 1;
    }

    switch (cur_token_type)
    {
      default:
        // identify start of token; following case statements determine end 
      case TOKEN_TYPE_NULL:
      {
        if ((ch == '\r' && ch_next == '\n') || ch == '\n')
        {
          cur_token_type = TOKEN_TYPE_NEWLINE;
          cur_token_start = off;
          inc = 0;
        }
        else if (ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f')
        {
          cur_token_type = TOKEN_TYPE_WHITESPACE;
          cur_token_start = off;
          inc = 1;
        }
        else
        {
          cur_token_start = off;
          cur_token_type = TOKEN_TYPE_ERROR;
          inc = 1;
        }
      } break;
      case TOKEN_TYPE_NEWLINE:
      {
        if (ch == '\r' && ch_next == '\n') inc = 2, ender = true;
        else if (ch == '\n') inc = 1, ender = true;
      } break;
      case TOKEN_TYPE_WHITESPACE:
      {
        if (ch != ' ' && ch != '\t' && ch != '\v' && ch != '\f')
        {
          ender = true;
          inc = 0;
        }
      } break;
      case TOKEN_TYPE_ERROR:
      {
        ender = true;
        inc = 0;
      } break;
    }

    if (ender)
    {
      TokenNode *token_node = MEM_ARENA_PUSH_STRUCT(temp.arena, TokenNode);
      token_node->token.type = cur_token_type; 
      token_node->token.range = range_u64(cur_token_start, off + inc);
      
      SLL_QUEUE_PUSH(token_list.first, token_list.last, token_node);
      token_list.count += 1;

      cur_token_type = TOKEN_TYPE_NULL;
      cur_token_start = token_node->token.range.max;
    }

    off += inc;
  }

  TokenArray result = token_array_from_list(arena, &token_list);

  mem_arena_temp_end(temp);

  return result;
}
