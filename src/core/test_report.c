/** @file test_report.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "test_report.h"
#include "core.h"
#include "os.h"

static uint32          num_results = 0;
static test_result_t** results     = NULL;

static uint64 generate_hash(const unsigned char* name);

status_t test_init(void)
{
    status_t status = ALL_OK;

    if (NULL == results)
    {
        results = (test_result_t**)os_calloc(sizeof(test_result_t*), 1);
        if (NULL == results)
        {
            status = OS_MEMORY_ALLOCATION_ERROR;
        }
    }

    return status;
}

void test_add_result(test_result_t* result)
{
    num_results = num_results + 1;
    results     = (test_result_t**)os_realloc(results, sizeof(test_result_t*) * (num_results));

    if (NULL == results)
    {
        os_log(LOG_ERROR, "Not enough memory to add new test result.");
        test_clear_results();
    }
    else
    {
        results[num_results - 1] = (test_result_t*)os_calloc(1, sizeof(test_result_t));
        if (NULL == results[num_results - 1])
        {
            os_log(LOG_ERROR, "Not enough memory to allocate for new test result.");
            test_clear_results();
        }
        else
        {
            results[num_results - 1]->has_passed          = result->has_passed;
            results[num_results - 1]->testsuite_name_hash = result->testsuite_name_hash;
            results[num_results - 1]->time                = result->time;

            if (result->package)
            {
                results[num_results - 1]->package = os_strdup(result->package);
            }
            if (result->class_name)
            {
                results[num_results - 1]->class_name = os_strdup(result->class_name);
            }
            if (result->test_name)
            {
                results[num_results - 1]->test_name = os_strdup(result->test_name);
            }
            if (result->error_type)
            {
                results[num_results - 1]->error_type = os_strdup(result->error_type);
            }
            if (result->error_message)
            {
                results[num_results - 1]->error_message = os_strdup(result->error_message);
            }
            if (result->call_stack)
            {
                results[num_results - 1]->call_stack = os_strdup(result->call_stack);
            }
        }
    }
}

void test_clear_results(void)
{
    uint32 i;
    for (i = 0; i < num_results; i = i + 1)
    {
        if (results[i]->package != NULL)
        {
            os_free((void*)results[i]->package);
        }
        if (results[i]->class_name != NULL)
        {
            os_free((void*)results[i]->class_name);
        }
        if (results[i]->test_name)
        {
            os_free((void*)results[i]->test_name);
        }
        if (results[i]->error_type)
        {
            os_free((void*)results[i]->error_type);
        }
        if (results[i]->error_message)
        {
            os_free((void*)results[i]->error_message);
        }
        if (results[i]->call_stack)
        {
            os_free((void*)results[i]->call_stack);
        }
        os_free(results[i]);
    }
    os_free(results);
    results     = NULL;
    num_results = 0;
}

status_t test_generate_report(const char* file_name)
{
    status_t status = ALL_OK;
    FILE_t*  file;

    if (NULL == file_name)
    {
        file_name = "test_report.xml";
    }

    file = os_fopen(file_name, "w+");
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
            char testsuit_name[512] = {0};

            if (NULL == results[i]->package)
            {
                results[i]->package = os_strdup("Tests");
            }

            if (NULL == results[i]->class_name)
            {
                results[i]->class_name = os_strdup("Generic");
            }

            os_snprintf(testsuit_name, sizeof(testsuit_name), "%s.%s", results[i]->package, results[i]->class_name);

            total_time                      = total_time + results[i]->time;
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
                    test_result_t* temp = results[i];
                    results[i]          = results[j];
                    results[j]          = temp;
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
                if (NULL == results[i]->test_name)
                {
                    results[i]->test_name = os_strdup("Test");
                }

                os_fprintf(file, "        <testcase name=\"%s\" classname=\"%s.%s\" time=\"%.6f\">\n",
                           results[i]->test_name,
                           results[i]->package,
                           results[i]->class_name,
                           results[i]->time);

                if (IS_FALSE == results[i]->has_passed)
                {
                    if (NULL == results[i]->error_type)
                    {
                        results[i]->error_type = os_strdup("AssertionError");
                    }

                    if (NULL == results[i]->error_message)
                    {
                        results[i]->error_message = os_strdup("No error message provided");
                    }

                    if (NULL == results[i]->call_stack)
                    {
                        results[i]->call_stack = os_strdup("<!-- No call stack provided. -->");
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

    if (ALL_OK == status)
    {
        test_clear_results();
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
