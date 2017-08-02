#include "serialize.h"
#include "memory.h"

#define MEMBER_INFO_FLAG_CSTRING  0x01

struct SerializedMemberInfo {
  size_t member_name_offset;
  size_t offset;
  union {
    char* member_name_ptr_do_not_use;
    char flags;
    char reserved[8];
  };
};

struct StringMemberMetadata {
  i32 member_index;
  size_t offset;
};

void push_members(Allocator* a, TypeInfo ti, i32* member_count,
                  StringMemberMetadata* string_members, i32* string_members_count) {
  MemberInfo* member = NULL;
  while (next_member(ti.id, &member)) {
    (*member_count)++;
    assert(member->member_kind != MemberKind_array);
    assert(member->member_kind != MemberKind_array_of_pointers);
    if (has_flag(member->flags, TYPEINFO_MEMBER_FLAG_CSTRING)) {
      assert(member->member_kind == MemberKind_pointer && member->member_type == TypeInfo_ID_char);
      SerializedMemberInfo out_member = {};
      out_member.offset = member->offset;
      out_member.member_name_ptr_do_not_use = member->member_name;
      stretchy_buffer_push(a, out_member);
      auto string_member = &string_members[(*string_members_count)++];
      string_member->offset = member->offset;
      string_member->member_index = *member_count - 1;
    } else {
      assert(member->member_kind != MemberKind_pointer);
      SerializedMemberInfo out_member = {};
      out_member.offset = member->offset;
      out_member.member_name_ptr_do_not_use = member->member_name;
      stretchy_buffer_push(a, out_member);

      auto member_type = get_type_info(member->member_type);
      if (member_type.kind == TypeKind_struct) {
        push_members(a, member_type, member_count, string_members, string_members_count);
      }
    }
  }
}

struct MemberString {
  char* value;
  char** serialized_member;
};

void* serialize_struct_array(Allocator* a, TypeInfo_ID type_id, void* data, i32 count, size_t* out_size) {
  i32 string_members_count = 0;
  StringMemberMetadata* string_members = allocator_alloc_array(a, StringMemberMetadata, 1024);
  void* result = stretchy_buffer_init(a);
  size_t result_size = 0;
  auto mc = 0;
  TypeInfo ti = get_type_info(type_id);
  i32* member_count = (i32*)stretchy_buffer_push(a, mc);
  stretchy_buffer_push(a, count);
  stretchy_buffer_push(a, ti.size);
  auto ds = 0;
  i32* data_start = (i32*)stretchy_buffer_push(a, ds);
  SerializedMemberInfo* member_start = (SerializedMemberInfo*)stretchy_buffer_tip(a);
  push_members(a, ti, member_count, string_members, &string_members_count);
  for (i32 i = 0; i < *member_count; i++) {
    char* ptr = sbprintf(a, "%s", member_start[i].member_name_ptr_do_not_use);
    member_start[i].member_name_offset = ((size_t)ptr) - (size_t)member_start;
    member_start[i].flags = 0;
  }
  *data_start = (size_t)stretchy_buffer_tip(a) - (size_t)result;

  void* out_items_start = stretchy_buffer_tip(a);
  void* last = NULL;
  for (i32 i = 0; i < count; i++) {
    last = stretchy_buffer_push_(a, ti.size, (char*)data + ti.size * i);
  }

  for (i32 i = 0; i < string_members_count; i++) {
    set_flag(member_start[string_members[i].member_index].flags, MEMBER_INFO_FLAG_CSTRING);
    for (i32 j = 0; j < count; j++) {
      char* item = (char*)out_items_start + ti.size * j;
      char** str = (char**)(item + string_members[i].offset);
      char* new_loc = sbprintf(a, "%s", *str);
      *str = (char*)(((size_t)new_loc) - (size_t)member_start);
    }
  }
  last = stretchy_buffer_tip(a);

  assert(last);
  *out_size = ((size_t)last) - (size_t)result;
  return result;
}

struct OffsetMapping {
  size_t in_offset;
  size_t out_offset;
  size_t size;
  char flags;
};

void push_offset_mappings_for_struct(Allocator* a, TypeInfo_ID type_id, i32 in_member_count,
                                     SerializedMemberInfo* in_members, i32* offset_map_count,
                                     i32* member_index,
                                     size_t in_base_offset, size_t out_base_offset);

