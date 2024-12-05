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
static cJSON* codb      = NULL;

const char* codb_desc_lookup(uint16 index, uint8 sub_index)
{
    static char desc[128] = { 0 };

    cJSON* object;

    if (NULL == codb)
    {
        return NULL;
    }

    object = NULL;
    cJSON_ArrayForEach(object, codb)
    {
        cJSON* json_index = cJSON_GetObjectItem(object, "index");
        if (json_index != NULL && json_index->valueint == index)
        {
            cJSON* sub_indices = cJSON_GetObjectItem(object, "sub_indices");
            cJSON* obj_desc    = cJSON_GetObjectItem(object, "desc");
            char*  desc_str    = os_strdup(obj_desc->valuestring);

            if (sub_indices != NULL)
            {
                cJSON* sub_index_item = cJSON_GetArrayItem(sub_indices, sub_index);
                if (sub_index_item != NULL)
                {
                    cJSON* sub_desc = cJSON_GetObjectItem(sub_index_item, "desc");
                    if (sub_desc != NULL)
                    {
                        if (0 == os_strcmp(desc_str, (char*)sub_desc->valuestring))
                        {
                            os_snprintf(desc, sizeof(desc), "%s", desc_str);
                        }
                        else
                        {
                            os_snprintf(desc, sizeof(desc), "%s, %s", desc_str, (char*)sub_desc->valuestring);
                        }
                        os_free(desc_str);
                        return (const char*)desc;
                    }
                }
            }

            os_free(desc_str);
        }
    }

    return NULL;
}

bool_t is_codb_loaded(void)
{
    return (codb != NULL) ? IS_TRUE : IS_FALSE;
}

void list_codb(void)
{
    DIR_t*   d            = os_opendir("codb");
    table_t  table        = { DARK_CYAN, DARK_WHITE, 3, 45, 1 };
    status_t status;
    uint32   status_width = 1;

    if (active_no > 0)
    {
        status_width = 6;
    }

    table.column_c_width = status_width;
    status               = table_init(&table, 1024);

    if (ALL_OK != status)
    {
        return;
    }

    if (d)
    {
        struct dirent_t* dir;
        uint32 file_no = 1;

        table_print_header(&table);
        if (0 == active_no)
        {
            table_print_row("No.", "Profile", "-", &table);
        }
        else
        {
            table_print_row("No.", "Profile", "Status", &table);
        }
        table_print_divider(&table);

        while ((dir = os_readdir(d)) != NULL)
        {
            char* extension = ".json";

            if (os_strstr(dir->d_name, extension) != NULL)
            {
                char file_no_str[4] = { 0 };

                os_snprintf(file_no_str, 4, "%3u", file_no);

                if ((active_no > 0) && (active_no == file_no))
                {
                    table_print_row(file_no_str, file_name_to_profile_desc(dir->d_name), "Active", &table);
                }
                else if (0 == active_no)
                {
                    table_print_row(file_no_str, file_name_to_profile_desc(dir->d_name), "-", &table);
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
    status_t status = ALL_OK;
    DIR_t*   d      = os_opendir("codb");

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
                    char file_path[128] = { 0 };
                    os_snprintf(file_path, 128, "codb/%s", dir->d_name);

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

status_t load_codb_ex(const char* file_name)
{
    status_t status = ALL_OK;
    FILE_t*  file;
    char*    file_content;
    size_t   file_size;

    if (NULL == file_name)
    {
        return status;
    }

    if (codb != NULL)
    {
        (void)unload_codb();
    }

    file = fopen(file_name, "r");
    if (NULL == file)
    {
        return OS_FILE_NOT_FOUND;
    }

    os_fseek(file, 0, SEEK_END);
    file_size = os_ftell(file);
    os_fseek(file, 0, SEEK_SET);

    file_content = (char*)os_calloc(file_size + 1, sizeof(char));
    if (NULL == file_content)
    {
        os_fclose(file);
        return OS_MEMORY_ALLOCATION_ERROR;
    }

    if (os_fread(file_content, 1, file_size, file) != file_size)
    {
        os_free(file_content);
        os_fclose(file);
        return OS_FILE_READ_ERROR;
    }
    file_content[file_size] = '\0';
    os_fclose(file);

    codb = cJSON_Parse(file_content);
    os_free(file_content);

    if (codb == NULL)
    {
        status = CODB_PARSE_ERROR;
    }

    return status;
}

/* When declared and defined as a void-function, compilation fails.  Why?!
 * C2371: 'unload_codb': redefinition; different basic types
 */
status_t unload_codb(void)
{
    if (codb != NULL)
    {
        cJSON_Delete(codb);
        active_no = 0;
        codb      = NULL;
    }

    return ALL_OK;
}

static const char* file_name_to_profile_desc(const char* file_name)
{
    size_t i;
    const struct {
        const char* file_name;
        const char* description;
    } lookup_table[] = {
        { "ds301.json", "[CiA 301] Application layer and communication" },
        { "ds401.json", "[CiA 401] I/O devices" },
        { "ds402.json", "[CiA 402] Drives and motion control" },
        { "ds405.json", "[CiA 405] IEC 61131-3 programmable devices" },
        { "ds406.json", "[CiA 406] Encoders" },
        { "ds408.json", "[CiA 408] Fluid Power Technology" },
        { "ds410.json", "[CiA 410] Inclinometer" },
        { "ds415.json", "[CiA 415] Road construction machinery" },
        { "ds416.json", "[CiA 416] Building door control" },
        { "ds418.json", "[CiA 418] Battery modules" },
        { "ds419.json", "[CiA 419] Battery chargers" },
    };

    for (i = 0; i < sizeof(lookup_table) / sizeof(lookup_table[0]); ++i)
    {
        if (0 == os_strcmp(file_name, lookup_table[i].file_name))
        {
            return lookup_table[i].description;
        }
    }

    return file_name;
}
