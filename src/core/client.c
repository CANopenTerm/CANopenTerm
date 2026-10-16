/** @file client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdio.h>

#include "client.h"
#include "command.h"
#include "os.h"

#define CLIENT_CONNECT_TIMEOUT_MS 5000
#define CLIENT_HANDSHAKE_TIMEOUT_MS 5000
#define CLIENT_RAW_SIZE 32768
#define CLIENT_KEY_BATCH 64

#define WS_OP_CONTINUATION 0x0
#define WS_OP_TEXT 0x1
#define WS_OP_BINARY 0x2
#define WS_OP_CLOSE 0x8
#define WS_OP_PING 0x9
#define WS_OP_PONG 0xA

typedef enum session_state
{
    SESSION_RUNNING,
    SESSION_ESCAPED,
    SESSION_CLOSED,
    SESSION_ERROR

} session_state_t;

static uint8 raw[CLIENT_RAW_SIZE];
static size_t raw_len = 0;
static uint32 rng_state = 0;

static os_net_socket* open_socket(const char* host, uint16 port);
static bool do_handshake(os_net_socket* socket, const char* host, uint16 port);
static session_state_t run_session(os_net_socket* socket);
static session_state_t forward_keys(os_net_socket* socket);
static session_state_t receive_frames(os_net_socket* socket);
static bool send_frame(os_net_socket* socket, uint8 opcode, const uint8* payload, size_t len);
static uint8 next_random(void);
static void base64_encode(const uint8* data, size_t len, char* out);

status_t client_connect(const char* host, uint16 port)
{
    os_net_socket* socket;
    session_state_t state;

    if (false == os_net_init())
    {
        os_log(LOG_ERROR, "Unable to initialise network: %s", os_get_error());
        return OS_INIT_ERROR;
    }

    socket = open_socket(host, port);
    if (NULL == socket)
    {
        os_net_quit();
        return OS_INIT_ERROR;
    }

    raw_len = 0;
    rng_state = (uint32)os_get_ticks() | 1u;

    if (false == do_handshake(socket, host, port))
    {
        os_net_socket_destroy(socket);
        os_net_quit();
        return OS_INIT_ERROR;
    }

    os_log(LOG_INFO, "Connected to ws://%s:%u.  Press q or Ctrl+] to disconnect.", host, (unsigned int)port);

    if (false == os_console_raw_enter())
    {
        os_log(LOG_WARNING, "Console is not interactive, input may not be forwarded correctly.");
    }

    state = run_session(socket);

    if (SESSION_ESCAPED == state)
    {
        send_frame(socket, WS_OP_CLOSE, NULL, 0);
        os_delay(50);
    }

    os_console_raw_leave();
    os_net_socket_destroy(socket);
    os_net_quit();

    os_printf("\r\n");
    os_log(LOG_INFO, "Disconnected.");

    return (SESSION_ERROR == state) ? OS_INIT_ERROR : ALL_OK;
}

static os_net_socket* open_socket(const char* host, uint16 port)
{
    os_net_address* address;
    os_net_socket* socket;

    address = os_net_resolve(host);
    if ((NULL == address) || (os_net_wait_resolved(address, CLIENT_CONNECT_TIMEOUT_MS) <= 0))
    {
        os_log(LOG_ERROR, "Unable to resolve host '%s': %s", host, os_get_error());

        if (NULL != address)
        {
            os_net_unref_address(address);
        }

        return NULL;
    }

    socket = os_net_client_create(address, port, 0);
    os_net_unref_address(address);

    if (NULL == socket)
    {
        os_log(LOG_ERROR, "Unable to connect to %s:%u: %s", host, (unsigned int)port, os_get_error());
        return NULL;
    }

    if (os_net_wait_connected(socket, CLIENT_CONNECT_TIMEOUT_MS) <= 0)
    {
        os_log(LOG_ERROR, "Unable to connect to %s:%u: %s", host, (unsigned int)port, os_get_error());
        os_net_socket_destroy(socket);
        return NULL;
    }

    return socket;
}

static bool do_handshake(os_net_socket* socket, const char* host, uint16 port)
{
    uint8 key_bytes[16];
    char key[32];
    char request[512];
    char* end = NULL;
    int request_len;
    int i;
    uint64 start;

    for (i = 0; i < 16; i++)
    {
        key_bytes[i] = next_random();
    }
    base64_encode(key_bytes, sizeof(key_bytes), key);

    request_len = os_snprintf(request,
                              sizeof(request),
                              "GET / HTTP/1.1\r\n"
                              "Host: %s:%u\r\n"
                              "Upgrade: websocket\r\n"
                              "Connection: Upgrade\r\n"
                              "Sec-WebSocket-Key: %s\r\n"
                              "Sec-WebSocket-Version: 13\r\n\r\n",
                              host,
                              (unsigned int)port,
                              key);
    if ((request_len <= 0) || ((size_t)request_len >= sizeof(request)))
    {
        os_log(LOG_ERROR, "Host name too long.");
        return false;
    }

    if (false == os_net_write(socket, request, request_len))
    {
        os_log(LOG_ERROR, "Unable to send handshake: %s", os_get_error());
        return false;
    }

    start = os_get_ticks();
    while (NULL == end)
    {
        int received;

        if (os_get_ticks() - start > (uint64)CLIENT_HANDSHAKE_TIMEOUT_MS * 1000000u)
        {
            os_log(LOG_ERROR, "Handshake timed out.");
            return false;
        }

        if (raw_len >= CLIENT_RAW_SIZE - 1)
        {
            os_log(LOG_ERROR, "Handshake response too large.");
            return false;
        }

        received = os_net_read(socket, raw + raw_len, (int)(CLIENT_RAW_SIZE - 1 - raw_len));
        if (received < 0)
        {
            os_log(LOG_ERROR, "Connection lost during handshake: %s", os_get_error());
            return false;
        }
        raw_len += (size_t)received;
        raw[raw_len] = 0;

        end = os_strstr((char*)raw, "\r\n\r\n");
        if (NULL == end)
        {
            os_delay(10);
        }
    }

    if (0 != os_strncmp((char*)raw, "HTTP/1.1 101", 12))
    {
        os_log(LOG_ERROR, "Server is not a CANopenTerm WebSocket server.");
        return false;
    }

    /* Keep frames that arrived together with the response. */
    i = (int)(end - (char*)raw) + 4;
    raw_len -= (size_t)i;
    os_memmove(raw, raw + i, raw_len);

    return true;
}

