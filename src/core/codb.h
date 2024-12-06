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
    DOMAIN_T,

} data_type_t;

typedef enum acc_type
{
    UNSPECIFIED,
    CONST_T,
    RO,
    WO,
    RW,
    WWR,
    RWW,

} acc_type_t;

typedef cJSON codb_t;

void        codb_init(void);
void        codb_deinit(void);
const char* codb_desc_lookup(codb_t* db, uint16 index, uint8 sub_index);
codb_t*     codb_get_ds301_profile(void);
codb_t*     codb_get_profile(void);
bool_t      is_ds301_loaded(void);
bool_t      is_codb_loaded(void);
void        list_codb(void);
status_t    load_codb(uint32 file_no);
status_t    load_codb_ex(const char* file_name);
void        unload_codb(void);

#endif /* CODB_H */
