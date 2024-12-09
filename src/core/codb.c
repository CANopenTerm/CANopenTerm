/** @file codb.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "cJSON.h"
#include "codb.h"
#include "common.h"
#include "core.h"
#include "os.h"
#include "table.h"

static const char* file_name_to_profile_desc(const char* file_name);

static uint32 active_no = 0;
static cJSON* ds301     = NULL;
static cJSON* codb      = NULL;

void codb_init(void)
{
    FILE_t*     file;
    char        file_path[512] = { 0 };
    const char* data_path      = os_find_data_path();
    char*       file_content;
    size_t      file_size;

    if (NULL == data_path)
    {
        os_log(LOG_ERROR, "Data path not found.");
        return;
    }

    os_snprintf(file_path, sizeof(file_path), "%s/codb/ds301.json", data_path);
    os_fix_path(file_path);

    file = os_fopen(file_path, "rb");
    if (NULL == file)
    {
        os_log(LOG_ERROR, "Failed to open file: %s", file_path);
        return;
    }

    os_fseek(file, 0, SEEK_END);
    file_size = os_ftell(file);
    os_fseek(file, 0, SEEK_SET);

    file_content = (char*)os_calloc(file_size + 1, sizeof(char));
    if (NULL == file_content)
    {
        os_log(LOG_ERROR, "Memory allocation failed for file content.");
        os_fclose(file);
        return;
    }

    if (os_fread(file_content, 1, file_size, file) != file_size)
    {
        os_log(LOG_ERROR, "Failed to read file: %s", file_path);
        os_free(file_content);
        os_fclose(file);
        return;
    }
    file_content[file_size] = '\0';
    os_fclose(file);

    ds301 = cJSON_Parse(file_content);
    os_free(file_content);

    if (ds301 == NULL)
    {
        os_log(LOG_ERROR, "Failed to parse JSON content from file: %s", file_path);
    }
}

void codb_deinit(void)
{
    if (ds301 != NULL)
    {
        cJSON_Delete(ds301);
        ds301 = NULL;
    }

    unload_codb();
}

const char* codb_desc_lookup(codb_t* db, uint16 index, uint8 sub_index)
{
    cJSON*      object    = NULL;
    static char desc[256] = { 0 };

    if (NULL == db)
    {
        return NULL;
    }

    object = NULL;
    cJSON_ArrayForEach(object, db)
    {
        cJSON* json_index = cJSON_GetObjectItem(object, "index");
        if (json_index != NULL && json_index->valueint == index)
        {
            cJSON* sub_indices = cJSON_GetObjectItem(object, "sub_indices");
            cJSON* obj_desc = cJSON_GetObjectItem(object, "desc");

            if (obj_desc == NULL || obj_desc->valuestring == NULL)
            {
                continue;
            }

            if (sub_indices != NULL)
            {
                cJSON* sub_index_item = cJSON_GetArrayItem(sub_indices, sub_index);
                if (sub_index_item != NULL)
                {
                    cJSON* sub_desc = cJSON_GetObjectItem(sub_index_item, "desc");
                    if (sub_desc != NULL && sub_desc->valuestring != NULL)
                    {
                        if (0 == os_strcmp(obj_desc->valuestring, sub_desc->valuestring))
                        {
                            os_snprintf(desc, sizeof(desc), "%s", obj_desc->valuestring);
                        }
                        else
                        {
                            os_snprintf(desc, sizeof(desc), "%s, %s", obj_desc->valuestring, sub_desc->valuestring);
                        }
                        return desc;
                    }
                }
                else
                {
                    return NULL;
                }
            }

            os_snprintf(desc, sizeof(desc), "%s", obj_desc->valuestring);
            return desc;
        }
    }

    return NULL;
}

codb_t* codb_get_ds301_profile(void)
{
    return (codb_t*)ds301;
}

codb_t* codb_get_profile(void)
{
    return (codb_t*)codb;
}

bool_t is_ds301_loaded(void)
{
    return (ds301 != NULL) ? IS_TRUE : IS_FALSE;
}

bool_t is_codb_loaded(void)
{
    return (codb != NULL) ? IS_TRUE : IS_FALSE;
}

void list_codb(void)
{
    const char* data_path      = os_find_data_path();
    char        file_path[512] = { 0 };
    DIR_t*      d;
    table_t     table          = { DARK_CYAN, DARK_WHITE, 3, 57, 1 };
    status_t    status;

    os_snprintf(file_path, sizeof(file_path), "%s/codb", data_path);

    d = os_opendir(file_path);
    if (NULL == d)
    {
        os_log(LOG_WARNING, "Could not open codb directory.");
        return;
    }

    table.column_c_width = 6;
    status               = table_init(&table, 1024);
    if (ALL_OK != status)
    {
        os_log(LOG_ERROR, "Failed to initialize table.");
        os_closedir(d);
        return;
    }

    if (ALL_OK != status)
    {
        return;
    }

    os_print(LIGHT_RED, "\n  The accuracy of the data has not been completely verified.\n");

    if (d)
    {
        struct dirent_t* dir;
        uint32 file_no = 1;

        table_print_header(&table);
        table_print_row("No.", "Profile", "Status", &table);
        table_print_divider(&table);

        while ((dir = os_readdir(d)) != NULL)
        {
            char* extension = ".json";

            if (os_strstr(dir->d_name, extension) != NULL)
            {
                char file_no_str[4] = { 0 };

                os_snprintf(file_no_str, 4, "%3u", file_no);

                if ((0 == os_strcmp(dir->d_name, "ds301.json")) || ((active_no > 0) && (active_no == file_no)))
                {
                    table.text_color = LIGHT_GREEN;
                    table_print_row(file_no_str, file_name_to_profile_desc(dir->d_name), "Active", &table);
                    table.text_color = DARK_WHITE;
                }
                else
                {
                    table_print_row(file_no_str, file_name_to_profile_desc(dir->d_name), " ", &table);
                }
                file_no++;
            }
        }
        os_closedir(d);
    }
    else
    {
        os_log(LOG_WARNING, "Could not open codb directory.");
    }

    table_print_footer(&table);
    table_flush(&table);
}

status_t load_codb(uint32 file_no)
{
    status_t    status         = ALL_OK;
    const char* data_path      = os_find_data_path();
    char        file_path[512] = { 0 };
    DIR_t*      d;

    os_snprintf(file_path, sizeof(file_path), "%s/codb", data_path);
    d = os_opendir(file_path);

    if (d)
    {
        struct dirent_t* dir;
        uint32 current_file_no = 1;
        bool_t found           = IS_FALSE;

        while ((dir = os_readdir(d)) != NULL)
        {
            if (os_strstr(dir->d_name, ".json") != NULL)
            {
                if (file_no == current_file_no)
                {
                    const char* data_path = os_find_data_path();

                    if ((0 == os_strcmp(dir->d_name, "ds301.json")) && (ds301 != NULL))
                    {
                        unload_codb();
                        break;
                    }

                    if (NULL == data_path)
                    {
                        os_log(LOG_ERROR, "Data path not found.");
                        return OS_FILE_NOT_FOUND;
                    }

                    os_snprintf(file_path, sizeof(file_path), "%s/codb/%s", data_path, dir->d_name);

                    if (load_codb_ex(file_path) != ALL_OK)
                    {
                        status = CODB_PARSE_ERROR;
                        break;
                    }
                    else
                    {
                        found     = IS_TRUE;
                        active_no = file_no;
                    }
                }
                current_file_no++;
            }
        }
        os_closedir(d);
        if (IS_FALSE == found)
        {
            status = OS_FILE_NOT_FOUND;
            list_codb();
        }
    }
    else
    {
        os_log(LOG_WARNING, "Could not open codb directory.");
    }

    return status;
}

status_t load_codb_ex(char* file_name)
{
    status_t status = ALL_OK;
    FILE_t* file;
    char* file_content;
    size_t   file_size;

    if (NULL == file_name)
    {
        os_log(LOG_ERROR, "File name is NULL.");
        return OS_INVALID_ARGUMENT;
    }

    if (codb != NULL)
    {
        unload_codb();
    }

    os_fix_path(file_name);
    file = os_fopen(file_name, "rb");
    if (NULL == file)
    {
        os_log(LOG_ERROR, "Failed to open file: %s", file_name);
        return OS_FILE_NOT_FOUND;
    }

    os_fseek(file, 0, SEEK_END);
    file_size = os_ftell(file);
    os_fseek(file, 0, SEEK_SET);

    file_content = (char*)os_calloc(file_size + 1, sizeof(char));
    if (NULL == file_content)
    {
        os_log(LOG_ERROR, "Memory allocation failed for file content.");
        os_fclose(file);
        return OS_MEMORY_ALLOCATION_ERROR;
    }

    if (os_fread(file_content, 1, file_size, file) != file_size)
    {
        os_log(LOG_ERROR, "Failed to read file: %s", file_name);
        os_free(file_content);
        os_fclose(file);
        return OS_FILE_READ_ERROR;
    }
    file_content[file_size] = '\0';
    os_fclose(file);

    codb = cJSON_Parse(file_content);
    os_free(file_content);

    if (NULL == codb)
    {
        os_log(LOG_ERROR, "Failed to parse JSON content from file: %s", file_name);
        status = CODB_PARSE_ERROR;
    }

    return status;
}

void unload_codb(void)
{
    if (codb != NULL)
    {
        cJSON_Delete(codb);
        active_no = 0;
        codb      = NULL;
    }
}

static const char* file_name_to_profile_desc(const char* file_name)
{
    size_t i;
    const struct
    {
        const char* file_name;
        const char* description;
    } lookup_table[] = {
        { "ds301.json", "[CiA 301] Application layer and communication" },
        { "ds302.json", "[CiA 302] Programmable CANopen Devices" },
        { "ds401.json", "[CiA 401] I/O devices" },
        { "ds402.json", "[CiA 402] Drives and motion control" },
        { "ds404.json", "[CiA 404] Measurement Devices and Closed Loop Controllers" },
        { "ds405.json", "[CiA 405] IEC 61131-3 programmable devices" },
        { "ds406.json", "[CiA 406] Encoders" },
        { "ds408.json", "[CiA 408] Fluid Power Technology" },
        { "ds410.json", "[CiA 410] Inclinometer" },
        { "ds413.json", "[CiA 413] Truck Gateways" },
        { "ds415.json", "[CiA 415] Road construction machinery" },
        { "ds416.json", "[CiA 416] Building door control" },
        { "ds417.json", "[CiA 417] Lift control systems" },
        { "ds418.json", "[CiA 418] Battery modules" },
        { "ds419.json", "[CiA 419] Battery chargers" },
        { "ds447.json", "[CiA 447] Special-purpose car add-on devices" }
    };

    for (i = 0; i < sizeof(lookup_table) / sizeof(lookup_table[0]); ++i)
    {
        if (0 == os_strncmp(file_name, lookup_table[i].file_name, 10))
        {
            return lookup_table[i].description;
        }
    }

    return file_name;
}
