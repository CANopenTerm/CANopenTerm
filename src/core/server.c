/** @file server.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "server.h"
#include "os.h"

#define SERVER_MAX_CLIENTS 4
#define SERVER_RAW_SIZE 4096
#define SERVER_IN_SIZE 2048
#define SERVER_OUT_SIZE 16384
#define SERVER_ACCEPT_SIZE 32

#define WS_OP_CONTINUATION 0x0
#define WS_OP_TEXT 0x1
#define WS_OP_BINARY 0x2
#define WS_OP_CLOSE 0x8
#define WS_OP_PING 0x9
#define WS_OP_PONG 0xA

typedef struct client
{
    os_net_socket* socket;
    bool is_upgraded;
    bool is_cr;
    uint8 esc_state;
    uint8 raw[SERVER_RAW_SIZE];
    size_t raw_len;
    uint8 in[SERVER_IN_SIZE];
    size_t in_len;
    size_t in_pos;
    char line[COMMAND_BUFFER_SIZE];
    size_t line_len;

} client_t;

static const char* const ansi_color[] = {
    "\x1b[0m",  /* DEFAULT_COLOR */
    "\x1b[30m", /* DARK_BLACK */
    "\x1b[34m", /* DARK_BLUE */
    "\x1b[32m", /* DARK_GREEN */
    "\x1b[36m", /* DARK_CYAN */
    "\x1b[31m", /* DARK_RED */
    "\x1b[35m", /* DARK_MAGENTA */
    "\x1b[33m", /* DARK_YELLOW */
    "\x1b[37m", /* DARK_WHITE */
    "\x1b[90m", /* LIGHT_BLACK */
    "\x1b[94m", /* LIGHT_BLUE */
    "\x1b[92m", /* LIGHT_GREEN */
    "\x1b[96m", /* LIGHT_CYAN */
    "\x1b[91m", /* LIGHT_RED */
    "\x1b[95m", /* LIGHT_MAGENTA */
    "\x1b[93m", /* LIGHT_YELLOW */
    "\x1b[97m"  /* LIGHT_WHITE */
};

static os_net_server* server = NULL;
static os_mutex* out_mutex = NULL;
static client_t clients[SERVER_MAX_CLIENTS];
static char out_queue[SERVER_OUT_SIZE];
static size_t out_len = 0;
static volatile sig_atomic_t is_sigint_pending = 0;

static void sigint_handler(int sig);
static void output_cb(const color_t color, const char* text);
static void queue_append(const char* data, size_t len);
static void queue_flush(void);
static void accept_clients(void);
static void drop_client(client_t* client);
static bool send_frame(client_t* client, uint8 opcode, const uint8* payload, size_t len);
static void send_text(client_t* client, const char* text, size_t len);
static bool read_client(client_t* client);
static bool do_handshake(client_t* client);
static bool parse_frame(client_t* client);
static bool process_input(client_t* client, char command[COMMAND_BUFFER_SIZE]);
static bool find_header(const char* request, const char* name, char* value, size_t value_size);
static void make_accept_key(const char* key, char accept[SERVER_ACCEPT_SIZE]);
static void sha1(const uint8* data, size_t len, uint8 digest[20]);
static void base64_encode(const uint8* data, size_t len, char* out);