b32 push_offset_mappings_for_member(Allocator* a, i32 in_member_count, SerializedMemberInfo* in_members,
                                    i32* offset_map_count, i32* member_index,
                                    MemberInfo* member, i32 i,
                                    size_t in_base_offset, size_t out_base_offset) {
  auto in_member = in_members[i];
  auto member_name = ((char*)in_members) + in_member.member_name_offset;
  if (!strcmp(member->member_name, member_name)) {
    auto member_type = get_type_info(member->member_type);
    if (member_type.kind == TypeKind_struct) {
      push_offset_mappings_for_struct(a, member->member_type, in_member_count, in_members,
                                      offset_map_count, member_index,
                                      in_base_offset + in_member.offset,
                                      out_base_offset + member->offset);
    } else {
      size_t size = member_type.size;
      if (member->member_kind == MemberKind_pointer) {
        size = sizeof(void*);
      }
      auto mapping = OffsetMapping{
        in_member.offset + in_base_offset,
        member->offset + out_base_offset,
        size,
        member->flags,
      };
      stretchy_buffer_push(a, mapping);
      (*offset_map_count)++;
    }
    return true;
  }
  return false;
}

void push_offset_mappings_for_struct(Allocator* a, TypeInfo_ID type_id, i32 in_member_count,
                                     SerializedMemberInfo* in_members, i32* offset_map_count,
                                     i32* member_index,
                                     size_t in_base_offset, size_t out_base_offset) {
  MemberInfo* member = NULL;
  MemberInfoIterator it = { 0 };
  while (next_member_flattened(type_id, &member, &it)) {
    assert(member->member_kind != MemberKind_array);
    assert(member->member_kind != MemberKind_array_of_pointers);
    if (member->member_kind == MemberKind_pointer) {
      assert(has_flag(member->flags, MEMBER_INFO_FLAG_CSTRING));
    }
    i32 start_member_index = *member_index;
    b32 found = false;
    while (*member_index < in_member_count && !found) {
      found = push_offset_mappings_for_member(a, in_member_count, in_members,
                                              offset_map_count, member_index,
                                              member, *member_index,
                                              in_base_offset, out_base_offset);
      (*member_index)++;
    }

    *member_index = start_member_index;
    if (!found && start_member_index < in_member_count) {
      for (i32 i = 0; i < start_member_index; i++) {
        if (push_offset_mappings_for_member(a, in_member_count, in_members,
                                            offset_map_count, member_index,
                                            member, i,
                                            in_base_offset, out_base_offset)) {
          break;
        }
      }
    }

    (*member_index)++;
  }
}

void* deserialize_struct_array(Allocator* a, TypeInfo_ID type_id, void* data,
                               size_t data_size, i32* out_count,
                               i32 string_buffer_length) {
  i32* in_member_count = (i32*)data;
  i32* in_value_count = (i32*)(in_member_count + 1);
  size_t* in_struct_size = (size_t*)(in_value_count + 1);
  i32* in_data_offset = (i32*)(in_struct_size + 1);
  SerializedMemberInfo* in_members = (SerializedMemberInfo*)(in_data_offset + 1);
  void* in_data = (char*)data + *in_data_offset;

  OffsetMapping* offset_map = (OffsetMapping*)stretchy_buffer_init(a);
  i32 offset_map_count = 0;

  i32 member_index = 0;
  push_offset_mappings_for_struct(a, type_id, *in_member_count, in_members,
                                  &offset_map_count, &member_index, 0, 0);

  void* result = stretchy_buffer_init(a);
  auto ti = get_type_info(type_id);

  for (i32 i = 0; i < *in_value_count; i++) {
    char* out_base = (char*)stretchy_buffer_grow_(a, ti.size);
    char* in_base = (char*)in_data + *in_struct_size * i;
    for (i32 j = 0; j < offset_map_count; j++) {
      auto map = offset_map[j];
      memcpy(out_base + map.out_offset, in_base + map.in_offset, map.size);
    }
  }

  for (i32 i = 0; i < *in_value_count; i++) {
    char* out_base = (char*)result + ti.size * i;
    char* in_base = (char*)in_data + *in_struct_size * i;
    for (i32 j = 0; j < offset_map_count; j++) {
      auto map = offset_map[j];
      if (has_flag(map.flags, MEMBER_INFO_FLAG_CSTRING)) {
        char** as_ptr = (char**)(out_base + map.out_offset);
        size_t as_size_t = (size_t)*as_ptr;
        char* to_copy = ((char*)in_members) + as_size_t;
        if (string_buffer_length == -1) {
          *as_ptr = sbprintf(a, "%s", to_copy);
        } else {
          *as_ptr = sbnprintf(a, string_buffer_length, "%s", to_copy);
        }
      }
    }
  }
  *out_count = *in_value_count;
  return result;
}
