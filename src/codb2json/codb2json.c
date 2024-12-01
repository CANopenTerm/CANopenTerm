/** @file codb2json.c
 *
 *  CANopen Profile data base to JSON converter.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>
#include "codb.h"
#include "os.h"
#include "cJSON.h"

/* codb file format:
 *
 * Empty lines are ignored.
 *
 * # at the beginning of a line indicates a comment.
 *
 * The fields are separated by colons.
 *
 * Field description:
 *
 * 1.  Main index, four digits hex value
 * 2.  Sub index, two digits hex value
 * 3.  Parameter name, string literal
 * 4.  Object kind, mandatory or optional
 * 5.  Unit, string literal
 * 6.  Object code, one of the following: DOMAIN, DEFTYPE, DEFSTRUCT, VAR, ARRAY, RECORD
 * 7.  Object code attribute, see attribute description below
 * 8.  Data type, one of the following: BOOLEAN, INTEGER8, UNSIGNED8, INTEGER16, UNSIGNED16
 *     INTEGER24, UNSIGNED24, INTEGER32, UNSIGNED32, INTEGER48, UNSIGNED48, INTEGER56, UNSIGNED56
 *     INTEGER64, UNSIGNED64, REAL32, REAL64, TIME_OF_DAY, VISIBLE_STRING, OCTET_STRING, DOMAIN,
 *     FLOAT
 * 9.  Data type attribute, see attribute description below
 * 10. Access type, one of the following: const, ro, wo, rw, wwr, rww
 * 11. Access type attribute, see attribute description below
 * 12. Minimum elements, 0..255
 * 13. Minimum elements attribute, see attribute description below
 * 14. Maximum elements, 0..255
 * 15. Maximum elements attribute, see attribute description below
 * 16. Low limit, decimal value or 0x followed by hex value
 * 17. Low limit attribute, see attribute description below
 * 18. High limit, decimal value or 0x followed by hex value
 * 19. High limit attribute, see attribute description below
 * 20. Default value, decimal value or 0x followed by hex value or "$NODEID"+ 0x followed by hex value
 * 21. Default value attribute, see attribute description below
 * 22. Mappable, y or n
 * 23. Mappable attribute, see attribute description below
 *
 * Attribute description:
 * [empty]   - data value, if exists, is a default and can be changed in the range specified by the data type
 * m         - mandatory, data value is mandatory and cannot be changed
 * d         - data value is a default and can be changed in the range specified by the data type
 * d[limits] - data value is a default and can be changed within the limits
 * n         - not applicable for the preceding data field, data field shall be empty
 *
 * A object can be duplicated using the following syntax:
 *
 * $1A01=1A00
 *
 * This creates a new entry 1A01 with the same values as 1A00.
 *
 * Sample lines:
 *
 * 1003::Predefined Error Field:optional::ARRAY:m::m::m:2:m:255:m::m::m::m::m
 * 1003:00:Number of Errors:optional::VAR:m:UNSIGNED8:m:rw:d[rw,ro]::n::n:::::0:m:n:m
 * 1003:01:Standard Error Field:optional::VAR:m:UNSIGNED32:m:ro:m::n::n::::d:0:m:n:m
 * 1005::COB ID SYNC:optional::VAR:m:UNSIGNED32:m:rw:d::n::n:0x00000001:d[,1...0xFFFFFFFF]:::0x80000080:d[0x80,0x80000080]:n:m
 * 1006::Communication Cycle Period:optional:µs:VAR:m:UNSIGNED32:m:rw:m::n::n:::::0x00000000:m:n:m
 * 1014::COB ID EMCY:optional::VAR:m:UNSIGNED32:m:ro:d[ro,rw]::n::n:0x00000001:d[,1...0xFFFFFFFF]:0xFFFFFFFF:d[,1...0xFFFFFFFF]:$NODEID+0x80:d[$NODEID+0x80000080,$NODEID+0x80]:n:m
 *
 */

#define BUFFER_SIZE 2048

typedef struct obj_attr
{
    obj_attr_type_t type;
    uint64          lower_limit;
    uint64          upper_limit;

} obj_attr_t;

