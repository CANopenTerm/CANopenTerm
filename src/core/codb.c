/** @file codb.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "cJSON.h"
#include "common.h"
#include "core.h"
#include "os.h"

static uint32 active_no = 0;
static cJSON* codb      = NULL;

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
                        active_no = file_no;
                    }
                }
                current_file_no++;
            }
        }
        os_closedir(d);
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
