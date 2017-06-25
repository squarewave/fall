#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <Windows.h>

#include "stb/stretchy_buffer.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

enum TokenType {
  TOKEN_L_PAREN,
  TOKEN_R_PAREN,
  TOKEN_L_BRACE,
  TOKEN_R_BRACE,
  TOKEN_L_BRACKET,
  TOKEN_R_BRACKET,
  TOKEN_COMMA,
  TOKEN_PERIOD,
  TOKEN_ASTERISK,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_EXCLAMATION,
  TOKEN_TILDE,
  TOKEN_BACKSLASH,
  TOKEN_SLASH,
  TOKEN_EQ,
  TOKEN_AMPERSAND,
  TOKEN_POUND,
  TOKEN_SEMICOLON,
  TOKEN_COLON,
  TOKEN_QUESTION_MARK,
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER,
  TOKEN_CHARACTER,
  TOKEN_STRING,
  TOKEN_UNKNOWN,
  TOKEN_EOF,
};

typedef struct {
  int len;
  char *data;
} Slice;

typedef struct {
  uint32_t type;
  Slice slice;
} Token;

enum TypeKind {
  TypeKind_struct,
  TypeKind_enum,
  TypeKind_union,
};

enum MemberKind {
  MemberKind_enum,
  MemberKind_array,
  MemberKind_pointer,
  MemberKind_array_of_pointers,
  MemberKind_value,
};

typedef struct {
  Slice type_name;
  TypeKind kind;
  int members_start;
} Type;

typedef struct {
  MemberKind kind;
  int enum_value;
  int parent_type;
  Slice type_name;
  Slice member_name;
} Member;

typedef struct {
  Token* tokens;
  int current;

  Type* types;
  Member* members;
} Parser;

Slice as_slice(char *s, int64_t len = -1) {
  Slice result;

  if (len == -1) {
    len = strlen(s);
  }
  result.len = len;
  result.data = s;
  return result;
}

Slice slice(Slice s, int64_t start, int64_t end) {
  Slice result;

  result.len = end - start;
  result.data = s.data + start;
  return result;
}

Slice slice_r(Slice s, int64_t start) {
  Slice result;

  result.len = s.len - start;
  result.data = s.data + start;
  return result;
}

Slice slice_l(Slice s, int64_t end) {
  Slice result;

  result.len = end;
  result.data = s.data;
  return result;
}

Slice copy_slice(Slice s) {
  Slice result;

  result.data = (char *)malloc(s.len);
  result.len = s.len;
  memcpy(result.data, s.data, s.len);
  return result;
}