typedef struct codb_entry
{
    char* object_name;
    uint16      main_index;
    uint8       sub_index;
    char* parameter_name;
    obj_kind_t  object_kind;
    char* unit;
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

typedef enum field_id
{
    MAIN_INDEX,
    SUB_INDEX,
    PARAMETER_NAME,
    OBJECT_KIND,
    UNIT,
    OBJECT_CODE,
    OBJECT_CODE_ATTR,
    DATA_TYPE,
    DATA_TYPE_ATTR,
    ACCESS_TYPE,
    ACCESS_TYPE_ATTR,
    MIN_ELEMENTS,
    MIN_ELEMENTS_ATTR,
    MAX_ELEMENTS,
    MAX_ELEMENTS_ATTR,
    LOW_LIMIT,
    LOW_LIMIT_ATTR,
    HIGH_LIMIT,
    HIGH_LIMIT_ATTR,
    DEFAULT_VALUE,
    DEFAULT_VALUE_ATTR,
    MAPPABLE,
    MAPPABLE_ATTR

} field_id_t;

static codb_database_t* codb_db;
static cJSON*           min_elements;

static bool_t add_sub_index_to_object(cJSON* sub_indices, size_t i);
static void   init_codb_entry(codb_entry_t* entry);
static bool_t is_codb_file(const char* input_file_name);
static void   read_codb(const char* input_file_name);
static void   write_json(const char* output_file_name);
static bool_t write_json_entry(cJSON* entry, size_t i);
static void   free_codb_database(codb_database_t* db);
static void   handle_attribute(obj_attr_t* attr, const char* token);
static void   handle_value8(uint8*  value, const char* token);
static void   handle_value64(uint64* value, const char* token);
static char*  to_upper_case(const char* str);

int codb2json(int argc, char* argv[])
{
    char  json_file_name[256] = { 0 };
    char* dot;

    if (argc != 2)
    {
        os_fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (IS_FALSE == is_codb_file(argv[1]))
    {
        os_fprintf(stderr, "Error: %s is not a .codb file.\n", argv[1]);
        return EXIT_FAILURE;
    }

    codb_db = (codb_database_t*)os_calloc(1, sizeof(codb_database_t));
    if (codb_db == NULL)
    {
        os_fprintf(stderr, "Error allocating memory for codb_db");
        return EXIT_FAILURE;
    }
    codb_db->entries       = NULL;
    codb_db->total_entries = 0;

    os_strlcpy(json_file_name, argv[1], sizeof(json_file_name) - 1);
    json_file_name[sizeof(json_file_name) - 1] = '\0';
    dot = os_strrchr(json_file_name, '.');
    if (dot != NULL)
    {
        *dot = '\0';
    }
    os_strlcat(json_file_name, ".json", sizeof(json_file_name));

    read_codb(argv[1]);
    write_json(json_file_name);

    free_codb_database(codb_db);

    return EXIT_SUCCESS;
}

static bool_t add_sub_index_to_object(cJSON *sub_indices, size_t i)
{
    cJSON* sub_index;
    cJSON* sub_index_value;
    cJSON* parameter_name;
    cJSON* object_kind;
    cJSON* unit;

    cJSON* data_type;
    cJSON* data_type_type;
    cJSON* data_type_attr;
    cJSON* data_type_lower_limit;
    cJSON* data_type_upper_limit;

    cJSON* access_type;
    cJSON* access_type_type;
    cJSON* access_type_attr;
    cJSON* access_type_lower_limit;
    cJSON* access_type_upper_limit;

    cJSON* low_limit;
    cJSON* low_limit_value;
    cJSON* low_limit_attr;
    cJSON* low_limit_lower_limit;
    cJSON* low_limit_upper_limit;

    cJSON* high_limit;
    cJSON* high_limit_value;
    cJSON* high_limit_attr;
    cJSON* high_limit_lower_limit;
    cJSON* high_limit_upper_limit;

    cJSON* default_value;
    cJSON* default_value_value;
    cJSON* default_value_attr;
    cJSON* default_value_lower_limit;
    cJSON* default_value_upper_limit;

    cJSON* mappable;
    cJSON* mappable_value;
    cJSON* mappable_attr;
    cJSON* mappable_lower_limit;
    cJSON* mappable_upper_limit;

    sub_index = cJSON_CreateObject();
    if (sub_index == NULL)
    {
        return IS_FALSE;
    }

    sub_index_value = cJSON_CreateNumber(codb_db->entries[i].sub_index);
    parameter_name  = cJSON_CreateString(codb_db->entries[i].parameter_name);
    object_kind     = cJSON_CreateNumber(codb_db->entries[i].object_kind);
    unit            = cJSON_CreateString(codb_db->entries[i].unit);

    data_type             = cJSON_CreateObject();
    data_type_type        = cJSON_CreateNumber(codb_db->entries[i].data_type);
    data_type_attr        = cJSON_CreateNumber(codb_db->entries[i].data_type_attr.type);
    data_type_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].data_type_attr.lower_limit);
    data_type_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].data_type_attr.upper_limit);
    cJSON_AddItemToObject(data_type, "type",  data_type_type);
    cJSON_AddItemToObject(data_type, "attr",  data_type_attr);
    cJSON_AddItemToObject(data_type, "lower", data_type_lower_limit);
    cJSON_AddItemToObject(data_type, "upper", data_type_upper_limit);

    access_type             = cJSON_CreateObject();
    access_type_type        = cJSON_CreateNumber(codb_db->entries[i].access_type);
    access_type_attr        = cJSON_CreateNumber(codb_db->entries[i].access_type_attr.type);
    access_type_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].access_type_attr.lower_limit);
    access_type_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].access_type_attr.upper_limit);
    cJSON_AddItemToObject(access_type, "type",  access_type_type);
    cJSON_AddItemToObject(access_type, "attr",  access_type_attr);
    cJSON_AddItemToObject(access_type, "lower", access_type_lower_limit);
    cJSON_AddItemToObject(access_type, "upper", access_type_upper_limit);

    low_limit             = cJSON_CreateObject();
    low_limit_value       = cJSON_CreateNumber(codb_db->entries[i].low_limit);
    low_limit_attr        = cJSON_CreateNumber(codb_db->entries[i].low_limit_attr.type);
    low_limit_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].low_limit_attr.lower_limit);
    low_limit_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].low_limit_attr.upper_limit);
    cJSON_AddItemToObject(low_limit, "value", low_limit_value);
    cJSON_AddItemToObject(low_limit, "attr",  low_limit_attr);
    cJSON_AddItemToObject(low_limit, "lower", low_limit_lower_limit);
    cJSON_AddItemToObject(low_limit, "upper", low_limit_upper_limit);

    high_limit             = cJSON_CreateObject();
    high_limit_value       = cJSON_CreateNumber(codb_db->entries[i].high_limit);
    high_limit_attr        = cJSON_CreateNumber(codb_db->entries[i].high_limit_attr.type);
    high_limit_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].high_limit_attr.lower_limit);
    high_limit_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].high_limit_attr.upper_limit);
    cJSON_AddItemToObject(high_limit, "value", high_limit_value);
    cJSON_AddItemToObject(high_limit, "attr",  high_limit_attr);
    cJSON_AddItemToObject(high_limit, "lower", high_limit_lower_limit);
    cJSON_AddItemToObject(high_limit, "upper", high_limit_upper_limit);

    default_value             = cJSON_CreateObject();
    default_value_value       = cJSON_CreateNumber(codb_db->entries[i].default_value);
    default_value_attr        = cJSON_CreateNumber(codb_db->entries[i].default_value_attr.type);
    default_value_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].default_value_attr.lower_limit);
    default_value_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].default_value_attr.upper_limit);
    cJSON_AddItemToObject(default_value, "value", default_value_value);
    cJSON_AddItemToObject(default_value, "attr",  default_value_attr);
    cJSON_AddItemToObject(default_value, "lower", default_value_lower_limit);
    cJSON_AddItemToObject(default_value, "upper", default_value_upper_limit);

    mappable             = cJSON_CreateObject();
    mappable_value       = cJSON_CreateBool(codb_db->entries[i].mappable);
    mappable_attr        = cJSON_CreateNumber(codb_db->entries[i].mappable_attr.type);
    mappable_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].mappable_attr.lower_limit);
    mappable_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].mappable_attr.upper_limit);
    cJSON_AddItemToObject(mappable, "value", mappable_value);
    cJSON_AddItemToObject(mappable, "attr",  mappable_attr);
    cJSON_AddItemToObject(mappable, "lower", mappable_lower_limit);
    cJSON_AddItemToObject(mappable, "upper", mappable_upper_limit);

    cJSON_AddItemToObject(sub_index, "index",         sub_index_value);
    cJSON_AddItemToObject(sub_index, "desc",          parameter_name);
    cJSON_AddItemToObject(sub_index, "kind",          object_kind);
    cJSON_AddItemToObject(sub_index, "unit",          unit);
    cJSON_AddItemToObject(sub_index, "data_type",     data_type);
    cJSON_AddItemToObject(sub_index, "access_type",   access_type);
    cJSON_AddItemToObject(sub_index, "low_limit",     low_limit);
    cJSON_AddItemToObject(sub_index, "high_limit",    high_limit);
    cJSON_AddItemToObject(sub_index, "default_value", default_value);
    cJSON_AddItemToObject(sub_index, "mappable",      mappable);

    cJSON_AddItemToArray(sub_indices, sub_index);

    return IS_TRUE;
}

