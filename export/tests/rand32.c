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
static int g_2 = 0xB110C93BL;
static int *g_6 = &g_2;
static int g_8 = 0xEBB0CB8CL;
static int func_1(void);
static int func_1(void)
{
    int *l_7 = &g_8;
    step_hash(5);
    for (g_2 = 0; (g_2 <= 4); g_2 += 3)
    {
        short l_5 = 0x98A5L;
        step_hash(4);
        return l_5;
    }
    step_hash(6);
    g_6 = (void*)0;
    step_hash(7);
    (*l_7) |= g_2;
    step_hash(8);
    return g_8;
}
void csmith_compute_hash(void)
{
    transparent_crc(g_2, "g_2", print_hash_value);
    transparent_crc(g_8, "g_8", print_hash_value);
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