status_t server_init(const char* host, uint16 port)
{
    os_net_address* address;

    if (false == os_net_init())
    {
        os_log(LOG_ERROR, "Unable to initialise network: %s", os_get_error());
        return OS_INIT_ERROR;
    }

    address = os_net_resolve(host);
    if ((NULL == address) || (os_net_wait_resolved(address, 5000) <= 0))
    {
        os_log(LOG_ERROR, "Unable to resolve host '%s': %s", host, os_get_error());

        if (NULL != address)
        {
            os_net_unref_address(address);
        }
        os_net_quit();

        return OS_INVALID_ARGUMENT;
    }

    server = os_net_server_create(address, port, 0);
    os_net_unref_address(address);

    if (NULL == server)
    {
        os_log(LOG_ERROR, "Unable to listen on %s:%u: %s", host, (unsigned int)port, os_get_error());
        os_net_quit();
        return OS_INIT_ERROR;
    }

    out_mutex = os_mutex_create();
    if (NULL == out_mutex)
    {
        os_net_server_destroy(server);
        server = NULL;
        os_net_quit();
        return OS_INIT_ERROR;
    }

    os_memset(clients, 0, sizeof(clients));

    out_len = 0;
    os_log(LOG_INFO, "Listening on ws://%s:%u", host, (unsigned int)port);
    os_silence_output();
    os_set_output_cb(output_cb);
    is_sigint_pending = 0;

    os_signal(OS_SIGINT, sigint_handler);

    return ALL_OK;
}

/* Returns true if Ctrl+C was pressed on the console while no WebSocket client is attached. */
bool server_is_quit_requested(void)
{
    int i;

    if (0 == is_sigint_pending)
    {
        return false;
    }
    is_sigint_pending = 0;

    for (i = 0; i < SERVER_MAX_CLIENTS; i++)
    {
        if (NULL != clients[i].socket && true == clients[i].is_upgraded)
        {
            return false;
        }
    }

    return true;
}

static void sigint_handler(int sig)
{
    (void)sig;
    is_sigint_pending = 1;
    os_signal(OS_SIGINT, sigint_handler);
}

bool server_update(char command[COMMAND_BUFFER_SIZE])
{
    int i;
    bool is_ready = false;

    if (NULL == server)
    {
        return false;
    }

    queue_flush();
    accept_clients();

    for (i = 0; i < SERVER_MAX_CLIENTS && false == is_ready; i++)
    {
        client_t* client = &clients[i];

        if (NULL == client->socket)
        {
            continue;
        }

        if (client->in_pos >= client->in_len)
        {
            if (false == read_client(client))
            {
                drop_client(client);
                continue;
            }
        }

        if (client->in_pos < client->in_len)
        {
            is_ready = process_input(client, command);
        }
    }

    return is_ready;
}

void server_deinit(void)
{
    int i;

    if (NULL == server)
    {
        return;
    }

    os_set_output_cb(NULL);
    os_signal(OS_SIGINT, SIG_DFL);

    for (i = 0; i < SERVER_MAX_CLIENTS; i++)
    {
        if (NULL != clients[i].socket)
        {
            send_frame(&clients[i], WS_OP_CLOSE, NULL, 0);
            drop_client(&clients[i]);
        }
    }

    os_net_server_destroy(server);
    server = NULL;
    os_mutex_destroy(out_mutex);
    out_mutex = NULL;
    os_net_quit();
}

/* May be called from any thread, e.g. the CAN monitor. */
static void output_cb(const color_t color, const char* text)
{
    const char* sgr = ansi_color[(color >= DEFAULT_COLOR && color <= LIGHT_WHITE) ? color : DEFAULT_COLOR];
    size_t len = os_strlen(text);
    size_t sgr_len = os_strlen(sgr);
    size_t i;
    size_t n = 0;
    char* tmp;

    tmp = (char*)os_calloc(2 * len + 2 * sgr_len + 1, sizeof(char));
    if (NULL == tmp)
    {
        return;
    }

    if (color != DEFAULT_COLOR)
    {
        os_memcpy(tmp, sgr, sgr_len);
        n = sgr_len;
    }

    /* Terminal emulators need CR LF. */
    for (i = 0; i < len; i++)
    {
        if ('\n' == text[i] && (0 == i || '\r' != text[i - 1]))
        {
            tmp[n++] = '\r';
        }
        tmp[n++] = text[i];
    }

    if (color != DEFAULT_COLOR)
    {
        os_memcpy(tmp + n, ansi_color[DEFAULT_COLOR], 4);
        n += 4;
    }

    queue_append(tmp, n);
    os_free(tmp);
}

