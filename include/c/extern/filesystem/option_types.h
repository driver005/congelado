/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef CONGELADO_C_FILESYSTEM_OPTION_TYPES_H_
#define CONGELADO_C_FILESYSTEM_OPTION_TYPES_H_

#include <stdint.h>

// The named union is needed here (as opposed to
// inside the `TF_Filesystem_Option_Value` struct)
// as MSVC does not recognize `typeof`.
typedef union TF_Filesystem_Option_Value_Union {
  int64_t int_val;              // Integer value
  double real_val;              // Floating-point value
  struct {
    char* buf;                  // Buffer containing the value
    int buf_length;             // Length of the buffer
  } buffer_val;                 // Buffer value
} TF_Filesystem_Option_Value_Union;

typedef struct TF_Filesystem_Option_Value {
  int type_tag;                 // Type of values in the values union (TF_Filesystem_Option_Type)
  int num_values;               // Number of values in the array
  TF_Filesystem_Option_Value_Union*
      values;                   // Array of values (owned by plugin)
} TF_Filesystem_Option_Value;

typedef enum TF_Filesystem_Option_Type {
  TF_Filesystem_Option_Type_Int = 0,
  TF_Filesystem_Option_Type_Real,
  TF_Filesystem_Option_Type_Buffer,
  TF_Filesystem_Num_Option_Types,  // must always be the last item
} TF_Filesystem_Option_Type;

typedef struct TF_Filesystem_Option {
  char* name;                         // Option name (null-terminated, owned)
  char* description;                  // Human-readable description (null-terminated, owned)
  int per_file;                       // Whether option applies per-file (bool)
  TF_Filesystem_Option_Value* value;  // Option value (owned)
} TF_Filesystem_Option;

#endif  // CONGELADO_C_FILESYSTEM_OPTION_TYPES_H_
