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

#include "core.h"

typedef enum obj_attr_type
{
    EMPTY,
    MANDATORY,
    DEFAULT,
    DEFAULT_LIMITS,
    NOT_APPLICABLE

} obj_attr_type_t;

typedef struct obj_attr
{
    obj_attr_type_t type;
    uint64          lower_limit;
    uint64          upper_limit;

} obj_attr_t;

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

typedef struct codb_entry
{
    char*       object_name;
    uint16      main_index;
    uint8       sub_index;
    char*       parameter_name;
    obj_kind_t  object_kind;
    char*       unit;
    obj_code_t  object_code;
    obj_attr_t  object_code_attr;
    data_type_t data_type;
    obj_attr_t  data_type_attr;
    acc_type_t  access_type;
    obj_attr_t  access_type_attr;
    uint8       min_elements;
    obj_attr_t  min_elements_attr;
    uint8       max_elements;
    obj_attr_t  max_elements_attr;
    uint64      low_limit;
    obj_attr_t  low_limit_attr;
    uint64      high_limit;
    obj_attr_t  high_limit_attr;
    uint64      default_value;
    obj_attr_t  default_value_attr;
    bool_t      mappable;
    obj_attr_t  mappable_attr;

} codb_entry_t;

typedef struct codb_database
{
    codb_entry_t* entries;
    size_t        total_entries;

} codb_database_t;

void list_codb(void);

#endif /* CODB_H */