bool slices_equal(Slice lhs, Slice rhs) {
  if (lhs.len != rhs.len)
    return false;
  else
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

char **get_directory(char *start_dir, int64_t *file_count) {
  char **result = 0;

  char *stack[1024] = { 0 };
  int32_t stack_size = 0;

  stack[stack_size++] = start_dir;

  while (stack_size) {
    char *dir_path = stack[--stack_size];

    char path[2048];
    snprintf(path, ARRAY_LENGTH(path), "%s/*.*", dir_path);

    WIN32_FIND_DATA fd;
    HANDLE handle;
    if ((handle = FindFirstFile(path, &fd)) == INVALID_HANDLE_VALUE)
      continue;

    do {
      if (strcmp(fd.cFileName, ".") != 0 &&
          strcmp(fd.cFileName, "..") != 0) {
        snprintf(path, ARRAY_LENGTH(path), "%s/%s", dir_path,
                 fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          int64_t path_length = strlen(path) + 1;
          stack[stack_size] = (char *)malloc(path_length);
          memcpy(stack[stack_size], path, path_length);
          stack_size++;
        } else {
          int64_t path_length = strlen(path) + 1;
          char *filename = (char *)malloc(path_length);
          memcpy(filename, path, path_length);

          sb_push(result, filename);
        }
      }
    } while (FindNextFile(handle, &fd));

    FindClose(handle);

    if (dir_path != start_dir)
      free(dir_path);
  }

  *file_count = sb_count(result);
  return result;
}

int32_t is_whitespace(char c) {
  return c == ' ' || (c >= '\t' && c <= '\r');
}

Slice eat_whitespace_and_comments(Slice s) {
  char *data = s.data;
  int64_t len = s.len;

  while (len > 0) {
    if (is_whitespace(*data)) {
      ++data;
      --len;
    } else if (len > 1 && data[0] == '#') {
      ++data;
      --len;
      while (len > 0 && *data != '\n') {
        ++data;
        --len;
      }
    } else if (len > 1 && data[0] == '/' && data[1] == '/') {
      data += 2;
      len -= 2;
      while (len > 0 && *data != '\n') {
        ++data;
        --len;
      }
    } else if (len > 1 && data[0] == '/' && data[1] == '*') {
      data += 2;
      len -= 2;
      while (len > 0 && !(data[0] == '*' && len > 1 && data[1] == '/')) {
        ++data;
        --len;
      }
      if (len > 2) {
        data += 2;
        len -= 2;
      } else {
        data += len;
        len = 0;
      }
    } else {
      break;
    }
  }

  Slice result;
  result.data = data;
  result.len = len;
  return result;
}

int32_t is_digit(char c) {
  return c >= '0' && c <= '9';
}

int32_t is_word_char(char c) {
  return (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    (c == '_');
}

int32_t is_identifier(char c) {
  return is_word_char(c) || is_digit(c);
}

bool lex_identifier(Slice *c, Token *token) {
  if (is_word_char(*c->data)) {
    int32_t len = 1;
    while (c->len - len) {
      if (is_word_char(c->data[len]) || is_digit(c->data[len]))
        ++len;
      else
        break;
    }

    Slice token_slice = slice_l(*c, len);
    *c = slice_r(*c, len);
    token->type = TOKEN_IDENTIFIER;
    token->slice = token_slice;
    return true;
  } else {
    return false;
  }
}

bool lex_constant(Slice *c, Token *token) {
  uint32_t token_type;

  if (is_digit(*c->data)) {
    token_type = TOKEN_NUMBER;
    int32_t len = 1;
    while (c->len - len) {
      char ch = c->data[len];
      if (!is_digit(ch) &&
          ch != '.' &&
          ch != 'x' &&
          ch != 'X')
        break;
      len++;
    }

    Slice token_slice = slice_l(*c, len);
    *c = slice_r(*c, len);
    token->type = token_type;
    token->slice = token_slice;
    return true;
  } else if (*c->data == '\'') {
    token_type = TOKEN_CHARACTER;
    int32_t len = 1;
    while (c->len - len - 1) {
      if (c->data[len] == '\\') {
        if (c->data[len + 1] == 'x') {
          len = min(len + 4, (int32_t)c->len + 1);
          continue;
        } else {
          len = min(len + 2, (int32_t)c->len + 1);
          continue;
        }
      }
      if (c->data[len] == '\'')
        break;
      len++;
    }

    Slice token_slice = slice_l(*c, len + 1);
    *c = slice_r(*c, len + 1);
    token->type = token_type;
    token->slice = token_slice;
    return true;
  } else {
    return false;
  }
}

bool lex_string(Slice *c, Token *token) {
  if (*c->data == '"') {
    int32_t len = 1;
    while (c->len - len - 1) {
      if (c->data[len] == '\\') {
        if (c->data[len + 1] == 'x') {
          len = min(len + 4, (int32_t)c->len + 1);
          continue;
        } else {
          len = min(len + 2, (int32_t)c->len + 1);
          continue;
        }
      }
      if (c->data[len] == '"')
        break;
      len++;
    }

    Slice token_slice = slice_l(*c, len + 1);
    *c = slice_r(*c, len + 1);
    token->type = TOKEN_STRING;
    token->slice = token_slice;
    return true;
  } else {
    return false;
  }
}

bool lex_single_char_token(Slice *c, Token *token) {
  uint32_t token_type;

  switch (*c->data) {
  case '(':
  {
    token_type = TOKEN_L_PAREN; break;
  }
  case ')':
  {
    token_type = TOKEN_R_PAREN; break;
  }
  case '{':
  {
    token_type = TOKEN_L_BRACE; break;
  }
  case '}':
  {
    token_type = TOKEN_R_BRACE; break;
  }
  case '[':
  {
    token_type = TOKEN_L_BRACKET; break;
  }
  case ']':
  {
    token_type = TOKEN_R_BRACKET; break;
  }
  case '*':
  {
    token_type = TOKEN_ASTERISK; break;
  }
  case ',':
  {
    token_type = TOKEN_COMMA; break;
  }
  case '.':
  {
    token_type = TOKEN_PERIOD; break;
  }
  case '-':
  {
    token_type = TOKEN_MINUS; break;
  }
  case '+':
  {
    token_type = TOKEN_PLUS; break;
  }
  case '!':
  {
    token_type = TOKEN_EXCLAMATION; break;
  }
  case '~':
  {
    token_type = TOKEN_TILDE; break;
  }
  case '\\':
  {
    token_type = TOKEN_BACKSLASH; break;
  }
  case '/':
  {
    token_type = TOKEN_SLASH; break;
  }
  case '=':
  {
    token_type = TOKEN_EQ; break;
  }
  case '&':
  {
    token_type = TOKEN_AMPERSAND; break;
  }
  case '#':
  {
    token_type = TOKEN_POUND; break;
  }
  case ';':
  {
    token_type = TOKEN_SEMICOLON; break;
  }
  case ':':
  {
    token_type = TOKEN_COLON; break;
  }
  case '?':
  {
    token_type = TOKEN_QUESTION_MARK; break;
  }
  default:
  {
    token_type = TOKEN_UNKNOWN; break;
  }
  }

  if (token_type != TOKEN_UNKNOWN) {
    auto token_slice = slice_l(*c, 1);
    *c = slice_r(*c, 1);
    token->type = token_type;
    token->slice = token_slice;
    return true;
  } else {
    return false;
  }
}

Token *lex_file(Slice s) {
  int32_t capacity = 64;
  Token *result = (Token *)malloc(capacity * sizeof(Token));
  Slice c = s;

  int32_t i = 0;

  while (true) {
    if (i >= capacity) {
      capacity += capacity / 2;
      result = (Token *)realloc(result, capacity * sizeof(Token));
    }

    Token *t = &result[i];

    if (!c.len) {
      t->type = TOKEN_EOF;
      t->slice = slice_l(c, 0);
      break;
    } else {
      c = eat_whitespace_and_comments(c);

      if (c.len) {
        if (lex_single_char_token(&c, t)) {
        } else if (lex_constant(&c, t)) {
        } else if (lex_string(&c, t)) {
        } else if (lex_identifier(&c, t)) {
        } else {
          t->type = TOKEN_UNKNOWN;
          t->slice = slice_l(c, 1);
          c = slice_r(c, 1);
        }

        assert(c.len >= 0);
        ++i;
      }
    }
  }
  return result;
}

Slice read_entire_file(char *f) {
  FILE *fp;
  fopen_s(&fp, f, "rb");

  if (!fp)
    return as_slice(0, 0);

  fseek(fp, 0, SEEK_END);
  int64_t len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *data = (char *)malloc(len + 1);
  if (!fread(data, len, 1, fp))
    return as_slice(0, 0);

  // ensure string result is null-terminated
  data[len] = 0;

  fclose(fp);

  return as_slice(data, len);
}

void free_file(Slice slice) {
  free(slice.data);
}

bool match_identifier(Token token, char *identifier) {
  if (token.type != TOKEN_IDENTIFIER)
    return false;

  for (int32_t i = 0; i < token.slice.len; i++)
    if (!identifier[i] || token.slice.data[i] != identifier[i])
      return false;

  return !identifier[token.slice.len];
}

bool match_string(Token token, char *s) {
  if (token.type != TOKEN_STRING)
    return false;

  for (int32_t i = 1; i < token.slice.len - 1; i++)
    if (!s[i - 1] || token.slice.data[i] != s[i - 1])
      return false;

  return !s[token.slice.len - 2];
}

Token get_token(Parser* parser) {
  return parser->tokens[parser->current++];
}

bool require_token(Parser* parser, int token_type) {
  return get_token(parser).type == token_type;
}

bool require_identifier(Parser* parser, char* identifier) {
  Token t = get_token(parser);
  return match_identifier(t, identifier);
}

bool parse_enum_member(Parser* parser, int parent_type, Token type_token, int* index) {
  if (type_token.type == TOKEN_IDENTIFIER) {
    auto member_name = type_token.slice;
    auto t = get_token(parser);

    if (t.type == TOKEN_COMMA || t.type == TOKEN_R_BRACE) {
      Member m = {};
      m.kind = MemberKind_enum;
      m.enum_value = (*index)++;
      m.member_name = member_name;
      m.parent_type = parent_type;
      sb_push(parser->members, m);
    } else if (t.type == TOKEN_EQ) {
      auto constant = get_token(parser);
      if (constant.type == TOKEN_NUMBER) {
        char str[64] = {};
        memcpy(str, constant.slice.data, constant.slice.len);
        Member m = {};
        m.kind = MemberKind_enum;
        *index = atoi(str);
        m.enum_value = (*index)++;
        m.member_name = member_name;
        m.parent_type = parent_type;
        sb_push(parser->members, m);
      }
    }
    
    return t.type != TOKEN_R_BRACE;
  }
  return true;
}

void parse_member(Parser* parser, int parent_struct, Token type_token) {
  if (type_token.type == TOKEN_IDENTIFIER) {
    auto type_name = type_token.slice;
    bool pointer = false;
    bool array = false;
    auto t = get_token(parser);
    if (t.type == TOKEN_ASTERISK) {
      pointer = true;
      t = get_token(parser);
    }

    auto next = get_token(parser);
    if (next.type == TOKEN_L_BRACKET) {
      array = true;
    }

    if (t.type == TOKEN_IDENTIFIER) {
      auto member_name = t.slice;
      Member m = {};
      if (pointer && array) {
        m.kind = MemberKind_array_of_pointers;
      } else if (pointer) {
        m.kind = MemberKind_pointer;
      } else if (array) {
        m.kind = MemberKind_array;
      } else {
        m.kind = MemberKind_value;
      }
      m.member_name = member_name;
      m.type_name = type_name;
      m.parent_type = parent_struct;
      sb_push(parser->members, m);
    }

    if (next.type != TOKEN_SEMICOLON && next.type != TOKEN_EOF && next.type != TOKEN_R_BRACE) {
      while (true) {
        auto t = get_token(parser);
        if (t.type == TOKEN_SEMICOLON || t.type == TOKEN_EOF || t.type == TOKEN_R_BRACE) {
          break;
        }
      }
    }
  }
}

void parse_struct(Parser* parser) {
  auto name_token = get_token(parser);
  if (name_token.type == TOKEN_IDENTIFIER) {
    auto struct_name = name_token.slice;
    printf("// %.*s\n", struct_name.len, struct_name.data);
    if (require_token(parser, TOKEN_L_BRACE)) {
      Type s = {};
      s.kind = TypeKind_struct;
      s.type_name = struct_name;
      sb_push(parser->types, s);
      while (true) {
        auto t = get_token(parser);
        if (t.type == TOKEN_EOF || t.type == TOKEN_R_BRACE) {
          break;
        }
        if (match_identifier(t, "ignore")) {
          while (true) {
            auto t1 = get_token(parser);
            if (t1.type == TOKEN_SEMICOLON || t1.type == TOKEN_EOF || t1.type == TOKEN_R_BRACE) {
              break;
            }
          }
          continue;
        }

        parse_member(parser, sb_count(parser->types) - 1, t);
      }
    }
  }
}

void parse_union(Parser* parser) {
  auto name_token = get_token(parser);
  if (name_token.type == TOKEN_IDENTIFIER) {
    auto struct_name = name_token.slice;
    if (require_token(parser, TOKEN_L_BRACE)) {
      Type s = {};
      s.kind = TypeKind_union;
      s.type_name = struct_name;
      sb_push(parser->types, s);
      while (true) {
        auto t = get_token(parser);
        if (t.type == TOKEN_EOF || t.type == TOKEN_R_BRACE) {
          break;
        }

        parse_member(parser, sb_count(parser->types) - 1, t);
      }
    }
  }
}

void parse_enum(Parser* parser) {
  auto name_token = get_token(parser);
  if (name_token.type == TOKEN_IDENTIFIER) {
    auto struct_name = name_token.slice;
    if (require_token(parser, TOKEN_L_BRACE)) {
      Type s = {};
      s.kind = TypeKind_enum;
      s.type_name = struct_name;
      sb_push(parser->types, s);
      int index = 0;
      while (true) {
        auto t = get_token(parser);
        if (t.type == TOKEN_EOF || t.type == TOKEN_R_BRACE) {
          break;
        }

        if (t.type == TOKEN_IDENTIFIER) {
          if (!parse_enum_member(parser, sb_count(parser->types) - 1, t, &index)) {
            break;
          }
        }
      }
    }
  }
}

void parse_file(Parser* parser) {
  while (true) {
    auto token = get_token(parser);
    if (token.type == TOKEN_EOF) {
      break;
    } else if (match_identifier(token, "reflectable")) {
      auto t = get_token(parser);
      if (match_identifier(t, "struct")) {
        parse_struct(parser);
      } else if (match_identifier(t, "enum")) {
        parse_enum(parser);
      } else if (match_identifier(t, "union")) {
        parse_union(parser);
      }
    }
  }
}

const char* get_member_kind_str(MemberKind member_kind) {
  switch (member_kind)
  {
  case MemberKind_enum: return "MemberKind_enum";
  case MemberKind_array: return "MemberKind_array";
  case MemberKind_pointer: return "MemberKind_pointer";
  case MemberKind_array_of_pointers: return "MemberKind_array_of_pointers";
  case MemberKind_value: return "MemberKind_value";
  }

  assert(false);
  return NULL;
}

const char* get_type_kind_str(TypeKind type_kind) {
  switch (type_kind)
  {
  case TypeKind_struct: return "TypeKind_struct";
  case TypeKind_enum: return "TypeKind_enum";
  case TypeKind_union: return "TypeKind_union";
  }

  assert(false);
  return NULL;
}

int main(int argc, char** argv) {
  char* files[] = {
    "platform.h",
    "editor.h",
    "meat_space.h",
    "assets.h",
    "geometry.h",
    "game.h",
    "grid.h",
    "asset_manager.h",
  };

  printf("#pragma once\n\n");
  Parser p = {};
  for (int32_t i = 0; i < ARRAY_LENGTH(files); i++) {
    printf("#include \"%s\"\n", files[i]);

    auto filedata = read_entire_file(files[i]);
    if (!filedata.data) {
      printf("Failed to read file: %s\n", files[i]);
      continue;
    }

    auto token = lex_file(filedata);
    p.current = 0;
    p.tokens = token;
    parse_file(&p);

    // Don't bother freeing file data. Computers have lots of memory
  }

  printf(R"(enum TypeInfo_ID {
  TypeInfo_ID_none,

  TypeInfo_ID_void,

  TypeInfo_ID_u64,
  TypeInfo_ID_u32,
  TypeInfo_ID_u16,
  TypeInfo_ID_u8,
  TypeInfo_ID_i64,
  TypeInfo_ID_i32,
  TypeInfo_ID_i16,
  TypeInfo_ID_i8,
  TypeInfo_ID_b32,
  TypeInfo_ID_f32,
  TypeInfo_ID_f64,

  TypeInfo_ID_char,
  TypeInfo_ID_int,

  TypeInfo_ID_end_primitives,
)");

  for (int i = 0; i < sb_count(p.types); i++) {
    printf("  TypeInfo_ID_%.*s,\n", p.types[i].type_name.len, p.types[i].type_name.data);
  }

  printf("\n  TypeInfo_ID_count\n");
  printf("};\n");

  puts(R"(
enum TypeKind {
  TypeKind_struct,
  TypeKind_enum,
  TypeKind_union,
  TypeKind_primitive,
};

enum MemberKind {
  MemberKind_enum,
  MemberKind_array,
  MemberKind_pointer,
  MemberKind_array_of_pointers,
  MemberKind_value,
};

struct MemberInfo {
  char* member_name;
  TypeInfo_ID parent_type;
  TypeInfo_ID member_type;
  MemberKind member_kind;
  int enum_value;
  size_t offset;
  size_t array_size;
  int table_index;
};

MemberInfo TypeInfo_member_table[] = {)");

  int last_parent_type = -1;
  for (int i = 0; i < sb_count(p.members); i++) {
    auto m = p.members[i];
    if (m.parent_type != last_parent_type) {
      p.types[m.parent_type].members_start = i;
      last_parent_type = m.parent_type;
    }
    auto struct_name = p.types[m.parent_type].type_name;
    if (m.kind == MemberKind_enum) {
      printf("  {\"%.*s\", TypeInfo_ID_%.*s, TypeInfo_ID_i32, %s, %d, 0, 0, %d},\n",
             m.member_name.len, m.member_name.data,
             struct_name.len, struct_name.data,
             get_member_kind_str(m.kind),
             m.enum_value,
             i);
    } else if (m.kind == MemberKind_array) {
      printf("  {\"%.*s\", TypeInfo_ID_%.*s, TypeInfo_ID_%.*s, %s, %d, (size_t)&((%.*s*)0)->%.*s, ARRAY_LENGTH(((%.*s*)0)->%.*s),%d},\n",
             m.member_name.len, m.member_name.data,
             struct_name.len, struct_name.data,
             m.type_name.len, m.type_name.data,
             get_member_kind_str(m.kind),
             m.enum_value,
             struct_name.len, struct_name.data,
             m.member_name.len, m.member_name.data,
             struct_name.len, struct_name.data,
             m.member_name.len, m.member_name.data,
             i);
    } else {
      printf("  {\"%.*s\", TypeInfo_ID_%.*s, TypeInfo_ID_%.*s, %s, %d, (size_t)&((%.*s*)0)->%.*s, 0, %d},\n",
             m.member_name.len, m.member_name.data,
             struct_name.len, struct_name.data,
             m.type_name.len, m.type_name.data,
             get_member_kind_str(m.kind),
             m.enum_value,
             struct_name.len, struct_name.data,
             m.member_name.len, m.member_name.data,
             i);
    }
  }

  puts(R"(
};

struct TypeInfo {
  TypeInfo_ID id;
  TypeKind kind;
  size_t size;
  int members_start;
};

TypeInfo TypeInfo_custom_type_table[] = {)");

  for (int i = 0; i < sb_count(p.types); i++) {
    auto t = p.types[i];
    printf("  {TypeInfo_ID_%.*s, %s, sizeof(%.*s), %d},\n",
           t.type_name.len, t.type_name.data,
           get_type_kind_str(t.kind),
           t.type_name.len, t.type_name.data,
           t.members_start);
  }

  puts(R"(
};

struct UnionMemberInfo {
  TypeInfo_ID union_type_id;
  i32 type;
  TypeInfo_ID union_member_type_id;
};

UnionMemberInfo TypeInfo_union_member_table[] = {)");

  for (int i = 0; i < sb_count(p.members); i++) {
    auto m = p.members[i];
    auto parent_kind = p.types[m.parent_type].kind;
    auto parent_type_name = p.types[m.parent_type].type_name;
    auto member_name = m.member_name;
    auto member_type_name = m.type_name;
    if (parent_kind == TypeKind_union && !slices_equal(member_name, as_slice("type"))) {
      printf("  {TypeInfo_ID_%.*s, %.*s_Type_%.*s, TypeInfo_ID_%.*s},\n",
             parent_type_name.len, parent_type_name.data,
             parent_type_name.len, parent_type_name.data,
             member_name.len, member_name.data,
             member_type_name.len, member_type_name.data);
    }
  }

  printf("};\n");

  return 0;
}
