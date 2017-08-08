#include <inttypes.h>

#include "platform.h"
#include "render_commands.h"
#include "imgui/imgui.h"
#include "debug.h"
#include "memory.h"

char debug_print_copy_buffer[DEBUG_PRINT_RING_BUFFER_SIZE + 1] = {0};

using namespace ImGui;

void show_debug_log() {
  SetNextWindowPos(ImVec2(g_render_commands->screen_width - 410.0f, 120.0f));
  if (Begin("Output", NULL, ImVec2(400.0f, 800.0f), 0.3f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
    if (*g_debug_print_ring_buffer_write_head < DEBUG_PRINT_RING_BUFFER_SIZE) {
      Text("%s", (const char*)g_debug_print_ring_buffer);
    } else {
      auto write_head = *g_debug_print_ring_buffer_write_head & DEBUG_PRINT_RING_BUFFER_MASK;
      strcpy_s(debug_print_copy_buffer,
               ARRAY_LENGTH(debug_print_copy_buffer),
               g_debug_print_ring_buffer + write_head + 1);
      strcpy_s(debug_print_copy_buffer + DEBUG_PRINT_RING_BUFFER_SIZE - write_head - 1,
               ARRAY_LENGTH(debug_print_copy_buffer),
               g_debug_print_ring_buffer);
      auto end_of_first_line = strchr(debug_print_copy_buffer, '\n');
      if (end_of_first_line) {
          Text("%s", (const char*)end_of_first_line + 1);
      } else {
          Text("%s", (const char*)debug_print_copy_buffer);
      }
    }
  }
  End();
}

void inspect_enum(int* value, MemberInfo member_info) {
  int item = 0;
  int item_count = 0;
  int index_offset = -1;
  char** combo_strings = (char**)stretchy_buffer_init(&g_transient_state->allocator);
  for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_member_table); i++) {
    auto ti = TypeInfo_member_table[i];
    if (ti.member_kind == MemberKind_enum && ti.parent_type == member_info.member_type) {
      if (index_offset == -1) {
        index_offset = i;
      }
      if (type_at_offset(value, int, 0) == ti.enum_value) {
        item = i - index_offset;
      }
      stretchy_buffer_push(&g_transient_state->allocator, ti.member_name);
      item_count++;
    }
  }

  if (Combo(member_info.member_name, &item, (const char**)combo_strings, item_count)) {
    *(int*)value = TypeInfo_member_table[item + index_offset].enum_value;
  }
}

void inspect_value(MemberInfo mi, void* value, size_t offset, bool collapse = true) {
  switch (mi.member_type) {
  case TypeInfo_ID_void: Text("%s: void", mi.member_name);  break;
  case TypeInfo_ID_size_t:
  case TypeInfo_ID_u64: {
    char buffer[64];
    sprintf(buffer, "%" PRIu64, type_at_offset(value, u64, offset));
    InputText(mi.member_name, buffer, sizeof(buffer));
    u64 scanned;
    if (sscanf(buffer, "%" PRIu64, &scanned)) {
      *type_ptr_at_offset(value, u64, offset) = scanned;
    }
  } break;
  case TypeInfo_ID_u32: {
    char buffer[32];
    sprintf(buffer, "%" PRIu32, type_at_offset(value, u32, offset));
    InputText(mi.member_name, buffer, sizeof(buffer));
    u32 scanned;
    if (sscanf(buffer, "%" PRIu32, &scanned)) {
      *type_ptr_at_offset(value, u32, offset) = scanned;
    }
  } break;
  case TypeInfo_ID_u16: {
    char buffer[16];
    sprintf(buffer, "%" PRIu16, type_at_offset(value, u16, offset));
    InputText(mi.member_name, buffer, sizeof(buffer));
    u16 scanned;
    if (sscanf(buffer, "%" PRIu16, &scanned)) {
      *type_ptr_at_offset(value, u16, offset) = scanned;
    }
  } break;
  case TypeInfo_ID_u8: Text("%s: %u", mi.member_name, type_at_offset(value, u8, offset)); break;
  case TypeInfo_ID_i64: {
    char buffer[64];
    sprintf(buffer, "%" PRIi64, type_at_offset(value, i64, offset));
    InputText(mi.member_name, buffer, sizeof(buffer));
    i64 scanned;
    if (sscanf(buffer, "%" PRIi64, &scanned)) {
      *type_ptr_at_offset(value, i64, offset) = scanned;
    }
  } break;
  case TypeInfo_ID_int:
  case TypeInfo_ID_i32: InputInt(mi.member_name, type_ptr_at_offset(value, i32, offset)); break;
  case TypeInfo_ID_i16: Text("%s: %hd", mi.member_name, type_at_offset(value, i16, offset)); break;
  case TypeInfo_ID_i8: Text("%s: %d", mi.member_name, type_at_offset(value, i8, offset)); break;
  case TypeInfo_ID_b32: Checkbox(mi.member_name, type_ptr_at_offset(value, bool, offset)); break;
  case TypeInfo_ID_f32: InputFloat(mi.member_name, type_ptr_at_offset(value, f32, offset)); break;
  case TypeInfo_ID_f64: Text("%s: %lf", mi.member_name, type_at_offset(value, f64, offset)); break;
  case TypeInfo_ID_char: Text("%s: %d", mi.member_name, type_at_offset(value, char, offset)); break;
  case TypeInfo_ID_vec2: {
    vec2* v = type_ptr_at_offset(value, vec2, offset);
    PushItemWidth(GetWindowWidth() * 0.25f);
    PushID(&v->x);
    InputFloat("", &v->x);
    PopID();
    SameLine();
    PushID(&v->y);
    InputFloat("", &v->y);
    PopID();
    SameLine();
    Text("%s", mi.member_name);
    PopItemWidth();
    break;
  }
  default: {
    auto ti = get_type_info(mi.member_type);
    switch (ti.kind) {
    case TypeKind_enum: {
      inspect_enum(type_ptr_at_offset(value, i32, offset), mi);
    } break;
    case TypeKind_struct: {
      inspect_struct_(mi.member_type, void_at_offset(value, offset), mi.member_name, collapse);
    } break;
    case TypeKind_union: {
      b32 found = false;
      i32 type = type_at_offset(value, i32, offset);
      for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_union_member_table); i++) {
        auto union_info = TypeInfo_union_member_table[i];
        if (union_info.union_type_id == mi.member_type && union_info.type == type) {
          found = true;
          inspect_struct_(union_info.union_member_type_id, void_at_offset(value, offset), mi.member_name, collapse);
          break;
        }
      }

      if (!found) {
        Text("%s: ---", mi.member_name);
      }
    } break;
    default: Text("%s: ---", mi.member_name);
    }
  } break;
  }
}

