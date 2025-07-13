void memset8(void *dst, u8 val, int count);
void memset16(void *dst, u16 val, int count);
void memset32(void *dst, u32 val, int count);
void memcpy16(void *dst, const void *src, int count);
void memcpy32(void *dst, const void *src, int count);
int clamp_i(int x, int min, int max);
int count_bits(int n);
int log2_c(int n);