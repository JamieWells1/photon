// Find size of array
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Matrix bounds
#define MAX_X 31
#define MAX_Y 7
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) <= MAX_X && (y) >= 0 && (y) <= MAX_Y)

// Debug levels
#if DEBUG_LEVEL == TRACE
#define debug(fmt, ...) printf("[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif
