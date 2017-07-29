#pragma once

#ifndef TYPEINFO_FILE
#define TYPEINFO_FILE "generated_typeinfo.h"
#endif
#include TYPEINFO_FILE

inline TypeInfo get_type_info(TypeInfo_ID id) {
  if (id > TypeInfo_ID_end_primitives) {
    i32 type_info_index = id - TypeInfo_ID_end_primitives - 1;
    return TypeInfo_custom_type_table[type_info_index];
  }

  switch (id) {
  case TypeInfo_ID_void: return { id, TypeKind_primitive, 0 };
  case TypeInfo_ID_u64: return { id, TypeKind_primitive, sizeof(u64) };
  case TypeInfo_ID_u32: return { id, TypeKind_primitive, sizeof(u32) };
  case TypeInfo_ID_u16: return { id, TypeKind_primitive, sizeof(u16) };
  case TypeInfo_ID_u8: return { id, TypeKind_primitive, sizeof(u8) };
  case TypeInfo_ID_i64: return { id, TypeKind_primitive, sizeof(i64) };
  case TypeInfo_ID_i32: return { id, TypeKind_primitive, sizeof(i32) };
  case TypeInfo_ID_i16: return { id, TypeKind_primitive, sizeof(i16) };
  case TypeInfo_ID_i8: return { id, TypeKind_primitive, sizeof(i8) };
  case TypeInfo_ID_b32: return { id, TypeKind_primitive, sizeof(b32) };
  case TypeInfo_ID_f32: return { id, TypeKind_primitive, sizeof(f32) };
  case TypeInfo_ID_f64: return { id, TypeKind_primitive, sizeof(f64) };
  case TypeInfo_ID_char: return { id, TypeKind_primitive, sizeof(char) };
  case TypeInfo_ID_int: return { id, TypeKind_primitive, sizeof(int) };
  default: assert(false);
  }
  return {};
}

inline b32 next_member(TypeInfo_ID id, MemberInfo** member_info) {
  assert(id > TypeInfo_ID_end_primitives);
  auto ti = get_type_info(id);
  if (!*member_info) {
    *member_info = TypeInfo_member_table + ti.members_start;
    assert(ti.members_start < ARRAY_LENGTH(TypeInfo_member_table));
    assert((*member_info)->parent_type == id);
    return true;
  } else {
    *member_info = *member_info + 1;
    if (*member_info - TypeInfo_member_table < ARRAY_LENGTH(TypeInfo_member_table) &&
      (*member_info)->parent_type == id) {
      return true;
    } else {
      *member_info = NULL;
      return false;
    }
  }
}

#define enum_member_name(type, value) enum_member_name_(TypeInfo_ID_##type, value)
inline char* enum_member_name_(TypeInfo_ID type_id, int value) {
  for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_member_table); i++) {
    auto ti = TypeInfo_member_table[i];
    if (ti.member_kind == MemberKind_enum && ti.parent_type == type_id && ti.enum_value == value) {
      return ti.member_name;
    }
  }

  return NULL;
}