static void init_codb_entry(codb_entry_t* entry)
{
    if (NULL == entry)
    {
        return;
    }

    entry->object_name                    = NULL;
    entry->main_index                     = 0;
    entry->sub_index                      = 0;
    entry->parameter_name                 = NULL;
    entry->object_kind                    = IS_OPTIONAL;
    entry->unit                           = NULL;
    entry->object_code                    = IS_DOMAIN;
    entry->object_code_attr.type          = EMPTY;
    entry->object_code_attr.lower_limit   = 0;
    entry->object_code_attr.upper_limit   = 0;
    entry->data_type                      = NONE_T;
    entry->data_type_attr.type            = EMPTY;
    entry->data_type_attr.lower_limit     = 0;
    entry->data_type_attr.upper_limit     = 0;
    entry->access_type                    = UNSPECIFIED;
    entry->access_type_attr.type          = EMPTY;
    entry->access_type_attr.lower_limit   = 0;
    entry->access_type_attr.upper_limit   = 0;
    entry->min_elements                   = 0;
    entry->min_elements_attr.type         = EMPTY;
    entry->min_elements_attr.lower_limit  = 0;
    entry->min_elements_attr.upper_limit  = 0;
    entry->max_elements                   = 0;
    entry->max_elements_attr.type         = EMPTY;
    entry->max_elements_attr.lower_limit  = 0;
    entry->max_elements_attr.upper_limit  = 0;
    entry->low_limit                      = 0;
    entry->low_limit_attr.type            = EMPTY;
    entry->low_limit_attr.lower_limit     = 0;
    entry->low_limit_attr.upper_limit     = 0;
    entry->high_limit                     = 0;
    entry->high_limit_attr.type           = EMPTY;
    entry->high_limit_attr.lower_limit    = 0;
    entry->high_limit_attr.upper_limit    = 0;
    entry->default_value                  = 0;
    entry->default_value_attr.type        = EMPTY;
    entry->default_value_attr.lower_limit = 0;
    entry->default_value_attr.upper_limit = 0;
    entry->mappable                       = IS_FALSE;
    entry->mappable_attr.type             = EMPTY;
    entry->mappable_attr.lower_limit      = 0;
    entry->mappable_attr.upper_limit      = 0;
}

static bool_t is_codb_file(const char* input_file_name)
{
    char line[BUFFER_SIZE] = { 0 };
    FILE* input_file       = os_fopen(input_file_name, "rb");

    if (input_file == NULL)
    {
        os_fprintf(stderr, "Error opening input file.");
        return IS_FALSE;
    }

    while (os_fgets(line, sizeof(line), input_file) != NULL)
    {
        if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r' || isxdigit(line[0]) || line[0] == '#' || line[0] == '$')
        {
            continue;
        }
        else
        {
            os_fclose(input_file);
            return IS_FALSE;
        }
    }

    os_fclose(input_file);
    return IS_TRUE;
}


