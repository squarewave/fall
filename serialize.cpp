#include "serialize.h"
#include "memory.h"

struct SerializedMemberInfo {
  size_t member_name_offset;
  size_t offset;
  char* member_name_ptr_do_not_use;
};

void push_members(TypeInfo ti, i32* member_count) {
  MemberInfo* member = NULL;
  while (next_member(ti.id, &member)) {
    (*member_count)++;
    assert(member->member_kind != MemberKind_array);
    assert(member->member_kind != MemberKind_pointer);
    assert(member->member_kind != MemberKind_array_of_pointers);

    SerializedMemberInfo out_member = {};
    out_member.offset = member->offset;
    out_member.member_name_ptr_do_not_use = member->member_name;
    stretchy_buffer_push(out_member);

    auto member_type = get_type_info(member->member_type);
    if (member_type.kind == TypeKind_struct) {
      push_members(member_type, member_count);
    }
  }
}

void* serialize_struct_array(TypeInfo_ID type_id, void* data, i32 count, size_t* out_size) {
  void* result = stretchy_buffer_init();
  size_t result_size = 0;
  auto mc = 0;
  TypeInfo ti = get_type_info(type_id);
  i32* member_count = (i32*)stretchy_buffer_push(mc);
  stretchy_buffer_push(count);
  stretchy_buffer_push(ti.size);
  auto ds = 0;
  i32* data_start = (i32*)stretchy_buffer_push(ds);
  SerializedMemberInfo* member_start = (SerializedMemberInfo*)stretchy_buffer_tip();
  push_members(ti, member_count);
  for (i32 i = 0; i < *member_count; i++) {
    char* ptr = sbprintf("%s", member_start[i].member_name_ptr_do_not_use);
    member_start[i].member_name_offset = (size_t)ptr - (size_t)member_start;
  }
  *data_start = (size_t)stretchy_buffer_tip() - (size_t)result;

  void* last = NULL;
  for (i32 i = 0; i < count; i++) {
    last = stretchy_buffer_push_(ti.size, (char*)data + ti.size * i);
  }

  assert(last);
  *out_size = ((size_t)last) + ti.size - (size_t)result;
  return result;
}

struct OffsetMapping {
  size_t in_offset;
  size_t out_offset;
  size_t size;
};

void push_offset_mappings_for_struct(TypeInfo_ID type_id, i32 in_member_count,
                                     SerializedMemberInfo* in_members, i32* offset_map_count,
                                     i32* member_index,
                                     size_t in_base_offset, size_t out_base_offset);

b32 push_offset_mappings_for_member(i32 in_member_count, SerializedMemberInfo* in_members,
                                    i32* offset_map_count, i32* member_index,
                                    MemberInfo* member, i32 i,
                                    size_t in_base_offset, size_t out_base_offset) {
  auto in_member = in_members[i];
  auto member_name = ((char*)in_members) + in_member.member_name_offset;
  if (!strcmp(member->member_name, member_name)) {
    auto member_type = get_type_info(member->member_type);
    if (member_type.kind == TypeKind_struct) {
      push_offset_mappings_for_struct(member->member_type, in_member_count, in_members,
                                      offset_map_count, member_index,
                                      in_base_offset + in_member.offset,
                                      out_base_offset + member->offset);
    } else {
      auto mapping = OffsetMapping{
        in_member.offset + in_base_offset,
        member->offset + out_base_offset,
        member_type.size
      };
      stretchy_buffer_push(mapping);
      (*offset_map_count)++;
    }
    return true;
  }
  return false;
}

void push_offset_mappings_for_struct(TypeInfo_ID type_id, i32 in_member_count,
                                     SerializedMemberInfo* in_members, i32* offset_map_count,
                                     i32* member_index,
                                     size_t in_base_offset, size_t out_base_offset) {
  MemberInfo* member = NULL;
  while (next_member(type_id, &member)) {
    assert(member->member_kind != MemberKind_array);
    assert(member->member_kind != MemberKind_pointer);
    assert(member->member_kind != MemberKind_array_of_pointers);
    i32 start_member_index = *member_index;
    b32 found = false;
    while (*member_index < in_member_count && !found) {
      found = push_offset_mappings_for_member(in_member_count, in_members,
                                              offset_map_count, member_index,
                                              member, *member_index,
                                              in_base_offset, out_base_offset);
      (*member_index)++;
    }

    *member_index = start_member_index;
    if (!found && start_member_index < in_member_count) {
      for (i32 i = 0; i < start_member_index; i++) {
        if (push_offset_mappings_for_member(in_member_count, in_members,
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

void deserialize_struct_array(TypeInfo_ID type_id, void* data, size_t data_size,
                              void* buffer, size_t buffer_size, i32* out_count) {
  i32* in_member_count = (i32*)data;
  i32* in_value_count = (i32*)(in_member_count + 1);
  size_t* in_struct_size = (size_t*)(in_value_count + 1);
  i32* in_data_offset = (i32*)(in_struct_size + 1);
  SerializedMemberInfo* in_members = (SerializedMemberInfo*)(in_data_offset + 1);
  void* in_data = (char*)data + *in_data_offset;

  OffsetMapping* offset_map = (OffsetMapping*)stretchy_buffer_init();
  i32 offset_map_count = 0;

  i32 member_index = 0;
  push_offset_mappings_for_struct(type_id, *in_member_count, in_members, &offset_map_count, &member_index, 0, 0);

  auto ti = get_type_info(type_id);

  for (i32 i = 0; i < *in_value_count; i++) {
    char* out_base = (char*)buffer + ti.size * i;
    char* in_base = (char*)in_data + *in_struct_size * i;
    for (i32 j = 0; j < offset_map_count; j++) {
      auto map = offset_map[j];
      memcpy(out_base + map.out_offset, in_base + map.in_offset, map.size);
    }
  }
  *out_count = *in_value_count;
}
