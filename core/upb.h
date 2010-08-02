/*
 * upb - a minimalist implementation of protocol buffers.
 *
 * Copyright (c) 2009 Joshua Haberman.  See LICENSE for details.
 *
 * This file contains shared definitions that are widely used across upb.
 */

#ifndef UPB_H_
#define UPB_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // only for size_t.
#include "descriptor_const.h"
#include "upb_atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

// inline if possible, emit standalone code if required.
#ifndef INLINE
#define INLINE static inline
#endif

#define UPB_MAX(x, y) ((x) > (y) ? (x) : (y))
#define UPB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define UPB_INDEX(base, i, m) (void*)((char*)(base) + ((i)*(m)))

// The maximum that any submessages can be nested.  Matches proto2's limit.
#define UPB_MAX_NESTING 64

// The maximum number of fields that any one .proto type can have.  Note that
// this is very different than the max field number.  It is hard to imagine a
// scenario where more than 32k fields makes sense.
#define UPB_MAX_FIELDS (1<<15)
typedef int16_t upb_field_count_t;

// Nested type names are separated by periods.
#define UPB_SYMBOL_SEPARATOR '.'

// This limit is for the longest fully-qualified symbol, eg. foo.bar.MsgType
#define UPB_SYMBOL_MAXLEN 128

// The longest chain that mutually-recursive types are allowed to form.  For
// example, this is a type cycle of length 2:
//   message A {
//     B b = 1;
//   }
//   message B {
//     A a = 1;
//   }
#define UPB_MAX_TYPE_CYCLE_LEN 16

// The maximum depth that the type graph can have.  Note that this setting does
// not automatically constrain UPB_MAX_NESTING, because type cycles allow for
// unlimited nesting if we do not limit it.
#define UPB_MAX_TYPE_DEPTH 64

// The biggest possible single value is a 10-byte varint.
#define UPB_MAX_ENCODED_SIZE 10


/* Fundamental types and type constants. **************************************/

// A list of types as they are encoded on-the-wire.
enum upb_wire_type {
  UPB_WIRE_TYPE_VARINT      = 0,
  UPB_WIRE_TYPE_64BIT       = 1,
  UPB_WIRE_TYPE_DELIMITED   = 2,
  UPB_WIRE_TYPE_START_GROUP = 3,
  UPB_WIRE_TYPE_END_GROUP   = 4,
  UPB_WIRE_TYPE_32BIT       = 5,

  // This isn't a real wire type, but we use this constant to describe varints
  // that are expected to be a maximum of 32 bits.
  UPB_WIRE_TYPE_32BIT_VARINT = 8
};

typedef uint8_t upb_wire_type_t;

// Type of a field as defined in a .proto file.  eg. string, int32, etc.  The
// integers that represent this are defined by descriptor.proto.  Note that
// descriptor.proto reserves "0" for errors, and we use it to represent
// exceptional circumstances.
typedef uint8_t upb_fieldtype_t;

// For referencing the type constants tersely.
#define UPB_TYPE(type) GOOGLE_PROTOBUF_FIELDDESCRIPTORPROTO_TYPE_ ## type
#define UPB_LABEL(type) GOOGLE_PROTOBUF_FIELDDESCRIPTORPROTO_LABEL_ ## type

// Info for a given field type.
typedef struct {
  uint8_t align;
  uint8_t size;
  upb_wire_type_t native_wire_type;
  uint8_t allowed_wire_types;  // For packable fields, also allows delimited.
  char *ctype;
} upb_type_info;

// A static array of info about all of the field types, indexed by type number.
extern upb_type_info upb_types[];

// The number of a field, eg. "optional string foo = 3".
typedef int32_t upb_field_number_t;

// Label (optional, repeated, required) as defined in a .proto file.  The
// values of this are defined by google.protobuf.FieldDescriptorProto.Label
// (from descriptor.proto).
typedef uint8_t  upb_label_t;

// A scalar (non-string) wire value.  Used only for parsing unknown fields.
typedef union {
  uint64_t varint;
  uint64_t _64bit;
  uint32_t _32bit;
} upb_wire_value;

/* Polymorphic values of .proto types *****************************************/

struct _upb_string;
typedef struct _upb_string upb_string;
struct _upb_array;
typedef struct _upb_array upb_array;
struct _upb_msg;
typedef struct _upb_msg upb_msg;

typedef uint32_t upb_strlen_t;

// A single .proto value.  The owner must have an out-of-band way of knowing
// the type, so that it knows which union member to use.
typedef union {
  double _double;
  float _float;
  int32_t int32;
  int64_t int64;
  uint32_t uint32;
  uint64_t uint64;
  bool _bool;
  upb_string *str;
  upb_msg *msg;
  upb_array *arr;
  upb_atomic_refcount_t *refcount;
  void *_void;
} upb_value;

