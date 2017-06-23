#ifndef DEBUG_H__
#define DEBUG_H__

#include "platform.h"
#include "typeinfo.h"

void show_debug_log();

#define inspect_struct(type, pvalue) inspect_struct_(TypeInfo_ID_##type, (void*)pvalue, (char*)#pvalue)
#define inspect_struct_named(type, pvalue, name) inspect_struct_(TypeInfo_ID_##type, (void*)pvalue, name)
#define enum_member_name(type, value) enum_member_name_(TypeInfo_ID_##type, value)

void inspect_struct_(TypeInfo_ID type_id, void* value, char* member_name, b32 collapse = true);
char* enum_member_name_(TypeInfo_ID type_id, int value);

#define debug_serialize_struct(type, pvalue) debug_serialize_struct_(TypeInfo_ID_##type, (void*)pvalue)
void* debug_serialize_struct_(TypeInfo_ID type_id, void* value);

#endif /* end of include guard: DEBUG_H__ */