static void queue_append(const char* data, size_t len)
{
    os_mutex_lock(out_mutex);

    if (out_len + len <= SERVER_OUT_SIZE)
    {
        os_memcpy(out_queue + out_len, data, len);
        out_len += len;
    }
    os_mutex_unlock(out_mutex);
}

static void queue_flush(void)
{
    int i;
    size_t len;
    static char chunk[SERVER_OUT_SIZE];

    os_mutex_lock(out_mutex);
    len = out_len;
    os_memcpy(chunk, out_queue, len);
    out_len = 0;
    os_mutex_unlock(out_mutex);

    if (0 == len)
    {
        return;
    }

    for (i = 0; i < SERVER_MAX_CLIENTS; i++)
    {
        if (NULL != clients[i].socket && true == clients[i].is_upgraded)
        {
            send_text(&clients[i], chunk, len);
        }
    }
}

static void accept_clients(void)
{
    os_net_socket* socket = NULL;

    while (os_net_accept(server, &socket) && (NULL != socket))
    {
        int i;

        for (i = 0; i < SERVER_MAX_CLIENTS; i++)
        {
            if (NULL == clients[i].socket)
            {
                os_memset(&clients[i], 0, sizeof(client_t));
                clients[i].socket = socket;
                socket = NULL;
                break;
            }
        }

        if (NULL != socket)
        {
            /* No free slot. */
            os_net_socket_destroy(socket);
            socket = NULL;
        }
    }
}

static void drop_client(client_t* client)
{
    os_net_socket_destroy(client->socket);
    os_memset(client, 0, sizeof(client_t));
}

static bool send_frame(client_t* client, uint8 opcode, const uint8* payload, size_t len)
{
    uint8 header[4];
    int header_len = 2;

    header[0] = (uint8)(0x80 | opcode);
    if (len < 126)
    {
        header[1] = (uint8)len;
    }
    else
    {
        /* Output chunks never exceed SERVER_OUT_SIZE (< 64 KiB). */
        header[1] = 126;
        header[2] = (uint8)((len >> 8) & 0xff);
        header[3] = (uint8)(len & 0xff);
        header_len = 4;
    }

    if (false == os_net_write(client->socket, header, header_len))
    {
        return false;
    }

    if ((len > 0) && (false == os_net_write(client->socket, payload, (int)len)))
    {
        return false;
    }

    return true;
}

static void send_text(client_t* client, const char* text, size_t len)
{
    if (false == send_frame(client, WS_OP_TEXT, (const uint8*)text, len))
    {
        drop_client(client);
    }
}

static bool read_client(client_t* client)
{
    int received;

    if (client->raw_len >= SERVER_RAW_SIZE)
    {
        return false;
    }

    received = os_net_read(client->socket, client->raw + client->raw_len, (int)(SERVER_RAW_SIZE - client->raw_len));
    if (received < 0)
    {
        return false;
    }
    client->raw_len += (size_t)received;

    if (false == client->is_upgraded)
    {
        return do_handshake(client);
    }

    return parse_frame(client);
}

static bool do_handshake(client_t* client)
{
    char key[64];
    char accept[SERVER_ACCEPT_SIZE];
    char response[256];
    char* end;
    size_t request_len;
    int response_len;

    client->raw[client->raw_len < SERVER_RAW_SIZE ? client->raw_len : SERVER_RAW_SIZE - 1] = 0;

    end = os_strstr((char*)client->raw, "\r\n\r\n");
    if (NULL == end)
    {
        /* Request incomplete, keep reading unless buffer is exhausted. */
        return client->raw_len < SERVER_RAW_SIZE - 1;
    }
    request_len = (size_t)(end - (char*)client->raw) + 4;

    if ((0 != os_strncmp((char*)client->raw, "GET ", 4)) ||
        (false == find_header((char*)client->raw, "Sec-WebSocket-Key", key, sizeof(key))))
    {
        const char* bad_request = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";

        os_net_write(client->socket, bad_request, (int)os_strlen(bad_request));
        return false;
    }

    make_accept_key(key, accept);
    response_len = os_snprintf(response,
                               sizeof(response),
                               "HTTP/1.1 101 Switching Protocols\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Accept: %s\r\n\r\n",
                               accept);
    if (false == os_net_write(client->socket, response, response_len))
    {
        return false;
    }

    client->is_upgraded = true;
    client->raw_len -= request_len;
    os_memmove(client->raw, client->raw + request_len, client->raw_len);

    send_text(client, "\x1b[2J\x1b[H", 7);
    if (NULL != client->socket)
    {
        send_text(client, ": ", 2);
    }

    return NULL != client->socket;
}