// A pointer to a .proto value.  The owner must have an out-of-band way of
// knowing the type, so it knows which union member to use.
typedef union {
  double *_double;
  float *_float;
  int32_t *int32;
  int64_t *int64;
  uint8_t *uint8;
  uint32_t *uint32;
  uint64_t *uint64;
  bool *_bool;
  upb_string **str;
  upb_msg **msg;
  upb_array **arr;
  void *_void;
} upb_valueptr;

// The type of a upb_value.  This is like a upb_fieldtype_t, but adds the
// constant UPB_VALUETYPE_ARRAY to represent an array.
typedef uint8_t upb_valuetype_t;
#define UPB_VALUETYPE_ARRAY 32

INLINE upb_valueptr upb_value_addrof(upb_value *val) {
  upb_valueptr ptr = {&val->_double};
  return ptr;
}

// Converts upb_value_ptr -> upb_value by reading from the pointer.  We need to
// know the value type to perform this operation, because we need to know how
// much memory to copy.
INLINE upb_value upb_value_read(upb_valueptr ptr, upb_fieldtype_t ft) {
  upb_value val;

#define CASE(t, member_name) \
  case UPB_TYPE(t): val.member_name = *ptr.member_name; break;

  switch(ft) {
    CASE(DOUBLE,   _double)
    CASE(FLOAT,    _float)
    CASE(INT32,    int32)
    CASE(INT64,    int64)
    CASE(UINT32,   uint32)
    CASE(UINT64,   uint64)
    CASE(SINT32,   int32)
    CASE(SINT64,   int64)
    CASE(FIXED32,  uint32)
    CASE(FIXED64,  uint64)
    CASE(SFIXED32, int32)
    CASE(SFIXED64, int64)
    CASE(BOOL,     _bool)
    CASE(ENUM,     int32)
    CASE(STRING,   str)
    CASE(BYTES,    str)
    CASE(MESSAGE,  msg)
    CASE(GROUP,    msg)
    default: break;
  }
  return val;

#undef CASE
}

// Writes a upb_value to a upb_value_ptr location. We need to know the value
// type to perform this operation, because we need to know how much memory to
// copy.
INLINE void upb_value_write(upb_valueptr ptr, upb_value val,
                            upb_fieldtype_t ft) {
#define CASE(t, member_name) \
  case UPB_TYPE(t): *ptr.member_name = val.member_name; break;

  switch(ft) {
    CASE(DOUBLE,   _double)
    CASE(FLOAT,    _float)
    CASE(INT32,    int32)
    CASE(INT64,    int64)
    CASE(UINT32,   uint32)
    CASE(UINT64,   uint64)
    CASE(SINT32,   int32)
    CASE(SINT64,   int64)
    CASE(FIXED32,  uint32)
    CASE(FIXED64,  uint64)
    CASE(SFIXED32, int32)
    CASE(SFIXED64, int64)
    CASE(BOOL,     _bool)
    CASE(ENUM,     int32)
    CASE(STRING,   str)
    CASE(BYTES,    str)
    CASE(MESSAGE,  msg)
    CASE(GROUP,    msg)
    default: break;
  }

#undef CASE
}

// Status codes used as a return value.  Codes >0 are not fatal and can be
// resumed.
enum upb_status_code {
  UPB_STATUS_OK = 0,

  // A read or write from a streaming src/sink could not be completed right now.
  UPB_STATUS_TRYAGAIN = 1,

  // A value had an incorrect wire type and will be skipped.
  UPB_STATUS_BADWIRETYPE = 2,

  // An unrecoverable error occurred.
  UPB_STATUS_ERROR = -1,

  // A varint went for 10 bytes without terminating.
  UPB_ERROR_UNTERMINATED_VARINT = -2,

  // The max nesting level (UPB_MAX_NESTING) was exceeded.
  UPB_ERROR_MAX_NESTING_EXCEEDED = -3
};

typedef struct {
  enum upb_status_code code;
  upb_string *str;
} upb_status;

#define UPB_STATUS_INIT {UPB_STATUS_OK, NULL}
#define UPB_ERRORMSG_MAXLEN 256

INLINE bool upb_ok(upb_status *status) {
  return status->code == UPB_STATUS_OK;
}

INLINE void upb_status_init(upb_status *status) {
  status->code = UPB_STATUS_OK;
  status->str = NULL;
}

void upb_printerr(upb_status *status);
void upb_clearerr(upb_status *status);
void upb_seterr(upb_status *status, enum upb_status_code code, const char *msg,
                ...);
void upb_copyerr(upb_status *to, upb_status *from);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* UPB_H_ */
