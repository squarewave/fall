#include "platform.h"
#include "imgui/imgui.h"
#include "debug.h"

char debug_print_copy_buffer[DEBUG_PRINT_RING_BUFFER_SIZE + 1] = {0};

void show_debug_log() {
  ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
  if (ImGui::Begin("Output")) {
    if (*g_debug_print_ring_buffer_write_head < DEBUG_PRINT_RING_BUFFER_SIZE) {
      ImGui::Text("%s", (const char*)g_debug_print_ring_buffer);
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
          ImGui::Text("%s", (const char*)end_of_first_line + 1);
      } else {
          ImGui::Text("%s", (const char*)debug_print_copy_buffer);
      }
    }
  }
  ImGui::End();
}

#define void_at_offset(value, offset) ((void*)(((char*)(value)) + (offset)))
#define type_ptr_at_offset(value, type, offset) ((type*)(((char*)(value)) + (offset)))
#define type_at_offset(value, type, offset) (*type_ptr_at_offset(value, type, offset))

void inspect_enum(int value, MemberInfo member_info) {
  for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_member_table); i++) {
    auto ti = TypeInfo_member_table[i];
    if (ti.member_kind == MemberKind_enum && ti.parent_type == member_info.member_type && ti.enum_value == value) {
      ImGui::Text("%s: %s", member_info.member_name, ti.member_name);
      break;
    }
  }
}

void inspect_value(MemberInfo mi, void* value) {
  if (mi.member_kind == MemberKind_value) {
    switch (mi.member_type) {
    case TypeInfo_ID_void: ImGui::Text("%s: void", mi.member_name);  break;
    case TypeInfo_ID_u64: ImGui::Text("%s: %llu", mi.member_name, type_at_offset(value, u64, mi.offset)); break;
    case TypeInfo_ID_u32: ImGui::Text("%s: %u", mi.member_name, type_at_offset(value, u32, mi.offset)); break;
    case TypeInfo_ID_u16: ImGui::Text("%s: %hu", mi.member_name, type_at_offset(value, u16, mi.offset)); break;
    case TypeInfo_ID_u8: ImGui::Text("%s: %u", mi.member_name, type_at_offset(value, u8, mi.offset)); break;
    case TypeInfo_ID_i64: ImGui::Text("%s: %lld", mi.member_name, type_at_offset(value, i64, mi.offset)); break;
    case TypeInfo_ID_int:
    case TypeInfo_ID_i32: ImGui::Text("%s: %d", mi.member_name, type_at_offset(value, i32, mi.offset)); break;
    case TypeInfo_ID_i16: ImGui::Text("%s: %hd", mi.member_name, type_at_offset(value, i16, mi.offset)); break;
    case TypeInfo_ID_i8: ImGui::Text("%s: %d", mi.member_name, type_at_offset(value, i8, mi.offset)); break;
    case TypeInfo_ID_b32: ImGui::Text(type_at_offset(value, b32, mi.offset) ? "%s: true" : "%s: false", mi.member_name); break;
    case TypeInfo_ID_f32: ImGui::InputFloat(mi.member_name, type_ptr_at_offset(value, f32, mi.offset), 0, 0, -1); break;
    case TypeInfo_ID_f64: ImGui::Text("%s: %lf", mi.member_name, type_at_offset(value, f64, mi.offset)); break;
    case TypeInfo_ID_char: ImGui::Text("%s: %d", mi.member_name, type_at_offset(value, char, mi.offset)); break;
    default: {
      if (mi.member_type > TypeInfo_ID_end_primitives) {
        int type_info_index = mi.member_type - TypeInfo_ID_end_primitives - 1;
        auto ti = TypeInfo_custom_type_table[type_info_index];
        switch (ti.kind) {
        case TypeKind_enum: {
          inspect_enum(type_at_offset(value, i32, mi.offset), mi);
        } break;
        case TypeKind_struct: {
            inspect_struct_(mi.member_type, void_at_offset(value, mi.offset), mi.member_name);
        } break;
        case TypeKind_union: break;
        }
      } else {
        ImGui::Text("%s: ---", mi.member_name);
      }
    } break;
    }
  } else if (mi.member_kind == MemberKind_array) {
    char buffer[64] = {};
    sprintf_s(buffer, ARRAY_LENGTH(buffer),"%s (array)", mi.member_name);
    if (ImGui::CollapsingHeader(buffer)) {
      inspect_struct_(mi.member_type, void_at_offset(value, mi.offset), mi.member_name, false);
    }
  } else if (mi.member_kind == MemberKind_pointer) {
    void* ptr = type_at_offset(value, void*, mi.offset);
    char buffer[64] = {};
    sprintf_s(buffer, ARRAY_LENGTH(buffer), "%s (pointer)", mi.member_name);
    if (ImGui::CollapsingHeader(buffer)) {
      if (ptr) {
        inspect_struct_(mi.member_type, ptr, mi.member_name, false);
      } else {
        ImGui::Indent();
        ImGui::Text("NULL");
        ImGui::Unindent();
      }
    }
  } else {
    ImGui::Text("%s: ---", mi.member_name);
  }
}

void inspect_struct_(TypeInfo_ID type_id, void* value, char* member_name, b32 collapse) {
  if (!collapse || ImGui::CollapsingHeader(member_name)) {
    ImGui::Indent();
    bool member_found = false;
    for (i32 i = 0; i < ARRAY_LENGTH(TypeInfo_member_table); i++) {
      auto ti = TypeInfo_member_table[i];
      if (ti.parent_type == type_id) {
        inspect_value(ti, value);
        member_found = true;
      }
    }
    if (!member_found) {
      ImGui::Text("no members");
    }
    ImGui::Unindent();
  }
}
