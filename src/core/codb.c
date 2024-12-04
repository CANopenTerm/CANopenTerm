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
    list_file_type("codb", "json", active_no);
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

    os_fread(file_content, 1, file_size, file);
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
