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

static uint64 generate_hash(const unsigned char* name);

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
    num_results = num_results + 1;
    results     = (junit_result_t**)os_realloc(results, sizeof(junit_result_t*) * (num_results));

    if (NULL == results)
    {
        os_log(LOG_ERROR, "Not enough memory to add new test result.");
        junit_clear_results();
    }
    else
    {
        results[num_results - 1] = (junit_result_t*)os_calloc(1, sizeof(junit_result_t));
        if (NULL == results[num_results - 1])
        {
            os_log(LOG_ERROR, "Not enough memory to allocate for new test result.");
            junit_clear_results();
        }
        else
        {
            os_memcpy(results[num_results - 1], result, sizeof(junit_result_t));
        }
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

    if (NULL == filename)
    {
        filename = "test_report.xml";
    }

    file = os_fopen(filename, "w+");
    if (NULL == file)
    {
        status = OS_FILE_NOT_FOUND;
    }
    else
    {
        float  total_time = 0.f;
        uint32 i;

        for (i = 0; i < num_results; i++)
        {
            char testsuit_name[512] = { 0 };

            if (NULL == results[i]->package)
            {
                results[i]->package = "Tests";
            }

            if (NULL == results[i]->class_name)
            {
                results[i]->class_name = "Generic";
            }

            os_snprintf(testsuit_name, sizeof(testsuit_name), "%s.%s", results[i]->package, results[i]->class_name);

            total_time = total_time + results[i]->time;
            results[i]->testsuite_name_hash = generate_hash((const unsigned char*)testsuit_name);
        }

        /* Sort results by testsuite name. */
        for (i = 0; i < num_results; i++)
        {
            uint32 j;
            for (j = i + 1; j < num_results; j++)
            {
                if (results[i]->testsuite_name_hash > results[j]->testsuite_name_hash)
                {
                    junit_result_t* temp = results[i];
                    results[i]           = results[j];
                    results[j]           = temp;
                }
            }
        }

        os_fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        os_fprintf(file, "<testsuites time=\"%.6f\">\n", total_time);

        /* Group test cases by testsuite and output them. */
        uint32 current_suite_start = 0;
        while (current_suite_start < num_results)
        {
            uint32 current_suite_end = current_suite_start;
            float  suite_time        = 0.f;

            while (current_suite_end < num_results &&
                results[current_suite_end]->testsuite_name_hash == results[current_suite_start]->testsuite_name_hash)
            {
                suite_time += results[current_suite_end]->time;
                current_suite_end += 1;
            }

            os_fprintf(file, "    <testsuite name=\"%s.%s\" time=\"%.6f\">\n",
                results[current_suite_start]->package,
                results[current_suite_start]->class_name,
                suite_time);

            for (i = current_suite_start; i < current_suite_end; i++)
            {
                os_fprintf(file, "        <testcase name=\"%s\" classname=\"%s.%s\" time=\"%.6f\">\n",
                    results[i]->test_name,
                    results[i]->package,
                    results[i]->class_name,
                    results[i]->time);

                if (IS_FALSE == results[i]->has_passed)
                {
                    if (NULL == results[i]->error_type)
                    {
                        results[i]->error_type = "AssertionError";
                    }

                    if (NULL == results[i]->error_message)
                    {
                        results[i]->error_message = "No error message provided";
                    }

                    if (NULL == results[i]->call_stack)
                    {
                        results[i]->call_stack = "<!-- No call stack provided. -->";
                    }

                    os_fprintf(file, "            <failure message=\"%s\" type=\"%s\">\n",
                        results[i]->error_message,
                        results[i]->error_type);
                    os_fprintf(file, "                %s\n", results[i]->call_stack);
                    os_fprintf(file, "            </failure>\n");
                }
                os_fprintf(file, "        </testcase>\n");
            }
            os_fprintf(file, "    </testsuite>\n");

            current_suite_start = current_suite_end;
        }
        os_fprintf(file, "</testsuites>\n");
        os_fclose(file);
    }

    return status;
}

/* djb2 by Dan Bernstein
 * http://www.cse.yorku.ca/~oz/hash.html
 */
static uint64 generate_hash(const unsigned char* name)
{
    uint64 hash = 5381;
    uint32 c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}