static void read_codb(const char* input_file_name)
{
    FILE*  input_file    = os_fopen(input_file_name, "rb");
    char*  line          = NULL;
    size_t entry_count   = 0;
    size_t line_count    = 0;
    size_t current_entry;
    size_t i;

    if (input_file == NULL)
    {
        os_fprintf(stderr, "Error opening input file.");
        os_exit(EXIT_FAILURE);
    }

    line = (char*)os_calloc(BUFFER_SIZE, sizeof(char));
    if (line == NULL)
    {
        os_fprintf(stderr, "Error allocating memory for line buffer.");
        os_fclose(input_file);
        os_exit(EXIT_FAILURE);
    }

    /* First pass: count the number of entries. */
    while (os_fgets(line, BUFFER_SIZE, input_file) != NULL)
    {
        if ('$' == line[0] ||  '#' == line[0] || '\n' == line[0] || '\r' == line[0] || '\0' == line[0])
        {
            continue;
        }
        entry_count++;
    }

    /* Allocate memory for all entries in advance. */
    codb_db->entries = (codb_entry_t*)os_calloc(entry_count, sizeof(codb_entry_t));
    if (codb_db->entries == NULL)
    {
        os_fprintf(stderr, "Error allocating memory for entries.");
        os_free(line);
        os_fclose(input_file);
        os_exit(EXIT_FAILURE);
    }
    codb_db->total_entries = entry_count;

    /* Initialize all entries to default values. */
    for (i = 0; i < entry_count; i++)
    {
        init_codb_entry(&codb_db->entries[i]);
    }

    /* Reset file pointer to the beginning */
    os_rewind(input_file);

    current_entry = 0;
    while (os_fgets(line, BUFFER_SIZE, input_file) != NULL)
    {
        field_id_t id;
        char*      token = line;

        line_count++;

        /* Check if the line is too long. */
        if (os_strchr(line, '\n') == NULL && !feof(input_file))
        {
            os_fprintf(stderr, "Error: Line too long in input file.\n");
            os_free(line);
            os_fclose(input_file);
            os_free(codb_db->entries);
            os_exit(EXIT_FAILURE);
        }

        /* Copy object. */
        if (line[0] == '$')
        {
            char*  end;
            uint16 source_index;
            uint16 dest_index;
            size_t source_count = 0;

            end = os_strchr(token, '=');
            if (end == NULL)
            {
                os_fprintf(stderr, "L%zd Error: Invalid object copy syntax.\n", line_count);
                os_free(line);
                os_fclose(input_file);
                os_free(codb_db->entries);
                os_exit(EXIT_FAILURE);
            }
            else
            {
                *end         = '\0';
                source_index = (uint16)os_strtol(end   + 1, NULL, 16);
                dest_index   = (uint16)os_strtol(token + 1, NULL, 16);
            }

            if (source_index == dest_index)
            {
                os_fprintf(stderr, "L%zd Error: Source and destination index are the same.\n", line_count);
                os_free(line);
                os_fclose(input_file);
                os_free(codb_db->entries);
                os_exit(EXIT_FAILURE);
            }

            for (i = 0; i < codb_db->total_entries; i++)
            {
                if (codb_db->entries[i].main_index == source_index)
                {
                    source_count++;
                }
            }

            if (0 == source_count)
            {
                os_fprintf(stderr, "L%zd Error: Source index not found.\n", line_count);
                os_free(line);
                os_fclose(input_file);
                os_free(codb_db->entries);
                os_exit(EXIT_FAILURE);
            }
            else
            {
                codb_db->entries = (codb_entry_t*)os_realloc(codb_db->entries, (codb_db->total_entries + source_count) * sizeof(codb_entry_t));
                if (NULL == codb_db->entries)
                {
                    os_fprintf(stderr, "Error reallocating memory for entries.");
                    os_free(line);
                    os_fclose(input_file);
                    os_exit(EXIT_FAILURE);
                }
                else
                {
                    for (i = codb_db->total_entries; i < codb_db->total_entries + source_count; i++)
                    {
                        init_codb_entry(&codb_db->entries[i]);
                    }
                }
            }

            for (i = 0; i < codb_db->total_entries; i++)
            {
                if (codb_db->entries[i].main_index == source_index)
                {
                    if (codb_db->entries[i].object_name != NULL)
                    {
                        char object_name_str[5] = { 0 };
                        os_snprintf(object_name_str, sizeof(object_name_str), "%04X", dest_index);
                        codb_db->entries[current_entry].object_name = os_strdup(object_name_str);
                    }

                    codb_db->entries[current_entry].main_index     = dest_index;
                    codb_db->entries[current_entry].sub_index      = codb_db->entries[i].sub_index;
                    codb_db->entries[current_entry].object_kind    = codb_db->entries[i].object_kind;
                    codb_db->entries[current_entry].object_code    = codb_db->entries[i].object_code;
                    codb_db->entries[current_entry].data_type      = codb_db->entries[i].data_type;
                    codb_db->entries[current_entry].access_type    = codb_db->entries[i].access_type;
                    codb_db->entries[current_entry].min_elements   = codb_db->entries[i].min_elements;
                    codb_db->entries[current_entry].max_elements   = codb_db->entries[i].max_elements;
                    codb_db->entries[current_entry].low_limit      = codb_db->entries[i].low_limit;
                    codb_db->entries[current_entry].high_limit     = codb_db->entries[i].high_limit;
                    codb_db->entries[current_entry].default_value  = codb_db->entries[i].default_value;
                    codb_db->entries[current_entry].mappable       = codb_db->entries[i].mappable;

                    if (NULL != codb_db->entries[i].parameter_name)
                    {
                        codb_db->entries[current_entry].parameter_name = os_strdup(codb_db->entries[i].parameter_name);
                    }
                    else
                    {
                        codb_db->entries[current_entry].parameter_name = NULL;
                    }

                    if (NULL != codb_db->entries[i].unit)
                    {
                        codb_db->entries[current_entry].unit = os_strdup(codb_db->entries[i].unit);
                    }
                    else
                    {
                        codb_db->entries[current_entry].unit = NULL;
                    }

                    os_memcpy(&codb_db->entries[current_entry].object_code_attr,   &codb_db->entries[i].object_code_attr,   sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].data_type_attr,     &codb_db->entries[i].data_type_attr,     sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].access_type_attr,   &codb_db->entries[i].access_type_attr,   sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].min_elements_attr,  &codb_db->entries[i].min_elements_attr,  sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].max_elements_attr,  &codb_db->entries[i].max_elements_attr,  sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].low_limit_attr,     &codb_db->entries[i].low_limit_attr,     sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].high_limit_attr,    &codb_db->entries[i].high_limit_attr,    sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].default_value_attr, &codb_db->entries[i].default_value_attr, sizeof(obj_attr_t));
                    os_memcpy(&codb_db->entries[current_entry].mappable_attr,      &codb_db->entries[i].mappable_attr,      sizeof(obj_attr_t));

                    current_entry++;

                }
            }
            codb_db->total_entries += source_count;
            continue;
        }

        /* Skip comments. */
        if (line[0] == '#')
        {
            continue;
        }

        /* Skip empty lines. */
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
        {
            continue;
        }

        for (id = MAIN_INDEX; id <= MAPPABLE_ATTR; id++)
        {
            char* end = os_strchr(token, ':');

            if (end != NULL)
            {
                *end = '\0';
            }

            switch (id)
            {
                case MAIN_INDEX:
                {
                    char*  endptr;
                    uint16 value = (uint16)os_strtol(token, &endptr, 16);

                    if (*endptr != '\0' || value == 0x0000 || value >= 0xFFFF)
                    {
                        os_fprintf(stderr, "L%zd Error: Invalid MAIN_INDEX value '%s'\n", line_count, token);
                        os_free(line);
                        os_fclose(input_file);
                        os_free(codb_db->entries);
                        os_exit(EXIT_FAILURE);
                    }
                    else
                    {
                        if (current_entry >= codb_db->total_entries)
                        {
                            os_fprintf(stderr, "L%zd Error: More entries than expected.\n", line_count);
                            os_free(line);
                            os_fclose(input_file);
                            os_free(codb_db->entries);
                            os_exit(EXIT_FAILURE);
                        }

                        codb_db->entries[current_entry].main_index = value;

                        if (':' == endptr[1])
                        {
                            for (i = 0; i < codb_db->total_entries; i++)
                            {
                                if (codb_db->entries[i].object_name != NULL)
                                {
                                    if (0 == os_strcmp(codb_db->entries[i].object_name, token))
                                    {
                                        os_fprintf(stderr, "L%zd Error: Duplicate OBJECT_NAME value '%s'\n", line_count, token);
                                        os_free(line);
                                        os_fclose(input_file);
                                        os_free(codb_db->entries);
                                        os_exit(EXIT_FAILURE);
                                    }
                                }
                            }
                            codb_db->entries[current_entry].object_name = os_strdup(token);
                        }
                        else
                        {
                            codb_db->entries[current_entry].object_name = NULL;
                        }
                    }
                    break;
                }
                case SUB_INDEX:
                {
                    char* endptr;
                    uint8 value = (uint8)os_strtol(token, &endptr, 16);

                    if (*endptr != '\0')
                    {
                        os_fprintf(stderr, "L%zd Error: Invalid SUB_INDEX value '%s'\n", line_count, token);
                        os_free(line);
                        os_fclose(input_file);
                        os_free(codb_db->entries);
                        os_exit(EXIT_FAILURE);
                    }
                    else
                    {
                        if (current_entry >= codb_db->total_entries)
                        {
                            os_fprintf(stderr, "L%zu Error: More entries than expected.\n", line_count);
                            os_free(line);
                            os_fclose(input_file);
                            os_free(codb_db->entries);
                            os_exit(EXIT_FAILURE);
                        }
                        codb_db->entries[current_entry].sub_index = value;
                    }

                    break;
                }
                case PARAMETER_NAME:
                {
                    if (token[0] != '\0')
                    {
                        codb_db->entries[current_entry].parameter_name = os_strdup(token);
                        if (codb_db->entries[current_entry].parameter_name == NULL)
                        {
                            os_fprintf(stderr, "Error duplicating parameter name.");
                            os_free(line);
                            os_fclose(input_file);
                            os_free(codb_db->entries);
                            os_exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        codb_db->entries[current_entry].parameter_name = NULL;
                    }
                    break;
                }
                case OBJECT_KIND:
                {
                    if (0 == os_strncmp(token, "mandatory", 9))
                    {
                        codb_db->entries[current_entry].object_kind = IS_MANDATORY;
                    }
                    else if (0 == os_strncmp(token, "optional", 8))
                    {
                        codb_db->entries[current_entry].object_kind = IS_OPTIONAL;
                    }
                    else if (0 == os_strncmp(token, "conditional", 11))
                    {
                        codb_db->entries[current_entry].object_kind = IS_CONDITIONAL;
                    }
                    else
                    {
                        os_fprintf(stderr, "L%zd Error: Invalid OBJECT_KIND value '%s'\n", line_count, token);
                        os_free(line);
                        os_fclose(input_file);
                        os_free(codb_db->entries);
                        os_exit(EXIT_FAILURE);
                    }
                    break;
                }
                case UNIT:
                {
                    if (token[0] != '\0')
                    {
                        codb_db->entries[current_entry].unit = os_strdup(token);
                        if (codb_db->entries[current_entry].unit == NULL)
                        {
                            os_fprintf(stderr, "L%zd Error duplicating unit.", line_count);
                            os_free(line);
                            os_fclose(input_file);
                            os_free(codb_db->entries);
                            os_exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        codb_db->entries[current_entry].unit = NULL;
                    }
                    break;
                }
                case OBJECT_CODE:
                {
                    char* object_code = to_upper_case(token);

                    if (0 == os_strncmp(object_code, "DOMAIN", 6))
                    {
                        codb_db->entries[current_entry].object_code = IS_DOMAIN;
                    }
                    else if (0 == os_strncmp(object_code, "DEFTYPE", 7))
                    {
                        codb_db->entries[current_entry].object_code = IS_DEFTYPE;
                    }
                    else if (0 == os_strncmp(object_code, "DEFSTRUCT", 9))
                    {
                        codb_db->entries[current_entry].object_code = IS_DEFSTRUCT;
                    }
                    else if (0 == os_strncmp(object_code, "VAR", 3))
                    {
                        codb_db->entries[current_entry].object_code = IS_VAR;
                    }
                    else if (0 == os_strncmp(object_code, "ARRAY", 5))
                    {
                        codb_db->entries[current_entry].object_code = IS_ARRAY;
                    }
                    else if (0 == os_strncmp(object_code, "RECORD", 6))
                    {
                        codb_db->entries[current_entry].object_code = IS_RECORD;
                    }
                    else
                    {
                        os_fprintf(stderr, "L%zu Error: Invalid OBJECT_CODE value '%s'\n", line_count, token);
                        os_free(object_code);
                        os_free(line);
                        os_fclose(input_file);
                        os_free(codb_db->entries);
                        os_exit(EXIT_FAILURE);
                    }
                    os_free(object_code);
                    break;
                }
                case OBJECT_CODE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case DATA_TYPE:
                {
                    char* data_type = to_upper_case(token);

                    if (token[0] == '\0' || token[0] == ' ')
                    {
                        codb_db->entries[current_entry].data_type = NONE_T;
                    }
                    else if (0 == os_strncmp(data_type, "BOOLEAN", 7))
                    {
                        codb_db->entries[current_entry].data_type = BOOLEAN_T;
                    }
                    else if (0 == os_strncmp(data_type, "INTEGER8", 8))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER8;
                    }
                    else if (0 == os_strncmp(data_type, "UNSIGNED8", 9))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED8;
                    }
                    else if (0 == os_strncmp(data_type, "INTEGER16", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER16;
                    }
                    else if (0 == os_strncmp(data_type, "UNSIGNED16", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED16;
                    }
                    else if (0 == os_strncmp(data_type, "INTEGER24", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER24;
                    }
                    else if (0 == os_strncmp(data_type, "UNSIGNED24", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED24;
                    }
                    else if (0 == os_strncmp(data_type, "INTEGER32", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER32;
                    }
                    else if (0 == os_strncmp(data_type, "UNSIGNED32", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED32;
                    }
                    else if (0 == os_strncmp(data_type, "INTEGER48", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER48;
                    }
                    else if (0 == os_strncmp(data_type, "UNSIGNED48", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED48;
                    }
                    else if (0 == os_strncmp(data_type, "INTEGER64", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER64;
                    }
                    else if (0 == os_strncmp(data_type, "UNSIGNED64", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED64;
                    }
                    else if (0 == os_strncmp(data_type, "REAL32", 6))
                    {
                        codb_db->entries[current_entry].data_type = REAL32;
                    }
                    else if (0 == os_strncmp(data_type, "REAL64", 6))
                    {
                        codb_db->entries[current_entry].data_type = REAL64;
                    }
                    else if (0 == os_strncmp(data_type, "TIME_OF_DAY", 11))
                    {
                        codb_db->entries[current_entry].data_type = TIME_OF_DAY;
                    }
                    else if (0 == os_strncmp(data_type, "VISIBLE_STRING", 14))
                    {
                        codb_db->entries[current_entry].data_type = VISIBLE_STRING;
                    }
                    else if (0 == os_strncmp(data_type, "OCTET_STRING", 12))
                    {
                        codb_db->entries[current_entry].data_type = OCTET_STRING;
                    }
                    else if (0 == os_strncmp(data_type, "DOMAIN", 6))
                    {
                        codb_db->entries[current_entry].data_type = DOMAIN_T;
                    }
                    else if (0 == os_strncmp(data_type, "FLOAT", 6))
                    {
                        codb_db->entries[current_entry].data_type = FLOAT_T;
                    }
                    else
                    {
                        os_fprintf(stderr, "L%zu Error: Invalid DATA_TYPE value '%s'\n", line_count, token);
                        os_free(data_type);
                        os_free(line);
                        os_fclose(input_file);
                        os_free(codb_db->entries);
                        os_exit(EXIT_FAILURE);
                    }
                    os_free(data_type);
                    break;
                }
                case DATA_TYPE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case ACCESS_TYPE:
                {
                    if (0 == os_strncmp(token, "const", 5))
                    {
                        codb_db->entries[current_entry].access_type = CONST_T;
                    }
                    else if (0 == os_strncmp(token, "ro", 2))
                    {
                        codb_db->entries[current_entry].access_type = RO;
                    }
                    else if (0 == os_strncmp(token, "wo", 2))
                    {
                        codb_db->entries[current_entry].access_type = WO;
                    }
                    else if (0 == os_strncmp(token, "rw", 2))
                    {
                        codb_db->entries[current_entry].access_type = RW;
                    }
                    else if (0 == os_strncmp(token, "wwr", 3))
                    {
                        codb_db->entries[current_entry].access_type = WWR;
                    }
                    else if (0 == os_strncmp(token, "rww", 3))
                    {
                        codb_db->entries[current_entry].access_type = RWW;
                    }
                    else if (token[0] == '\0' || token[0] == ' ')
                    {
                        codb_db->entries[current_entry].access_type = UNSPECIFIED;
                    }
                    else
                    {
                        os_fprintf(stderr, "L%zu Error: Invalid ACCESS_TYPE value '%s'\n", line_count, token);
                        os_free(line);
                        os_fclose(input_file);
                        os_free(codb_db->entries);
                        os_exit(EXIT_FAILURE);
                    }
                    break;
                }
                case ACCESS_TYPE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case MIN_ELEMENTS:
                {
                    handle_value8(&codb_db->entries[current_entry].min_elements, token);
                    break;
                }
                case MIN_ELEMENTS_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case MAX_ELEMENTS:
                {
                    handle_value8(&codb_db->entries[current_entry].max_elements, token);
                    break;
                }
                case MAX_ELEMENTS_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case LOW_LIMIT:
                {
                    handle_value64(&codb_db->entries[current_entry].low_limit, token);
                    break;
                }
                case LOW_LIMIT_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case HIGH_LIMIT:
                {
                    handle_value64(&codb_db->entries[current_entry].high_limit, token);
                    break;
                }
                case HIGH_LIMIT_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case DEFAULT_VALUE:
                {
                    uint64 value;

                    if (token[0] == '$')
                    {
                        if (0 == os_strncmp(token, "$NODEID", 7))
                        {
                            uint64 data_value = os_strtoull(token + 7, NULL, 0);

                            value = 0x80000000 | data_value;
                        }
                        else
                        {
                            os_fprintf(stderr, "Error: Invalid value '%s'\n", token);
                            free_codb_database(codb_db);
                            os_exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        value = os_strtoull(token, NULL, 0);
                    }
                    codb_db->entries[current_entry].default_value = value;
                    break;
                }
                case DEFAULT_VALUE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case MAPPABLE:
                {
                    if (0 == os_strncmp(token, "y", 1))
                    {
                        codb_db->entries[current_entry].mappable = IS_TRUE;
                    }
                    else if (0 == os_strncmp(token, "n", 1))
                    {
                        codb_db->entries[current_entry].mappable = IS_FALSE;
                    }
                    else if (token[0] == '\0' || token[0] == ' ')
                    {
                        codb_db->entries[current_entry].mappable = IS_FALSE;
                    }
                    break;
                }
                case MAPPABLE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                default:
                    break;
            }

            if (end == NULL)
            {
                break;
            }

            token = end + 1;
        }
        current_entry++;
    }

    os_free(line);
    os_fclose(input_file);
}

static void write_json(const char* output_file_name)
{
    char*   string = NULL;
    cJSON*  root;
    FILE_t* output_file;
    size_t i;

    output_file = os_fopen(output_file_name, "wb+");
    if (NULL == output_file)
    {
        os_fprintf(stderr, "Error opening output file.");
        os_exit(EXIT_FAILURE);
    }

    root = cJSON_CreateArray();
    if (root == NULL)
    {
        os_fprintf(stderr, "Error creating JSON array.");
        os_fclose(output_file);
        os_exit(EXIT_FAILURE);
    }

    for (i = 0; i < codb_db->total_entries; i++)
    {
        if (codb_db->entries[i].object_name != NULL)
        {
            if (IS_FALSE == write_json_entry(root, i))
            {
                os_fprintf(stderr, "Error writing JSON entry.");
                cJSON_Delete(root);
                os_fclose(output_file);
                os_exit(EXIT_FAILURE);
            }
        }
    }

    string = cJSON_PrintUnformatted(root);
    os_fprintf(output_file, "%s\n", string);
    os_free(string);
    cJSON_Delete(root);

    os_fclose(output_file);
}

static void free_codb_database(codb_database_t* db)
{
    if (db != NULL)
    {
        if (db->entries != NULL)
        {
            size_t i;
            for (i = 0; i < db->total_entries; i++)
            {
                if (db->entries[i].object_name != NULL)
                {
                    os_free(db->entries[i].object_name);
                    db->entries[i].object_name = NULL;
                }
                if (db->entries[i].parameter_name != NULL)
                {
                    os_free(db->entries[i].parameter_name);
                    db->entries[i].parameter_name = NULL;
                }
                if (db->entries[i].unit != NULL)
                {
                    os_free(db->entries[i].unit);
                    db->entries[i].unit = NULL;
                }
            }
            os_free(db->entries);
            db->entries = NULL;
        }
        os_free(db);
    }
}

static bool_t write_json_entry(cJSON* root, size_t i)
{
    cJSON* id;
    cJSON* index;
    cJSON* entry;

    cJSON* parameter_name;
    cJSON* object_kind;

    cJSON* code;
    cJSON* code_type;
    cJSON* code_attr;
    cJSON* code_attr_lower_limit;
    cJSON* code_attr_upper_limit;

    cJSON* min_elements;
    cJSON* min_elements_value;
    cJSON* min_elements_attr;
    cJSON* min_elements_lower_limit;
    cJSON* min_elements_upper_limit;

    cJSON* max_elements;
    cJSON* max_elements_value;
    cJSON* max_elements_attr;
    cJSON* max_elements_lower_limit;
    cJSON* max_elements_upper_limit;

    cJSON* sub_indices;

    entry = cJSON_CreateObject();
    if (entry == NULL)
    {
        return IS_FALSE;
    }

    id    = cJSON_CreateString(codb_db->entries[i].object_name);
    index = cJSON_CreateNumber(codb_db->entries[i].main_index);
    cJSON_AddItemToObject(entry, "id", id);
    cJSON_AddItemToObject(entry, "index", index);

    sub_indices = cJSON_CreateArray();
    if (sub_indices == NULL)
    {
        cJSON_Delete(entry);
        return IS_FALSE;
    }
    cJSON_AddItemToObject(entry, "sub_indices", sub_indices);

    /* Object root data. */
    code                  = cJSON_CreateObject();
    code_type             = cJSON_CreateNumber(codb_db->entries[i].object_code);
    code_attr             = cJSON_CreateNumber(codb_db->entries[i].object_code_attr.type);
    code_attr_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].object_code_attr.lower_limit);
    code_attr_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].object_code_attr.upper_limit);
    cJSON_AddItemToObject(code, "type",  code_type);
    cJSON_AddItemToObject(code, "attr",  code_attr);
    cJSON_AddItemToObject(code, "lower", code_attr_lower_limit);
    cJSON_AddItemToObject(code, "upper", code_attr_upper_limit);

    min_elements             = cJSON_CreateObject();
    min_elements_value       = cJSON_CreateNumber(codb_db->entries[i].min_elements);
    min_elements_attr        = cJSON_CreateNumber(codb_db->entries[i].min_elements_attr.type);
    min_elements_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].min_elements_attr.lower_limit);
    min_elements_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].min_elements_attr.upper_limit);
    cJSON_AddItemToObject(min_elements, "value", min_elements_value);
    cJSON_AddItemToObject(min_elements, "attr",  min_elements_attr);
    cJSON_AddItemToObject(min_elements, "lower", min_elements_lower_limit);
    cJSON_AddItemToObject(min_elements, "upper", min_elements_upper_limit);

    max_elements             = cJSON_CreateObject();
    max_elements_value       = cJSON_CreateNumber(codb_db->entries[i].max_elements);
    max_elements_attr        = cJSON_CreateNumber(codb_db->entries[i].max_elements_attr.type);
    max_elements_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].max_elements_attr.lower_limit);
    max_elements_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].max_elements_attr.upper_limit);
    cJSON_AddItemToObject(max_elements, "value", max_elements_value);
    cJSON_AddItemToObject(max_elements, "attr",  max_elements_attr);
    cJSON_AddItemToObject(max_elements, "lower", max_elements_lower_limit);
    cJSON_AddItemToObject(max_elements, "upper", max_elements_upper_limit);

    cJSON_AddItemToObject(entry, "code",         code);
    cJSON_AddItemToObject(entry, "min_elements", min_elements);
    cJSON_AddItemToObject(entry, "max_elements", max_elements);

    parameter_name = cJSON_CreateString(codb_db->entries[i].parameter_name);

    cJSON_AddItemToObject(entry, "desc", parameter_name);

    /* Object has only one sub-index. */
    if (i + 1 >= codb_db->total_entries || codb_db->entries[i].main_index != codb_db->entries[i + 1].main_index)
    {
        if (IS_FALSE == add_sub_index_to_object(sub_indices, i))
            {
                cJSON_Delete(entry);
                return IS_FALSE;
            }
    }
    else
    {
        i++;
        while (i < codb_db->total_entries && codb_db->entries[i - 1].main_index == codb_db->entries[i].main_index)
        {
            if (IS_FALSE == add_sub_index_to_object(sub_indices, i))
            {
                cJSON_Delete(entry);
                return IS_FALSE;
            }
            i++;
        }
    }

    cJSON_AddItemToArray(root, entry);
    return IS_TRUE;
}

