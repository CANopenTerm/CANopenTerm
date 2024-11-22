/** @file junit.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "junit.h"
#include "os.h"

static uint32           num_results = 0;
static junit_result_t** results     = NULL;

status_t junit_init(void)
{
    status_t status = ALL_OK;

    if (NULL == results)
    {
        results = (junit_result_t**)os_calloc(sizeof(junit_result_t*), 1);
        if (NULL == results)
        {
            status = OS_MEMORY_ALLOCATION_ERROR;
        }
    }

    return status;
}

void junit_add_result(junit_result_t* result)
{
    results = (junit_result_t**)os_realloc(results, sizeof(junit_result_t*) * (num_results + 1));

    if (NULL == results)
    {
        os_log(LOG_ERROR, "Not enough memory to add new test result.");
        junit_clear_results();
    }
    else
    {
        num_results = num_results + 1;
        os_memcpy(results[num_results - 1], result, sizeof(junit_result_t));
    }
}

void junit_clear_results(void)
{
    uint32 i;
    for (i = 0; i < num_results; i = i + 1)
    {
        os_free(results[i]);
    }
    os_free(results);
    results = NULL;
    num_results = 0;
}

status_t junit_generate_report(const char* filename)
{
    status_t status = ALL_OK;
    FILE_t*  file;

    file = os_fopen(filename, "w+");
    if (NULL == file)
    {
        status = OS_FILE_NOT_FOUND;
    }
    else
    {
        uint32 i;

        os_fprintf(file, "<testsuites>\n");

        for (i = 0; i < num_results; i++)
        {
            junit_result_t* result = results[i];

            if (NULL == result->package)
            {
                result->package = "Tests";
            }

            if (NULL == result->class_name)
            {
                result->class_name = "Generic";
            }

            os_fprintf(file, "    <testsuite name=\"%s.%s\" time=\"%.6f\">\n", result->package, result->class_name, result->time);
            os_fprintf(file, "        <testcase name=\"%s\" classname=\"%s\" time=\"%.3f\" />\n", result->test_name, result->class_name, result->time);

            if (IS_FALSE == result->has_passed)
            {
                if (NULL == result->error_message)
                {
                    result->error_message = "No error message provided";
                }

                if (NULL == result->error_type)
                {
                    result->error_type = "AssertionError";
                }

                if (NULL == result->call_stack)
                {
                    result->call_stack = "<!-- No call stack provided. -->";
                }

                os_fprintf(file, "            <failure message=\"%s\" type=\"%s\">\n", result->error_message, result->error_type);
                os_fprintf(file, "                %s\n", result->call_stack);
                os_fprintf(file, "            </failure>\n");
            }

            os_fprintf(file, "    </testsuite>\n");
        }

        os_fprintf(file, "</testsuites>\n");
        os_fclose(file);
    }

    return status;
}