/* Extract the next complete frame into client->in. Returns false on protocol error or close. */
static bool parse_frame(client_t* client)
{
    uint8 opcode;
    uint8 mask[4];
    size_t header_len = 2;
    size_t payload_len;
    size_t i;
    const uint8* raw = client->raw;

    if (client->raw_len < 2)
    {
        return true;
    }

    opcode = raw[0] & 0x0f;
    payload_len = raw[1] & 0x7f;

    /* Clients must mask; frames > 64 KiB are not supported. */
    if (0 == (raw[1] & 0x80) || 127 == payload_len)
    {
        return false;
    }

    if (126 == payload_len)
    {
        if (client->raw_len < 4)
        {
            return true;
        }
        payload_len = ((size_t)raw[2] << 8) | raw[3];
        header_len = 4;
    }

    if (payload_len > SERVER_IN_SIZE)
    {
        return false;
    }

    if (client->raw_len < header_len + 4 + payload_len)
    {
        return true;
    }

    os_memcpy(mask, raw + header_len, 4);
    header_len += 4;

    for (i = 0; i < payload_len; i++)
    {
        client->in[i] = raw[header_len + i] ^ mask[i & 3];
    }

    client->raw_len -= header_len + payload_len;
    os_memmove(client->raw, client->raw + header_len + payload_len, client->raw_len);

    switch (opcode)
    {
        case WS_OP_CONTINUATION:
        case WS_OP_TEXT:
        case WS_OP_BINARY:
            client->in_len = payload_len;
            client->in_pos = 0;
            break;
        case WS_OP_PING:
            return send_frame(client, WS_OP_PONG, client->in, payload_len);
        case WS_OP_CLOSE:
            send_frame(client, WS_OP_CLOSE, NULL, 0);
            return false;
        default:
            break;
    }

    return true;
}

/* Line editor. Returns true as soon as one line is complete. */
static bool process_input(client_t* client, char command[COMMAND_BUFFER_SIZE])
{
    while (client->in_pos < client->in_len)
    {
        uint8 c = client->in[client->in_pos++];

        /* Swallow escape sequences, e.g. cursor keys. */
        if (1 == client->esc_state)
        {
            client->esc_state = ('[' == c || 'O' == c) ? 2 : 0;
            continue;
        }
        else if (2 == client->esc_state)
        {
            if (c >= 0x40 && c <= 0x7e)
            {
                client->esc_state = 0;
            }
            continue;
        }

        if (0x1b == c)
        {
            client->esc_state = 1;
        }
        else if ('\r' == c || ('\n' == c && false == client->is_cr))
        {
            client->is_cr = ('\r' == c);
            client->line[client->line_len] = '\0';
            os_strlcpy(command, client->line, COMMAND_BUFFER_SIZE);
            client->line_len = 0;
            send_text(client, "\r\n", 2);
            return true;
        }
        else if ('\n' == c)
        {
            client->is_cr = false;
        }
        else if (0x7f == c || 0x08 == c)
        {
            if (client->line_len > 0)
            {
                /* Skip UTF-8 continuation bytes. */
                do
                {
                    client->line_len--;
                } while (client->line_len > 0 && (client->line[client->line_len] & 0xc0) == 0x80);
                send_text(client, "\b \b", 3);
            }
        }
        else if (0x03 == c)
        {
            client->line_len = 0;
            send_text(client, "^C\r\n: ", 6);
        }
        else if (c >= 0x20 && client->line_len < COMMAND_BUFFER_SIZE - 1)
        {
            char ch = (char)c;

            client->line[client->line_len++] = ch;
            send_text(client, &ch, 1);
        }

        if (NULL == client->socket)
        {
            break;
        }
    }

    return false;
}

