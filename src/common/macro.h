#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#if DEBUG_LEVEL == TRACE
#define debug(fmt, ...) printf("[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif
