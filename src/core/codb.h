/** @file codb.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CODB_H
#define CODB_H

#include "cJSON.h"
#include "os.h"

#define CODB_MAX_DESC_LEN 256

extern const char* data_type_lookup[];
extern const char* object_code_lookup[];
extern const char* object_kind_lookup[];
extern const char* access_type_lookup[];

typedef enum obj_attr_type
{
    EMPTY,
    MANDATORY,
    DEFAULT,
    DEFAULT_LIMITS,
    NOT_APPLICABLE

} obj_attr_type_t;

typedef enum obj_kind
{
    IS_OPTIONAL,
    IS_MANDATORY,
    IS_CONDITIONAL

} obj_kind_t;

typedef enum obj_code
{
    IS_DOMAIN,
    IS_DEFTYPE,
    IS_DEFSTRUCT,
    IS_VAR,
    IS_ARRAY,
    IS_RECORD

} obj_code_t;

typedef enum data_type
{
    NONE_T,
    BOOLEAN_T,
    INTEGER8,
    UNSIGNED8,
    INTEGER16,
    UNSIGNED16,
    INTEGER24,
    UNSIGNED24,
    INTEGER32,
    UNSIGNED32,
    INTEGER48,
    UNSIGNED48,
    INTEGER56,
    UNSIGNED56,
    INTEGER64,
    UNSIGNED64,
    REAL32,
    REAL64,
    FLOAT_T,
    TIME_OF_DAY,
    VISIBLE_STRING,
    OCTET_STRING,
    DOMAIN_T

} data_type_t;

typedef enum acc_type
{
    UNSPECIFIED,
    CONST_T,
    RO,
    WO,
    RW,
    WWR,
    RWW

} acc_type_t;

typedef struct object_info
{
    int             entry_count;
    uint16          index;
    uint8           sub_index;
    obj_code_t      code;
    data_type_t     data_type;
    obj_kind_t      category;
    obj_kind_t      entry_category;
    obj_attr_type_t attribute;
    acc_type_t      access_type;
    bool_t          pdo_mapping;
    bool_t          does_exist;
    uint32          value_range_lower;
    uint32          value_range_upper;
    uint32          default_value;
    char            name[CODB_MAX_DESC_LEN];
    char            sub_index_name[CODB_MAX_DESC_LEN];

} object_info_t;

typedef cJSON codb_t;

void        codb_init(void);
void        codb_deinit(void);
const char* codb_desc_lookup(codb_t* db, uint16 index, uint8 sub_index);
const char* codb_desc_lookup_ex(codb_t* db, uint16 index, uint8 sub_index, char* object_desc, char* sub_index_desc);
void        codb_info_lookup(codb_t* db, uint16 index, uint8 sub_index, object_info_t* info);
codb_t*     codb_get_ds301_profile(void);
codb_t*     codb_get_profile(void);
bool_t      is_ds301_loaded(void);
bool_t      is_codb_loaded(void);
void        list_codb(void);
status_t    load_codb(uint32 file_no);
status_t    load_codb_ex(char* file_name);
void        unload_codb(void);

#endif /* CODB_H */
