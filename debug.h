#ifndef DEBUG_H__
#define DEBUG_H__

#include "platform.h"
#include "typeinfo.h"

#define EDITABLE_STRING_BUFFER_LENGTH 128

void show_debug_log();

#define inspect_struct(type, pvalue) inspect_struct_(TypeInfo_ID_##type, (void*)pvalue, (char*)#pvalue)
#define inspect_struct_no_collapse(type, pvalue) inspect_struct_(TypeInfo_ID_##type, (void*)pvalue, (char*)#pvalue, false)
#define inspect_struct_named(type, pvalue, name) inspect_struct_(TypeInfo_ID_##type, (void*)pvalue, name)

void inspect_struct_(TypeInfo_ID type_id, void* value, char* member_name, b32 collapse = true);

#define debug_serialize_struct(type, pvalue) debug_serialize_struct_(TypeInfo_ID_##type, (void*)pvalue)
void* debug_serialize_struct_(TypeInfo_ID type_id, void* value);

#endif /* end of include guard: DEBUG_H__ */
