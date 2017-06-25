#pragma once

#include "typeinfo.h"

void* serialize_struct_array(TypeInfo_ID type_id, void* data, i32 count, size_t* out_size);
void deserialize_struct_array(TypeInfo_ID type_id, void* data, size_t data_size,
                              void* buffer, size_t buffer_size, i32* out_count);