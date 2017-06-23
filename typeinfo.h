#pragma once

#include "generated_typeinfo.h"

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