size_t get_type_size(TypeInfo_ID type) {
  switch (type) {
  case TypeInfo_ID_void: return 0;
  case TypeInfo_ID_u64: return 8;
  case TypeInfo_ID_u32: return 4;
  case TypeInfo_ID_u16: return 2;
  case TypeInfo_ID_u8: return 1;
  case TypeInfo_ID_i64: return 8;
  case TypeInfo_ID_int: return 4;
  case TypeInfo_ID_i32: return 4;
  case TypeInfo_ID_i16: return 2;
  case TypeInfo_ID_i8: return 1;
  case TypeInfo_ID_b32: return 4;
  case TypeInfo_ID_f32: return 4;
  case TypeInfo_ID_f64: return 8;
  case TypeInfo_ID_char: return 1;
  case TypeInfo_ID_size_t: return sizeof(size_t);
  default: {
    if (type > TypeInfo_ID_end_primitives) {
      int type_info_index = type - TypeInfo_ID_end_primitives - 1;
      auto ti = TypeInfo_custom_type_table[type_info_index];

      return ti.size;
    }
  } break;
  }

  return 0;
}

void inspect_member(MemberInfo mi, void* value) {
  PushID(void_at_offset(value, mi.offset));
  if (mi.member_kind == MemberKind_value) {
    inspect_value(mi, value, mi.offset);
  } else if (mi.member_kind == MemberKind_array) {
    auto size = get_type_size(mi.member_type);
    if (CollapsingHeader(tprintf("%s (array)", mi.member_name))) {
      Indent();
      for (int i = 0; i < mi.array_size; ++i) {
        if (CollapsingHeader(tprintf("(%d)", i))) {
          inspect_value(mi, void_at_offset(value, mi.offset + i * size), 0, false);
        }
      }
      Unindent();
    }
  } else if (mi.member_kind == MemberKind_pointer) {
    void* ptr = type_at_offset(value, void*, mi.offset);

    if (has_flag(mi.flags, TYPEINFO_MEMBER_FLAG_CSTRING)) {
      if (ptr) {
        InputText(mi.member_name, (char*)ptr, EDITABLE_STRING_BUFFER_LENGTH);
      } else {
        if (Button("Allocate string (calloc)")) {
          *type_ptr_at_offset(value, void*, mi.offset) = calloc(EDITABLE_STRING_BUFFER_LENGTH, 1);
        }
      }
    } else if (has_flag(mi.flags, TYPEINFO_MEMBER_FLAG_ARRAY)) {
        auto size = get_type_size(mi.member_type);
        if (CollapsingHeader(tprintf("%s (array)", mi.member_name))) {
          if (ptr) {
            Indent();
            i32 count = get_array_count(mi, value);
            for (int i = 0; i < count; ++i) {
              if (CollapsingHeader(tprintf("(%d)", i))) {
                inspect_value(mi, void_at_offset(ptr, i * size), 0, false);
              }
            }
            Unindent();
          } else {
            Indent();
            Text("NULL");
            Unindent();
          }
        }
    } else {
      if (CollapsingHeader(tprintf("%s (pointer)", mi.member_name))) {
        if (ptr) {
          Indent();
          inspect_struct_(mi.member_type, ptr, mi.member_name, false);
          Unindent();
        } else {
          Indent();
          Text("NULL");
          Unindent();
        }
      }
    }
  } else {
    Text("%s: ---", mi.member_name);
  }
  PopID();
}

void debug_inspect_(TypeInfo_ID type, void* value, char* name) {
  MemberInfo dummy_member_info = {};
  dummy_member_info.member_name = name;
  dummy_member_info.member_type = type;
  inspect_value(dummy_member_info, value, 0);
}

void inspect_struct_(TypeInfo_ID type_id, void* value, char* member_name, b32 collapse) {
  PushID(value);
  if (!collapse || CollapsingHeader(member_name)) {
    if (collapse) {
      Indent();
    }
    bool member_found = false;
    for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_member_table); i++) {
      auto ti = TypeInfo_member_table[i];
      if (ti.parent_type == type_id) {
        inspect_member(ti, value);
        member_found = true;
      }
    }
    if (!member_found) {
      Text("no members");
    }
    if (collapse) {
      Unindent();
    }
  }
  PopID();
}

void* debug_serialize_struct_(TypeInfo_ID type_id, void* value) {
  stretchy_buffer_init(&g_transient_state->allocator);
  for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_member_table); i++) {
    auto ti = TypeInfo_member_table[i];
    if (ti.parent_type == type_id) {
      inspect_member(ti, value);
    }
  }

  return NULL;
}
