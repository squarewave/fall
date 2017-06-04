#ifndef DEBUG_H__
#define DEBUG_H__

#include "platform.h"
#include "generated_typeinfo.h"

void show_debug_log();

#define inspect_struct(type, pvalue) inspect_struct_(TypeInfo_ID_##type, (void*)pvalue, (char*)#pvalue)

void inspect_struct_(TypeInfo_ID type_id, void* value, char* member_name, b32 collapse = true);

#endif /* end of include guard: DEBUG_H__ */
