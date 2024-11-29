/** @file codb2json.c
 *
 *  CANopen Profile data base to JSON converter.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cJSON.h"

#ifdef _WIN32
#define strdup _strdup
#endif

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
 * 12. Minimum elements, 0..254
 * 13. Minimum elements attribute, see attribute description below
 * 14. Maximum elements, 0..254
 * 15. Maximum elements attribute, see attribute description below
 * 16. Low limit, decimal valu or 0x followed by hex value
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
 * d[limits] - data value is a default and can be changed within the limites
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
    uint64_t        lower_limit;
    uint64_t        upper_limit;

} obj_attr_t;


typedef enum obj_kind
{
    IS_MANDATORY,
    IS_OPTIONAL,
    IS_CONDITIONAL

} obj_kind_t;

typedef enum obj_code
{
    DOMAIN,
    DEFTYPE,
    DEFSTRUCT,
    VAR,
    ARRAY,
    RECORD

} obj_code_t;

typedef enum data_type
{
    NONE,
    BOOLEAN,
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
    FLOAT,
    TIME_OF_DAY,
    VISIBLE_STRING,
    OCTET_STRING,
    DOMAIN_TYPE

} data_type_t;

typedef enum acc_type
{
    CONST,
    RO,
    WO,
    RW,
    WWR,
    RWW,
    UNSPECIFIED

} acc_type_t;

typedef struct codb_entry
{
    char*       object_name;
    uint16_t    main_index;
    uint8_t     sub_index;
    char*       parameter_name;
    obj_kind_t  object_kind;
    char*       unit;
    obj_code_t  object_code;
    obj_attr_t  object_code_attr;
    data_type_t data_type;
    obj_attr_t  data_type_attr;
    acc_type_t  access_type;
    obj_attr_t  access_type_attr;
    uint8_t     min_elements;
    obj_attr_t  min_elements_attr;
    uint8_t     max_elements;
    obj_attr_t  max_elements_attr;
    uint64_t    low_limit;
    obj_attr_t  low_limit_attr;
    uint64_t    high_limit;
    obj_attr_t  high_limit_attr;
    uint64_t    default_value;
    obj_attr_t  default_value_attr;
    bool        mappable;
    obj_attr_t  mappable_attr;

} codb_entry_t;

typedef struct codb_database
{
    codb_entry_t* entries;
    size_t        total_entries;

} codb_database_t;

static codb_database_t* codb_db;

static bool  add_sub_index_to_object(cJSON* sub_indices, size_t i);
static bool  is_codb_file(const char* input_file_name);
static void  read_codb(const char* input_file_name);
static void  write_json(const char* output_file_name);
static bool  write_json_entry(cJSON* entry, size_t i);
static void  free_codb_database(codb_database_t* db);
static void  handle_attribute(obj_attr_t* attr, const char* token);
static void  handle_value8(uint8_t*  value, const char* token);
static void  handle_value64(uint64_t* value, const char* token);
static char* to_upper_case(const char* str);

int main(int argc, char* argv[])
{
    char  json_file_name[256] = { 0 };
    char* dot;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (false == is_codb_file(argv[1]))
    {
        fprintf(stderr, "Error: %s is not a .codb file.\n", argv[1]);
        return EXIT_FAILURE;
    }

    codb_db = (codb_database_t*)malloc(sizeof(codb_database_t));
    if (codb_db == NULL)
    {
        perror("Error allocating memory for codb_db");
        return EXIT_FAILURE;
    }
    codb_db->entries       = NULL;
    codb_db->total_entries = 0;

    strncpy(json_file_name, argv[1], sizeof(json_file_name) - 1);
    json_file_name[sizeof(json_file_name) - 1] = '\0';
    dot = strrchr(json_file_name, '.');
    if (dot != NULL)
    {
        *dot = '\0';
    }
    strcat(json_file_name, ".json");

    read_codb(argv[1]);
    write_json(json_file_name);

    free_codb_database(codb_db);
    printf("File conversion completed successfully.\n");

    return EXIT_SUCCESS;
}

static bool add_sub_index_to_object(cJSON *sub_indices, size_t i)
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

    sub_index = cJSON_CreateObject();
    if (sub_index == NULL)
    {
        return false;
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

    cJSON_AddItemToObject(sub_index, "index",       sub_index_value);
    cJSON_AddItemToObject(sub_index, "desc",        parameter_name);
    cJSON_AddItemToObject(sub_index, "kind",        object_kind);
    cJSON_AddItemToObject(sub_index, "unit",        unit);
    cJSON_AddItemToObject(sub_index, "data_type",   data_type);
    cJSON_AddItemToObject(sub_index, "access_type", access_type);

    cJSON_AddItemToArray(sub_indices, sub_index);

    return true;
}

static bool is_codb_file(const char* input_file_name)
{
    char line[BUFFER_SIZE] = { 0 };
    FILE* input_file       = fopen(input_file_name, "rb");

    if (input_file == NULL)
    {
        perror("Error opening input file.");
        return false;
    }

    while (fgets(line, sizeof(line), input_file) != NULL)
    {
        if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r' || isxdigit(line[0]) || line[0] == '#' || line[0] == '$')
        {
            continue;
        }
        else
        {
            fclose(input_file);
            return false;
        }
    }

    fclose(input_file);
    return true;
}


static void read_codb(const char* input_file_name)
{
    FILE*  input_file    = fopen(input_file_name, "rb");
    char*  line          = NULL;
    size_t entry_count   = 0;
    size_t line_count    = 0;
    size_t current_entry;
    size_t i;

    if (input_file == NULL)
    {
        perror("Error opening input file.");
        exit(EXIT_FAILURE);
    }

    line = (char*)malloc(BUFFER_SIZE);
    if (line == NULL)
    {
        perror("Error allocating memory for line buffer.");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    /* First pass: count the number of entries. */
    while (fgets(line, BUFFER_SIZE, input_file) != NULL)
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
        {
            continue;
        }
        entry_count++;
    }

    /* Allocate memory for all entries in advance. */
    codb_db->entries = (codb_entry_t*)malloc(entry_count * sizeof(codb_entry_t));
    if (codb_db->entries == NULL)
    {
        perror("Error allocating memory for entries.");
        free(line);
        fclose(input_file);
        exit(EXIT_FAILURE);
    }
    codb_db->total_entries = entry_count;

    /* Initialize all entries to default values. */
    for (i = 0; i < entry_count; i++)
    {
        codb_db->entries[i].object_name                    = NULL;
        codb_db->entries[i].main_index                     = 0;
        codb_db->entries[i].sub_index                      = 0;
        codb_db->entries[i].parameter_name                 = NULL;
        codb_db->entries[i].object_kind                    = IS_OPTIONAL;
        codb_db->entries[i].unit                           = NULL;
        codb_db->entries[i].object_code                    = DOMAIN;
        codb_db->entries[i].object_code_attr.type          = EMPTY;
        codb_db->entries[i].object_code_attr.lower_limit   = 0;
        codb_db->entries[i].object_code_attr.upper_limit   = 0;
        codb_db->entries[i].data_type                      = NONE;
        codb_db->entries[i].data_type_attr.type            = EMPTY;
        codb_db->entries[i].data_type_attr.lower_limit     = 0;
        codb_db->entries[i].data_type_attr.upper_limit     = 0;
        codb_db->entries[i].access_type                    = UNSPECIFIED;
        codb_db->entries[i].access_type_attr.type          = EMPTY;
        codb_db->entries[i].access_type_attr.lower_limit   = 0;
        codb_db->entries[i].access_type_attr.upper_limit   = 0;
        codb_db->entries[i].min_elements                   = 0;
        codb_db->entries[i].min_elements_attr.type         = EMPTY;
        codb_db->entries[i].min_elements_attr.lower_limit  = 0;
        codb_db->entries[i].min_elements_attr.upper_limit  = 0;
        codb_db->entries[i].max_elements                   = 0;
        codb_db->entries[i].max_elements_attr.type         = EMPTY;
        codb_db->entries[i].max_elements_attr.lower_limit  = 0;
        codb_db->entries[i].max_elements_attr.upper_limit  = 0;
        codb_db->entries[i].low_limit                      = 0;
        codb_db->entries[i].low_limit_attr.type            = EMPTY;
        codb_db->entries[i].low_limit_attr.lower_limit     = 0;
        codb_db->entries[i].low_limit_attr.upper_limit     = 0;
        codb_db->entries[i].high_limit                     = 0;
        codb_db->entries[i].high_limit_attr.type           = EMPTY;
        codb_db->entries[i].high_limit_attr.lower_limit    = 0;
        codb_db->entries[i].high_limit_attr.upper_limit    = 0;
        codb_db->entries[i].default_value                  = 0;
        codb_db->entries[i].default_value_attr.type        = EMPTY;
        codb_db->entries[i].default_value_attr.lower_limit = 0;
        codb_db->entries[i].default_value_attr.upper_limit = 0;
        codb_db->entries[i].mappable                       = false;
        codb_db->entries[i].mappable_attr.type             = EMPTY;
        codb_db->entries[i].mappable_attr.lower_limit      = 0;
        codb_db->entries[i].mappable_attr.upper_limit      = 0;
    }

    /* Reset file pointer to the beginning */
    rewind(input_file);

    current_entry = 0;
    while (fgets(line, BUFFER_SIZE, input_file) != NULL)
    {
        field_id_t id;
        char*      token = line;

        line_count++;

        /* Check if the line is too long. */
        if (strchr(line, '\n') == NULL && !feof(input_file))
        {
            fprintf(stderr, "Error: Line too long in input file.\n");
            free(line);
            fclose(input_file);
            free(codb_db->entries);
            exit(EXIT_FAILURE);
        }

        /* Copy object. */
        if (line[0] == '$')
        {
            char*    end;
            uint16_t source_index;
            uint16_t dest_index;

            end = strchr(token, '=');
            if (end == NULL)
            {
                fprintf(stderr, "L%zu Error: Invalid object copy syntax.\n", line_count);
                free(line);
                fclose(input_file);
                free(codb_db->entries);
                exit(EXIT_FAILURE);
            }
            else
            {
                *end         = '\0';
                source_index = (uint16_t)strtol(end   + 1, NULL, 16);
                dest_index   = (uint16_t)strtol(token + 1, NULL, 16);
            }

            for (i = 0; i < codb_db->total_entries; i++)
            {
                if (codb_db->entries[i].main_index == source_index)
                {
                    char object_name_str[5] = { 0 };
                    sprintf(object_name_str, "%04X", dest_index);
                    codb_db->entries[current_entry].object_name    = strdup(object_name_str);
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
                        codb_db->entries[current_entry].parameter_name = strdup(codb_db->entries[i].parameter_name);
                    }
                    else
                    {
                        codb_db->entries[current_entry].parameter_name = NULL;
                    }

                    if (NULL != codb_db->entries[i].unit)
                    {
                        codb_db->entries[current_entry].unit = strdup(codb_db->entries[i].unit);
                    }
                    else
                    {
                        codb_db->entries[current_entry].unit = NULL;
                    }

                    memcpy(&codb_db->entries[current_entry].object_code_attr,   &codb_db->entries[i].object_code_attr,   sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].data_type_attr,     &codb_db->entries[i].data_type_attr,     sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].access_type_attr,   &codb_db->entries[i].access_type_attr,   sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].min_elements_attr,  &codb_db->entries[i].min_elements_attr,  sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].max_elements_attr,  &codb_db->entries[i].max_elements_attr,  sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].low_limit_attr,     &codb_db->entries[i].low_limit_attr,     sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].high_limit_attr,    &codb_db->entries[i].high_limit_attr,    sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].default_value_attr, &codb_db->entries[i].default_value_attr, sizeof(obj_attr_t));
                    memcpy(&codb_db->entries[current_entry].mappable_attr,      &codb_db->entries[i].mappable_attr,      sizeof(obj_attr_t));

                    current_entry++;
                    break;
                }
            }

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
            char* end = strchr(token, ':');

            if (end != NULL)
            {
                *end = '\0';
            }

            switch (id)
            {
                case MAIN_INDEX:
                {
                    char*    endptr;
                    uint16_t value = (uint16_t)strtol(token, &endptr, 16);

                    if (*endptr != '\0' || value <= 0x0000 || value >= 0xFFFF)
                    {
                        fprintf(stderr, "L%zu Error: Invalid MAIN_INDEX value '%s'\n", line_count, token);
                        free(line);
                        fclose(input_file);
                        free(codb_db->entries);
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        if (current_entry >= entry_count)
                        {
                            fprintf(stderr, "L%zu Error: More entries than expected.\n", line_count);
                            free(line);
                            fclose(input_file);
                            free(codb_db->entries);
                            exit(EXIT_FAILURE);
                        }

                        codb_db->entries[current_entry].main_index = value;

                        if (':' == endptr[1])
                        {
                            for (i = 0; i < codb_db->total_entries; i++)
                            {
                                if (codb_db->entries[i].object_name != NULL)
                                {
                                    if (0 == strcmp(codb_db->entries[i].object_name, token))
                                    {
                                        fprintf(stderr, "L%zu Error: Duplicate OBJECT_NAME value '%s'\n", line_count, token);
                                        free(line);
                                        fclose(input_file);
                                        free(codb_db->entries);
                                        exit(EXIT_FAILURE);
                                    }
                                }
                            }
                            codb_db->entries[current_entry].object_name = strdup(token);
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
                    char*   endptr;
                    uint8_t value = (uint8_t)strtol(token, &endptr, 16);

                    if (*endptr != '\0')
                    {
                        fprintf(stderr, "L%ld Error: Invalid SUB_INDEX value '%s'\n", line_count, token);
                        free(line);
                        fclose(input_file);
                        free(codb_db->entries);
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        if (current_entry >= entry_count)
                        {
                            fprintf(stderr, "L%zu Error: More entries than expected.\n", line_count);
                            free(line);
                            fclose(input_file);
                            free(codb_db->entries);
                            exit(EXIT_FAILURE);
                        }
                        codb_db->entries[current_entry].sub_index = value;
                    }

                    break;
                }
                case PARAMETER_NAME:
                {
                    if (token[0] != '\0')
                    {
                        codb_db->entries[current_entry].parameter_name = strdup(token);
                        if (codb_db->entries[current_entry].parameter_name == NULL)
                        {
                            perror("Error duplicating parameter name.");
                            free(line);
                            fclose(input_file);
                            free(codb_db->entries);
                            exit(EXIT_FAILURE);
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
                    if (0 == strncmp(token, "mandatory", 9))
                    {
                        codb_db->entries[current_entry].object_kind = IS_MANDATORY;
                    }
                    else if (0 == strncmp(token, "optional", 8))
                    {
                        codb_db->entries[current_entry].object_kind = IS_OPTIONAL;
                    }
                    else if (0 == strncmp(token, "conditional", 11))
                    {
                        codb_db->entries[current_entry].object_kind = IS_CONDITIONAL;
                    }
                    else
                    {
                        fprintf(stderr, "L%zu Error: Invalid OBJECT_KIND value '%s'\n", line_count, token);
                        free(line);
                        fclose(input_file);
                        free(codb_db->entries);
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
                case UNIT:
                {
                    if (token[0] != '\0')
                    {
                        codb_db->entries[current_entry].unit = strdup(token);
                        if (codb_db->entries[current_entry].unit == NULL)
                        {
                            perror("L%zu Error duplicating unit.");
                            free(line);
                            fclose(input_file);
                            free(codb_db->entries);
                            exit(EXIT_FAILURE);
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

                    if (0 == strncmp(object_code, "DOMAIN", 6))
                    {
                        codb_db->entries[current_entry].object_code = DOMAIN;
                    }
                    else if (0 == strncmp(object_code, "DEFTYPE", 7))
                    {
                        codb_db->entries[current_entry].object_code = DEFTYPE;
                    }
                    else if (0 == strncmp(object_code, "DEFSTRUCT", 9))
                    {
                        codb_db->entries[current_entry].object_code = DEFSTRUCT;
                    }
                    else if (0 == strncmp(object_code, "VAR", 3))
                    {
                        codb_db->entries[current_entry].object_code = VAR;
                    }
                    else if (0 == strncmp(object_code, "ARRAY", 5))
                    {
                        codb_db->entries[current_entry].object_code = ARRAY;
                    }
                    else if (0 == strncmp(object_code, "RECORD", 6))
                    {
                        codb_db->entries[current_entry].object_code = RECORD;
                    }
                    else
                    {
                        fprintf(stderr, "L%zu Error: Invalid OBJECT_CODE value '%s'\n", line_count, token);
                        free(object_code);
                        free(line);
                        fclose(input_file);
                        free(codb_db->entries);
                        exit(EXIT_FAILURE);
                    }
                    free(object_code);
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
                        codb_db->entries[current_entry].data_type = NONE;
                    }
                    else if (0 == strncmp(data_type, "BOOLEAN", 7))
                    {
                        codb_db->entries[current_entry].data_type = BOOLEAN;
                    }
                    else if (0 == strncmp(data_type, "INTEGER8", 8))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER8;
                    }
                    else if (0 == strncmp(data_type, "UNSIGNED8", 9))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED8;
                    }
                    else if (0 == strncmp(data_type, "INTEGER16", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER16;
                    }
                    else if (0 == strncmp(data_type, "UNSIGNED16", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED16;
                    }
                    else if (0 == strncmp(data_type, "INTEGER24", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER24;
                    }
                    else if (0 == strncmp(data_type, "UNSIGNED24", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED24;
                    }
                    else if (0 == strncmp(data_type, "INTEGER32", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER32;
                    }
                    else if (0 == strncmp(data_type, "UNSIGNED32", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED32;
                    }
                    else if (0 == strncmp(data_type, "INTEGER48", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER48;
                    }
                    else if (0 == strncmp(data_type, "UNSIGNED48", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED48;
                    }
                    else if (0 == strncmp(data_type, "INTEGER64", 9))
                    {
                        codb_db->entries[current_entry].data_type = INTEGER64;
                    }
                    else if (0 == strncmp(data_type, "UNSIGNED64", 10))
                    {
                        codb_db->entries[current_entry].data_type = UNSIGNED64;
                    }
                    else if (0 == strncmp(data_type, "REAL32", 6))
                    {
                        codb_db->entries[current_entry].data_type = REAL32;
                    }
                    else if (0 == strncmp(data_type, "REAL64", 6))
                    {
                        codb_db->entries[current_entry].data_type = REAL64;
                    }
                    else if (0 == strncmp(data_type, "TIME_OF_DAY", 11))
                    {
                        codb_db->entries[current_entry].data_type = TIME_OF_DAY;
                    }
                    else if (0 == strncmp(data_type, "VISIBLE_STRING", 14))
                    {
                        codb_db->entries[current_entry].data_type = VISIBLE_STRING;
                    }
                    else if (0 == strncmp(data_type, "OCTET_STRING", 12))
                    {
                        codb_db->entries[current_entry].data_type = OCTET_STRING;
                    }
                    else if (0 == strncmp(data_type, "DOMAIN", 6))
                    {
                        codb_db->entries[current_entry].data_type = DOMAIN_TYPE;
                    }
                    else if (0 == strncmp(data_type, "FLOAT", 6))
                    {
                        codb_db->entries[current_entry].data_type = FLOAT;
                    }
                    else
                    {
                        fprintf(stderr, "L%zu Error: Invalid DATA_TYPE value '%s'\n", line_count, token);
                        free(data_type);
                        free(line);
                        fclose(input_file);
                        free(codb_db->entries);
                        exit(EXIT_FAILURE);
                    }
                    free(data_type);
                    break;
                }
                case DATA_TYPE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case ACCESS_TYPE:
                {
                    if (0 == strncmp(token, "const", 5))
                    {
                        codb_db->entries[current_entry].access_type = CONST;
                    }
                    else if (0 == strncmp(token, "ro", 2))
                    {
                        codb_db->entries[current_entry].access_type = RO;
                    }
                    else if (0 == strncmp(token, "wo", 2))
                    {
                        codb_db->entries[current_entry].access_type = WO;
                    }
                    else if (0 == strncmp(token, "rw", 2))
                    {
                        codb_db->entries[current_entry].access_type = RW;
                    }
                    else if (0 == strncmp(token, "wwr", 3))
                    {
                        codb_db->entries[current_entry].access_type = WWR;
                    }
                    else if (0 == strncmp(token, "rww", 3))
                    {
                        codb_db->entries[current_entry].access_type = RWW;
                    }
                    else if (token[0] == '\0' || token[0] == ' ')
                    {
                        codb_db->entries[current_entry].access_type = UNSPECIFIED;
                    }
                    else
                    {
                        fprintf(stderr, "L%zu Error: Invalid ACCESS_TYPE value '%s'\n", line_count, token);
                        free(line);
                        fclose(input_file);
                        free(codb_db->entries);
                        exit(EXIT_FAILURE);
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
                    uint64_t value;

                    if (token[0] == '$')
                    {
                        if (0 == strncmp(token, "$NODEID", 7))
                        {
                            uint64_t data_value = strtoull(token + 7, NULL, 0);

                            value = 0x80000000 | data_value;

                            codb_db->entries[current_entry].default_value = value;
                        }
                        else
                        {
                            fprintf(stderr, "Error: Invalid value '%s'\n", token);
                            free_codb_database(codb_db);
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        value = strtoull(token, NULL, 0);
                    }
                    break;
                }
                case DEFAULT_VALUE_ATTR:
                {
                    handle_attribute(&codb_db->entries[current_entry].object_code_attr, token);
                    break;
                }
                case MAPPABLE:
                {
                    if (0 == strncmp(token, "y", 1))
                    {
                        codb_db->entries[current_entry].mappable = true;
                    }
                    else if (0 == strncmp(token, "n", 1))
                    {
                        codb_db->entries[current_entry].mappable = false;
                    }
                    else if (token[0] == '\0' || token[0] == ' ')
                    {
                        codb_db->entries[current_entry].mappable = false;
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

    free(line);
    fclose(input_file);
}

static void write_json(const char* output_file_name)
{
    char*  string = NULL;
    cJSON* root;
    FILE*  output_file;
    size_t i;

    output_file = fopen(output_file_name, "wb+");
    if (NULL == output_file)
    {
        perror("Error opening output file.");
        exit(EXIT_FAILURE);
    }

    root = cJSON_CreateArray();
    if (root == NULL)
    {
        perror("Error creating JSON array.");
        fclose(output_file);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < codb_db->total_entries; i++)
    {
        if (codb_db->entries[i].object_name != NULL)
        {
            if (false == write_json_entry(root, i))
            {
                perror("Error writing JSON entry.");
                cJSON_Delete(root);
                fclose(output_file);
                exit(EXIT_FAILURE);
            }
        }
    }

    string = cJSON_Print(root);
    fprintf(output_file, "%s\n", string);
    free(string);
    cJSON_Delete(root);

    fclose(output_file);
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
                    free(db->entries[i].object_name);
                    db->entries[i].object_name = NULL;
                }
                if (db->entries[i].parameter_name != NULL)
                {
                    free(db->entries[i].parameter_name);
                    db->entries[i].parameter_name = NULL;
                }
                if (db->entries[i].unit != NULL)
                {
                    free(db->entries[i].unit);
                    db->entries[i].unit = NULL;
                }
            }
            free(db->entries);
            db->entries = NULL;
        }
        free(db);
        db = NULL;
    }
}

static bool write_json_entry(cJSON* root, size_t i)
{
    cJSON* id;
    cJSON* index;
    cJSON* entry;

    cJSON* parameter_name;
    cJSON* object_kind;

    cJSON* object_code;
    cJSON* object_code_type;
    cJSON* code_attr;
    cJSON* code_attr_lower_limit;
    cJSON* code_attr_upper_limit;

    cJSON* sub_indices;

    entry = cJSON_CreateObject();
    if (entry == NULL)
    {
        return false;
    }

    id    = cJSON_CreateString(codb_db->entries[i].object_name);
    index = cJSON_CreateNumber(codb_db->entries[i].main_index);
    cJSON_AddItemToObject(entry, "id", id);
    cJSON_AddItemToObject(entry, "index", index);

    sub_indices = cJSON_CreateArray();
    if (sub_indices == NULL)
    {
        cJSON_Delete(entry);
        return false;
    }
    cJSON_AddItemToObject(entry, "sub_indices", sub_indices);

    /* Object root data. */
    object_code           = cJSON_CreateObject();
    object_code_type      = cJSON_CreateNumber(codb_db->entries[i].object_code);
    code_attr             = cJSON_CreateNumber(codb_db->entries[i].object_code_attr.type);
    code_attr_lower_limit = cJSON_CreateNumber((double)codb_db->entries[i].object_code_attr.lower_limit);
    code_attr_upper_limit = cJSON_CreateNumber((double)codb_db->entries[i].object_code_attr.upper_limit);
    cJSON_AddItemToObject(object_code, "type",  object_code_type);
    cJSON_AddItemToObject(object_code, "attr",  code_attr);
    cJSON_AddItemToObject(object_code, "lower", code_attr_lower_limit);
    cJSON_AddItemToObject(object_code, "upper", code_attr_upper_limit);

    cJSON_AddItemToObject(entry, "code", object_code);

    parameter_name = cJSON_CreateString(codb_db->entries[i].parameter_name);
    object_kind    = cJSON_CreateNumber(codb_db->entries[i].object_kind);

    cJSON_AddItemToObject(entry, "desc", parameter_name);
    cJSON_AddItemToObject(entry, "kind", object_kind);

    /* Object has only one sub-index. */
    if (i + 1 >= codb_db->total_entries || codb_db->entries[i].main_index != codb_db->entries[i + 1].main_index)
    {
        if (false == add_sub_index_to_object(sub_indices, i))
            {
                cJSON_Delete(entry);
                return false;
            }
        i++;
    }
    else
    {
        i++;
        while (i < codb_db->total_entries && codb_db->entries[i - 1].main_index == codb_db->entries[i].main_index)
        {
            if (false == add_sub_index_to_object(sub_indices, i))
            {
                cJSON_Delete(entry);
                return false;
            }
            i++;
        }
    }

    cJSON_AddItemToArray(root, entry);
    return true;
}

static void handle_attribute(obj_attr_t* attr, const char* token)
{
    if (0 == strncmp(token, "m", 1))
    {
        attr->type = MANDATORY;
    }
    else if (0 == strncmp(token, "d", 1))
    {
        if (token[1] == '[')
        {
            const char* limits_start = token + 2;
            const char* limits_end   = strchr(limits_start, ']');
            attr->type               = DEFAULT_LIMITS;

            if (limits_end != NULL)
            {
                char  limits[256];
                char* lower_limit_str;
                char* upper_limit_str;

                strncpy(limits, limits_start, limits_end - limits_start);
                limits[limits_end - limits_start] = '\0';

                lower_limit_str = strtok(limits, ",");
                upper_limit_str = strtok(NULL,   ",");

                if (lower_limit_str != NULL)
                {
                    attr->lower_limit = strtoull(lower_limit_str, NULL, 0);
                }
                if (upper_limit_str != NULL)
                {
                    attr->upper_limit = strtoull(upper_limit_str, NULL, 0);
                }
            }
        }
        else
        {
            attr->type = DEFAULT;
        }
    }
    else if (0 == strncmp(token, "n", 1))
    {
        attr->type = NOT_APPLICABLE;
    }
    else
    {
        attr->type = EMPTY;
    }
}

static void handle_value8(uint8_t* value, const char* token)
{
    *value = (uint8_t)strtoul(token, NULL, 0);
}

static void handle_value64(uint64_t* value, const char* token)
{
    *value = strtoull(token, NULL, 0);
}

static char* to_upper_case(const char* str)
{
    size_t i;
    char*  upper_case = strdup(str);

    if (NULL == upper_case)
    {
        perror("Error duplicating string.");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < strlen(upper_case); i++)
    {
        upper_case[i] = toupper(upper_case[i]);
    }

    return upper_case;
}
