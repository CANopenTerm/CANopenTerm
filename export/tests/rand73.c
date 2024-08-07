#include <stdio.h>
int print_hash_value = 1;
static void platform_main_begin(void)
{
}
static unsigned crc32_tab[256];
static unsigned crc32_context = 0xFFFFFFFFUL;
static void
crc32_gentab (void)
{
 unsigned crc;
 unsigned poly = 0xEDB88320UL;
 int i, j;
 for (i = 0; i < 256; i++) {
  crc = i;
  for (j = 8; j > 0; j--) {
   if (crc & 1) {
    crc = (crc >> 1) ^ poly;
   } else {
    crc >>= 1;
   }
  }
  crc32_tab[i] = crc;
 }
}
static void
crc32_byte (unsigned char b) {
 crc32_context =
  ((crc32_context >> 8) & 0x00FFFFFF) ^
  crc32_tab[(crc32_context ^ b) & 0xFF];
}

static void
crc32_8bytes (unsigned val)
{
 crc32_byte ((val>>0) & 0xff);
 crc32_byte ((val>>8) & 0xff);
 crc32_byte ((val>>16) & 0xff);
 crc32_byte ((val>>24) & 0xff);
}
static void
transparent_crc (unsigned val, char* vname, int flag)
{
 crc32_8bytes(val);
 if (flag) {
    printf("...checksum after hashing %s : %X\n", vname, crc32_context ^ 0xFFFFFFFFU);
 }
}
static void
platform_main_end (int x, int flag)
{
  if (!flag) printf ("checksum = %x\n", x);
}
static long __undefined;
void csmith_compute_hash(void);
void step_hash(int stmt_id);
static int g_4 = 0L;
static int g_14 = 0x8ACC4B7CL;
static unsigned short g_15 = 65531UL;
static unsigned char func_1(void);
static unsigned char func_1(void)
{
    int *l_2 = (void*)0;
    int *l_3 = &g_4;
    int *l_5 = (void*)0;
    int l_6[6] = {0L, 0L, 0L, 0L, 0L, 0L};
    int *l_7 = &g_4;
    int *l_8 = &l_6[4];
    int *l_9 = &l_6[2];
    int *l_10 = (void*)0;
    int *l_11 = &l_6[1];
    int *l_12 = &l_6[5];
    int *l_13[4][9];
    unsigned l_18 = 5UL;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 9; j++)
            l_13[i][j] = &l_6[2];
    }
    step_hash(1);
    g_15--;
    step_hash(2);
    (*l_7) ^= g_14;
    step_hash(3);
    (*l_7) = l_18;
    step_hash(4);
    return g_15;
}
void csmith_compute_hash(void)
{
    transparent_crc(g_4, "g_4", print_hash_value);
    transparent_crc(g_14, "g_14", print_hash_value);
    transparent_crc(g_15, "g_15", print_hash_value);
}
void step_hash(int stmt_id)
{
    int i = 0;
    csmith_compute_hash();
    printf("before stmt(%d): checksum = %X\n", stmt_id, crc32_context ^ 0xFFFFFFFFUL);
    crc32_context = 0xFFFFFFFFUL;
    for (i = 0; i < 256; i++) {
        crc32_tab[i] = 0;
    }
    crc32_gentab();
}

int print_hash_val = 0;
platform_main_begin();
crc32_gentab();
func_1();
csmith_compute_hash();
platform_main_end(crc32_context ^ 0xFFFFFFFFUL, print_hash_val);
