/** @file dbc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "dbc.h"
#include "core.h"
#include "os.h"

static dbc_t* dbc;

static uint64 extract_raw_signal(uint64 can_frame, uint8 start_bit, uint8 length, endian_t endianness);
static void parse_message_line(char* line, message_t* message);
static void parse_signal_line(char* line, signal_t* signal);
static bool starts_with(const char* str, const char* prefix);
static char* str_tolower(const char* str);
static char* trim_whitespace(char* str);

const char* dbc_decode(uint32 can_id, uint64 data)
{
    static char result[4096] = {0};
    int pos = 0;
    int i;

    if (NULL == dbc)
    {
        return "";
    }

    for (i = 0; i <= dbc->message_count; ++i)
    {
        if (dbc->messages[i].id == can_id)
        {
            message_t* msg = &dbc->messages[i];
            int n = os_snprintf(result + pos, sizeof(result) - pos, "%s (%Xh)\n", msg->name, msg->id);
            int j;

            if (n > 0)
            {
                pos += n;
            }

            for (j = 0; j < msg->signal_count; ++j)
            {
                signal_t* signal = &msg->signals[j];
                uint64 raw_value = extract_raw_signal(data, signal->start_bit, signal->length, signal->endianness);
                double value = (raw_value * signal->scale) + signal->offset;
                int k;

                n = os_snprintf(result + pos, sizeof(result) - pos, "  %s", signal->name);
                if (n > 0)
                {
                    pos += n;
                }

                for (k = 0; k <= 35 - os_strlen(signal->name); ++k)
                {
                    n = os_snprintf(result + pos, sizeof(result) - pos, " ");
                    if (n > 0)
                    {
                        pos += n;
                    }
                }

                n = os_snprintf(result + pos, sizeof(result) - pos, ": %f %s\n", value, signal->unit);
                if (n > 0)
                {
                    pos += n;
                }
            }
            result[sizeof(result) - 1] = '\0';
            break;
        }
    }

    if (0 == pos)
    {
        return "";
    }

    return result;
}

status_t dbc_find_id_by_name(uint32* id, const char* search)
{
    int i;

    if ((NULL == dbc) || (NULL == search) || (NULL == id))
    {
        return OS_INVALID_ARGUMENT;
    }

    char* lower_search = str_tolower(search);
    if (NULL == lower_search)
    {
        return OS_MEMORY_ALLOCATION_ERROR;
    }

    for (i = 0; i < dbc->message_count; ++i)
    {
        char* lower_name = str_tolower(dbc->messages[i].name);
        if (NULL == lower_name)
        {
            os_free(lower_search);
            return OS_MEMORY_ALLOCATION_ERROR;
        }

        if (os_strstr(lower_name, lower_search) != NULL)
        {
            *id = dbc->messages[i].id;
            os_free(lower_name);
            os_free(lower_search);
            return ALL_OK;
        }

        os_free(lower_name);
    }

    os_free(lower_search);
    return ITEM_NOT_FOUND;
}

status_t dbc_load(char* filename)
{
    FILE_t* file;
    char line[1024] = {0};
    message_t* current_message = NULL;

    dbc_unload();

    if (NULL == dbc)
    {
        dbc = os_calloc(1, sizeof(dbc_t));
        if (NULL == dbc)
        {
            return OS_MEMORY_ALLOCATION_ERROR;
        }
    }
    else
    {
        dbc->message_count = 0;
        dbc->messages = NULL;
    }

    os_fix_path(filename);
    file = os_fopen(filename, "r");

    if (NULL == file)
    {
        return OS_FILE_NOT_FOUND;
    }

    while (os_fgets(line, sizeof(line), file) != NULL)
    {
        char* trimmed_line = trim_whitespace(line);

        if (starts_with(trimmed_line, "BO_ "))
        {
            message_t* temp = os_realloc(dbc->messages, sizeof(message_t) * (dbc->message_count + 1));
            if (NULL == temp)
            {
                os_fclose(file);
                dbc_unload();
                return OS_MEMORY_ALLOCATION_ERROR;
            }

            dbc->messages = temp;
            current_message = &dbc->messages[dbc->message_count];
            os_memset(current_message, 0, sizeof(message_t));

            dbc->message_count += 1;
            parse_message_line(trimmed_line, current_message);
        }
        else if (starts_with(trimmed_line, "SG_ ") && current_message != NULL)
        {
            signal_t* temp = os_realloc(current_message->signals, sizeof(signal_t) * (current_message->signal_count + 1));
            if (NULL == temp)
            {
                os_fclose(file);
                dbc_unload();
                return OS_MEMORY_ALLOCATION_ERROR;
            }

            current_message->signals = temp;
            signal_t* current_signal = &current_message->signals[current_message->signal_count];
            os_memset(current_signal, 0, sizeof(signal_t));

            current_message->signal_count += 1;
            parse_signal_line(trimmed_line, current_signal);
        }
    }

    os_fclose(file);
    return ALL_OK;
}

void dbc_print(void)
{
    int i, j;

    if (NULL == dbc)
    {
        return;
    }

    os_printf("DBC File contains %d messages\n", dbc->message_count);
    for (i = 0; i < dbc->message_count; ++i)
    {
        const message_t* msg = &dbc->messages[i];
        os_printf("Message %d: ID=%u, Name=%s, DLC=%u, Transmitter=%s\n",
                  i + 1, msg->id, msg->name, msg->dlc, msg->transmitter);
        os_printf("  Contains %d signals\n", msg->signal_count);
        for (j = 0; j < msg->signal_count; ++j)
        {
            const signal_t* sig = &msg->signals[j];
            os_printf("  Signal %d: Name=%s, StartBit=%d, Length=%d, Endianness=%d, Scale=%.6f, Offset=%.2f, Min=%.2f, Max=%.2f, Unit=%s, Receiver=%s\n",
                      j + 1, sig->name, sig->start_bit, sig->length, sig->endianness, sig->scale, sig->offset, sig->min_value, sig->max_value, sig->unit, sig->receiver);
        }
    }
}

void dbc_unload(void)
{
    int i, j;

    if (NULL == dbc)
    {
        return;
    }

    for (i = 0; i < dbc->message_count; ++i)
    {
        message_t* msg = &dbc->messages[i];

        os_free(msg->name);
        os_free(msg->transmitter);

        for (j = 0; j < msg->signal_count; ++j)
        {
            signal_t* sig = &msg->signals[j];
            os_free(sig->name);
            os_free(sig->unit);
            os_free(sig->receiver);
        }

        os_free(msg->signals);
    }

    os_free(dbc->messages);
    dbc->message_count = 0;
    dbc->messages = NULL;
}

static uint64 extract_raw_signal(uint64 can_frame, uint8 start_bit, uint8 length, endian_t endianness)
{
    uint64 mask = (1ULL << length) - 1;
    uint64 raw_value;

    if (ENDIANNESS_MOTOROLA == endianness)
    {
        int bit_pos = 64 - (start_bit + length);
        raw_value = (can_frame >> bit_pos) & mask;
    }
    else
    {
        raw_value = (can_frame >> start_bit) & mask;
    }

    return raw_value;
}

static void parse_message_line(char* line, message_t* message)
{
    char* token;
    char* rest = line;

    os_strtokr_r(rest, " ", &rest);

    token = os_strtokr_r(rest, " ", &rest);
    message->id = os_strtoul(token, NULL, 10) & 0x7FFFFFFF;
    token = os_strtokr_r(rest, ":", &rest);
    message->name = os_strdup(token);
    token = os_strtokr_r(rest, " ", &rest);
    message->dlc = os_atoi(token);
    message->transmitter = os_strdup(trim_whitespace(rest));
}

static void parse_signal_line(char* line, signal_t* signal)
{
    char* token;
    char* rest = line;

    signal->name = NULL;
    signal->unit = NULL;
    signal->receiver = NULL;
    signal->start_bit = 0;
    signal->length = 0;
    signal->scale = 1.0;
    signal->offset = 0.0;
    signal->min_value = 0.0;
    signal->max_value = 0.0;
    signal->endianness = 0;

    os_strtokr_r(rest, " ", &rest);

    token = os_strtokr_r(rest, " ", &rest);
    if (token != NULL)
    {
        signal->name = os_strdup(token);
    }

    token = os_strtokr_r(rest, "|", &rest);
    if (token != NULL)
    {
        signal->start_bit = os_atoi(token);
        token = os_strtokr_r(rest, "@", &rest);
        if (token != NULL)
        {
            signal->length = os_atoi(token);
            if (*rest != '\0')
            {
                signal->endianness = os_atoi(rest);
                rest++;
            }
        }
    }

    token = os_strtokr_r(rest, "(", &rest);
    if (token != NULL)
    {
        token = os_strtokr_r(rest, ",", &rest);
        if (token != NULL)
        {
            signal->scale = os_atof(token);
            token = os_strtokr_r(rest, ")", &rest);
            if (token != NULL)
            {
                signal->offset = os_atof(token);
            }
        }
    }

    token = os_strtokr_r(rest, "[", &rest);
    if (token != NULL)
    {
        token = os_strtokr_r(rest, "|", &rest);
        if (token != NULL)
        {
            signal->min_value = os_atof(token);
            token = os_strtokr_r(rest, "]", &rest);
            if (token != NULL)
            {
                signal->max_value = os_atof(token);
            }
        }
    }

    os_strtokr_r(rest, "\"", &rest);
    if (rest[0] == '\"')
    {
        signal->unit = os_strdup("");
        rest++;
    }
    else
    {
        char* end = os_strtokr_r(rest, "\"", &rest);
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

    token = os_strtokr_r(rest, " ", &rest);
    if (token != NULL)
    {
        signal->receiver = os_strdup(token);
    }
    else
    {
        signal->receiver = os_strdup("");
    }
}

static bool starts_with(const char* str, const char* prefix)
{
    bool status;
    size_t len_prefix = os_strlen(prefix);
    size_t len_str = os_strlen(str);

    if (len_str < len_prefix)
    {
        status = false;
    }
    else
    {
        status = (bool)(os_strncmp(prefix, str, len_prefix) == 0);
    }

    return status;
}

static char* str_tolower(const char* str)
{
    int i;
    char* lower_str;

    if (NULL == str)
    {
        return NULL;
    }

    lower_str = os_strdup(str);
    if (NULL == lower_str)
    {
        return NULL;
    }

    for (i = 0; lower_str[i]; i++)
    {
        lower_str[i] = os_tolower((unsigned char)lower_str[i]);
    }

    return lower_str;
}

static char* trim_whitespace(char* str)
{
    char* end;

    if (NULL == str)
    {
        return "";
    }

    while (os_isspace((unsigned char)*str))
    {
        str++;
    }

    if (0 == *str)
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