static void handle_attribute(obj_attr_t* attr, const char* token)
{
    if (0 == os_strncmp(token, "m", 1))
    {
        attr->type = MANDATORY;
    }
    else if (0 == os_strncmp(token, "d", 1))
    {
        if (token[1] == '[')
        {
            const char* limits_start = token + 2;
            const char* limits_end   = os_strchr(limits_start, ']');
            attr->type               = DEFAULT_LIMITS;

            if (limits_end != NULL)
            {
                char  limits[256];
                char* lower_limit_str;
                char* upper_limit_str;
                char* saveptr;

                os_strlcpy(limits, limits_start, limits_end - limits_start);
                limits[limits_end - limits_start] = '\0';

                lower_limit_str = os_strtokr(limits, ",", &saveptr);
                upper_limit_str = os_strtokr(NULL,   ",", &saveptr);

                if (lower_limit_str != NULL)
                {
                    attr->lower_limit = os_strtoull(lower_limit_str, NULL, 0);
                }
                if (upper_limit_str != NULL)
                {
                    attr->upper_limit = os_strtoull(upper_limit_str, NULL, 0);
                }
            }
        }
        else
        {
            attr->type = DEFAULT;
        }
    }
    else if (0 == os_strncmp(token, "n", 1))
    {
        attr->type = NOT_APPLICABLE;
    }
    else
    {
        attr->type = EMPTY;
    }
}

static void handle_value8(uint8* value, const char* token)
{
    *value = (uint8)os_strtoul(token, NULL, 0);
}

static void handle_value64(uint64* value, const char* token)
{
    *value = os_strtoull(token, NULL, 0);
}

static char* to_upper_case(const char* str)
{
    size_t i;
    char*  upper_case = os_strdup(str);

    if (NULL == upper_case)
    {
        os_fprintf(stderr, "Error duplicating string.");
        os_exit(EXIT_FAILURE);
    }

    for (i = 0; i < os_strlen(upper_case); i++)
    {
        upper_case[i] = os_toupper(upper_case[i]);
    }

    return upper_case;
}