static session_state_t run_session(os_net_socket* socket)
{
    session_state_t state = SESSION_RUNNING;

    while (SESSION_RUNNING == state)
    {
        state = receive_frames(socket);

        if (SESSION_RUNNING == state)
        {
            state = forward_keys(socket);
        }

        os_delay(5);
    }

    return state;
}

/* Sends pending keystrokes to the server, which performs the line editing and echo. */
static session_state_t forward_keys(os_net_socket* socket)
{
    static char line[COMMAND_BUFFER_SIZE];
    static size_t line_len = 0;
    static uint8 esc_state = 0;
    uint8 keys[CLIENT_KEY_BATCH];
    size_t count = 0;

    while (count < CLIENT_KEY_BATCH)
    {
        int key = os_console_getch();

        if (key < 0)
        {
            break;
        }

        if (CLIENT_ESCAPE_KEY == key)
        {
            return SESSION_ESCAPED;
        }

        /* Mirror the server's line editor to recognise the quit command. */
        if (1 == esc_state)
        {
            esc_state = ('[' == key || 'O' == key) ? 2 : 0;
        }
        else if (2 == esc_state)
        {
            esc_state = (key >= 0x40 && key <= 0x7e) ? 0 : 2;
        }
        else if (0x1b == key)
        {
            esc_state = 1;
        }
        else if ('\r' == key || '\n' == key)
        {
            size_t pos = 0;

            line[line_len] = '\0';
            line_len = 0;

            while (' ' == line[pos])
            {
                pos++;
            }
            if ('q' == line[pos] && (' ' == line[pos + 1] || '\0' == line[pos + 1]))
            {
                return SESSION_ESCAPED;
            }
        }
        else if (0x7f == key || 0x08 == key)
        {
            if (line_len > 0)
            {
                do
                {
                    line_len--;
                } while (line_len > 0 && (line[line_len] & 0xc0) == 0x80);
            }
        }
        else if (0x03 == key)
        {
            line_len = 0;
        }
        else if (key >= 0x20 && line_len < sizeof(line) - 1)
        {
            line[line_len++] = (char)key;
        }

        keys[count++] = (uint8)key;
    }

    if ((count > 0) && (false == send_frame(socket, WS_OP_TEXT, keys, count)))
    {
        return SESSION_ERROR;
    }

    return SESSION_RUNNING;
}

