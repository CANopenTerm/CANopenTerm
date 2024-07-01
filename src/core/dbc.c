/** @file dbc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "dbc.h"
#include "os.h"

static void   parse_message_line(char *line, message_t *message);
static void   parse_signal_line(char *line, signal_t *signal);
static bool_t starts_with(const char *str, const char *prefix);
static char*  trim_whitespace(char *str);

void dbc_deinit(dbc_t *dbc)
{
    int i, j;

    if (NULL == dbc)
    {
        return;
    }

    for (i = 0; i < dbc->message_count; ++i)
    {
        message_t *msg = &dbc->messages[i];

        if (msg->name)
        {
            os_free(msg->name);
        }

        if (msg->transmitter)
        {
            os_free(msg->transmitter);
        }

        for (j = 0; j < msg->signal_count; ++j)
        {
            signal_t *sig = &msg->signals[j];

            os_free(sig->name);
            os_free(sig->unit);
            os_free(sig->receiver);

        }

        if (msg->signals)
        {
            os_free(msg->signals);
        }
    }

    if (dbc->messages)
    {
        os_free(dbc->messages);
    }

    dbc->messages      = NULL;
    dbc->message_count = 0;
}

status_t dbc_init(dbc_t **dbc)
{
    *dbc = os_calloc(1, sizeof(dbc_t));
    if (NULL == *dbc)
    {
        return OS_MEMORY_ALLOCATION_ERROR;
    }
    else
    {
        return ALL_OK;
    }
}

status_t dbc_load(dbc_t* dbc, const char *filename)
{
    char       line[1024]      = { 0 };
    FILE*      file            = os_fopen(filename, "r");
    message_t* current_message = NULL;

    if (NULL == file)
    {
        return OS_FILE_NOT_FOUND;
    }

    if (NULL == dbc)
    {
        os_fclose(file);
        return OS_INVALID_ARGUMENT;
    }

    while (os_fgets(line, sizeof(line), file) != NULL)
    {
        char *trimmed_line = trim_whitespace(line);

        if (starts_with(trimmed_line, "BO_ "))
        {
            message_t* temp = os_realloc(dbc->messages, sizeof(message_t) * (dbc->message_count + 1));
            if (NULL == temp)
            {
                os_fclose(file);
                return OS_MEMORY_ALLOCATION_ERROR;
            }

            dbc->messages   = temp;
            current_message = &dbc->messages[dbc->message_count];
            os_memset(current_message, 0, sizeof(message_t));

            if (current_message != NULL)
            {
                dbc->message_count += 1;
                parse_message_line(trimmed_line, current_message);
            }
        }
        else if (starts_with(trimmed_line, "SG_ "))
        {
            if (current_message != NULL)
            {
                signal_t* current_signal;
                current_message->signals = realloc(current_message->signals, sizeof(signal_t) * (current_message->signal_count + 1));
                current_message->signal_count += 1;
                current_signal = &current_message->signals[current_message->signal_count - 1];
                parse_signal_line(trimmed_line, current_signal);
            }
        }
    }

    os_fclose(file);
    return ALL_OK;
}

void dbc_print(const dbc_t *dbc)
{
    int i, j;

    if (NULL == dbc)
    {
        return;
    }

    os_printf("DBC File contains %d messages\n", dbc->message_count);
    for (i = 0; i < dbc->message_count; ++i)
    {
        const message_t *msg = &dbc->messages[i];
        os_printf("Message %d: ID=%u, Name=%s, DLC=%u, Transmitter=%s\n",
            i + 1, msg->id, msg->name, msg->dlc, msg->transmitter);
        os_printf("  Contains %d signals\n", msg->signal_count);
        for (j = 0; j < msg->signal_count; ++j)
        {
            const signal_t *sig = &msg->signals[j];
            os_printf("  Signal %d: Name=%s, StartBit=%d, Length=%d, Scale=%.6f, Offset=%.2f, Min=%.2f, Max=%.2f, Unit=%s, Receiver=%s\n",
                j + 1, sig->name, sig->start_bit, sig->length, sig->scale, sig->offset, sig->min_value, sig->max_value, sig->unit, sig->receiver);
        }
    }
}

static void parse_message_line(char *line, message_t *message)
{
    char *token;
    char *rest = line;

    token                = os_strtokr(rest, " ", &rest);
    message->id          = os_strtoul(token, NULL, 10);
    token                = os_strtokr(rest, ":", &rest);
    message->name        = os_strdup(token);
    token                = os_strtokr(rest, " ", &rest);
    message->dlc         = os_atoi(token);
    message->transmitter = os_strdup(trim_whitespace(rest));
}

void parse_signal_line(char *line, signal_t *signal)
{
    char* token;
    char* rest = line;

    signal->name      = NULL;
    signal->unit      = NULL;
    signal->receiver  = NULL;
    signal->start_bit = 0;
    signal->length    = 0;
    signal->scale     = 1.0;
    signal->offset    = 0.0;
    signal->min_value = 0.0;
    signal->max_value = 0.0;

    token = os_strtokr(rest, " ", &rest);
    token = os_strtokr(rest, " ", &rest);
    if (token != NULL)
    {
        signal->name = os_strdup(token);
    }

    token = os_strtokr(rest, "|", &rest);
    if (token != NULL)
    {
        signal->start_bit = os_atoi(token);
        token             = os_strtokr(rest, "@", &rest);
        if (token != NULL)
        {
            signal->length = os_atoi(token);
        }
    }

    token = os_strtokr(rest, "(", &rest);
    if (token != NULL)
    {
        token = os_strtokr(rest, ",", &rest);
        if (token != NULL)
        {
            signal->scale = os_atof(token);
            token         = os_strtokr(rest, ")", &rest);
            if (token != NULL)
            {
                signal->offset = os_atof(token);
            }
        }
    }

    token = os_strtokr(rest, "[", &rest);
    if (token != NULL)
    {
        token = os_strtokr(rest, "|", &rest);
        if (token != NULL)
        {
            signal->min_value = os_atof(token);
            token             = os_strtokr(rest, "]", &rest);
            if (token != NULL)
            {
                signal->max_value = os_atof(token);
            }
        }
    }

    token = os_strtokr(rest, "\"", &rest);
    if (rest[0] == '\"') // Check if the next character is a quote, indicating an empty unit
    {
        signal->unit = os_strdup("");
        rest++; // Move past the second quote
    }
    else
    {
        char* end = os_strtokr(rest, "\"", &rest);
        if (end != NULL)
        {
            signal->unit = os_strdup(end);
        }
        else
        {
            signal->unit = os_strdup("");
        }
    }

    while (*rest == ' ')
    {
        rest++;
    }

    token = os_strtokr(rest, " ", &rest);
    if (token != NULL)
    {
        signal->receiver = os_strdup(token);
    }
    else
    {
        signal->receiver = os_strdup("");
    }
}

static bool_t starts_with(const char *str, const char *prefix)
{
    bool_t status;
    size_t len_prefix = os_strlen(prefix);
    size_t len_str    = os_strlen(str);

    if (len_str < len_prefix)
    {
        status = IS_FALSE;
    }
    else
    {
        status = (bool_t)(os_strncmp(prefix, str, len_prefix) == 0);
    }

    return status;
}

static char* trim_whitespace(char *str)
{
    char *end;

    if (NULL == str)
    {
        return "";
    }

    while (os_isspace((unsigned char)*str))
    {
        str++;
    }

    if (0  == *str)
    {
        return str;
    }

    end = str + os_strlen(str) - 1;
    while ((end > str) && (os_isspace((unsigned char)*end)))
    {
        end--;
    }

    *(end + 1) = 0;

    return str;
}