static bool find_header(const char* request, const char* name, char* value, size_t value_size)
{
    size_t name_len = os_strlen(name);
    const char* line = request;

    while (NULL != line && '\0' != *line)
    {
        const char* eol = os_strstr(line, "\r\n");

        if (0 == os_strncasecmp(line, name, name_len) && ':' == line[name_len])
        {
            size_t len;
            const char* start = line + name_len + 1;

            while (' ' == *start)
            {
                start++;
            }
            len = (NULL != eol) ? (size_t)(eol - start) : os_strlen(start);
            if (len >= value_size)
            {
                return false;
            }
            os_memcpy(value, start, len);
            value[len] = '\0';
            return true;
        }

        line = (NULL != eol) ? eol + 2 : NULL;
    }

    return false;
}

static void make_accept_key(const char* key, char accept[SERVER_ACCEPT_SIZE])
{
    uint8 digest[20];
    uint8 data[128];
    const char* guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    size_t key_len = os_strlen(key);
    size_t guid_len = os_strlen(guid);

    /* Key length is bounded by the caller's buffer. */
    os_memcpy(data, key, key_len);
    os_memcpy(data + key_len, guid, guid_len);
    sha1(data, key_len + guid_len, digest);
    base64_encode(digest, 20, accept);
}

#define ROL32(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static void sha1(const uint8* data, size_t len, uint8 digest[20])
{
    uint32 h[5];
    uint32 w[80];
    uint8 block[64];
    size_t total = len + 1;
    size_t padded;
    size_t offset;
    int i;

    h[0] = 0x67452301;
    h[1] = 0xEFCDAB89;
    h[2] = 0x98BADCFE;
    h[3] = 0x10325476;
    h[4] = 0xC3D2E1F0;

    /* Data + 0x80 + zero padding + 64 bit length, multiple of 64. */
    padded = ((total + 8 + 63) / 64) * 64;

    for (offset = 0; offset < padded; offset += 64)
    {
        uint32 a, b, c, d, e;

        for (i = 0; i < 64; i++)
        {
            size_t pos = offset + (size_t)i;

            if (pos < len)
            {
                block[i] = data[pos];
            }
            else if (pos == len)
            {
                block[i] = 0x80;
            }
            else if (pos >= padded - 8)
            {
                /* Message length in bits, big endian; len is far below 2^29. */
                size_t shift = (padded - 1 - pos) * 8;

                block[i] = (shift < 32) ? (uint8)(((uint32)len * 8u >> shift) & 0xff) : 0;
            }
            else
            {
                block[i] = 0;
            }
        }

        for (i = 0; i < 16; i++)
        {
            w[i] = ((uint32)block[i * 4] << 24) | ((uint32)block[i * 4 + 1] << 16) | ((uint32)block[i * 4 + 2] << 8) | (uint32)block[i * 4 + 3];
        }
        for (i = 16; i < 80; i++)
        {
            w[i] = ROL32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        a = h[0];
        b = h[1];
        c = h[2];
        d = h[3];
        e = h[4];

        for (i = 0; i < 80; i++)
        {
            uint32 f, k, temp;

            if (i < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (i < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (i < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            temp = ROL32(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = ROL32(b, 30);
            b = a;
            a = temp;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }

    for (i = 0; i < 20; i++)
    {
        digest[i] = (uint8)((h[i / 4] >> (24 - 8 * (i % 4))) & 0xff);
    }
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