/* Reads from the socket and prints complete text frames. */
static session_state_t receive_frames(os_net_socket* socket)
{
    int received;

    if (raw_len < CLIENT_RAW_SIZE)
    {
        received = os_net_read(socket, raw + raw_len, (int)(CLIENT_RAW_SIZE - raw_len));
        if (received < 0)
        {
            return SESSION_CLOSED;
        }
        raw_len += (size_t)received;
    }

    while (raw_len >= 2)
    {
        uint8 opcode = raw[0] & 0x0f;
        size_t header_len = 2;
        size_t payload_len = raw[1] & 0x7f;

        /* Servers must not mask; frames > 64 KiB are not supported. */
        if ((0 != (raw[1] & 0x80)) || (127 == payload_len))
        {
            os_log(LOG_ERROR, "Protocol error.");
            return SESSION_ERROR;
        }

        if (126 == payload_len)
        {
            if (raw_len < 4)
            {
                break;
            }
            payload_len = ((size_t)raw[2] << 8) | raw[3];
            header_len = 4;
        }

        if (raw_len < header_len + payload_len)
        {
            if (header_len + payload_len > CLIENT_RAW_SIZE)
            {
                os_log(LOG_ERROR, "Protocol error.");
                return SESSION_ERROR;
            }
            break;
        }

        switch (opcode)
        {
            case WS_OP_CONTINUATION:
            case WS_OP_TEXT:
            case WS_OP_BINARY:
                fwrite(raw + header_len, 1, payload_len, stdout);
                fflush(stdout);
                break;
            case WS_OP_PING:
                if (false == send_frame(socket, WS_OP_PONG, raw + header_len, payload_len))
                {
                    return SESSION_ERROR;
                }
                break;
            case WS_OP_CLOSE:
                return SESSION_CLOSED;
            default:
                break;
        }

        raw_len -= header_len + payload_len;
        os_memmove(raw, raw + header_len + payload_len, raw_len);
    }

    return SESSION_RUNNING;
}

/* Clients must mask their frames. */
static bool send_frame(os_net_socket* socket, uint8 opcode, const uint8* payload, size_t len)
{
    uint8 frame[2 + 4 + 125];
    uint8 mask[4];
    size_t i;

    /* Only short frames are needed for keystrokes, pongs and close. */
    if (len > 125)
    {
        len = 125;
    }

    for (i = 0; i < 4; i++)
    {
        mask[i] = next_random();
    }

    frame[0] = (uint8)(0x80 | opcode);
    frame[1] = (uint8)(0x80 | len);
    os_memcpy(frame + 2, mask, 4);

    for (i = 0; i < len; i++)
    {
        frame[6 + i] = payload[i] ^ mask[i & 3];
    }

    return os_net_write(socket, frame, (int)(6 + len));
}

/* Not cryptographically secure, the WebSocket key and mask only need to vary. */
static uint8 next_random(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;

    return (uint8)(rng_state >> 8);
}

static void base64_encode(const uint8* data, size_t len, char* out)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i;
    size_t n = 0;

    for (i = 0; i < len; i += 3)
    {
        uint32 v = (uint32)data[i] << 16;

        if (i + 1 < len)
        {
            v |= (uint32)data[i + 1] << 8;
        }
        if (i + 2 < len)
        {
            v |= data[i + 2];
        }

        out[n++] = table[(v >> 18) & 0x3f];
        out[n++] = table[(v >> 12) & 0x3f];
        out[n++] = (i + 1 < len) ? table[(v >> 6) & 0x3f] : '=';
        out[n++] = (i + 2 < len) ? table[v & 0x3f] : '=';
    }
    out[n] = '\0';
}
