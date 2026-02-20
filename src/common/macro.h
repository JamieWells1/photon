// Find size of array
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Matrix bounds
#define MAX_X 31
#define MAX_Y 7
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) <= MAX_X && (y) >= 0 && (y) <= MAX_Y)

// Debug levels
#if DEBUG_LEVEL == TRACE
#define debug(fmt, ...) printf("[TRACE] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif

// Assert macro that panics with useful information
#define ASSERT(condition, msg, ...)                      \
    do                                                   \
    {                                                    \
        if (!(condition))                                \
        {                                                \
            printf("\n*** ASSERTION FAILED ***\n");      \
            printf("File: %s\n", __FILE__);              \
            printf("Line: %d\n", __LINE__);              \
            printf("Condition: %s\n", #condition);       \
            printf("Message: " msg "\n", ##__VA_ARGS__); \
            panic("Assertion failed: %s", #condition);   \
        }                                                \
    } while (0)

// Null pointer check
#define ASSERT_NOT_NULL(ptr) ASSERT((ptr) != NULL, "Pointer '%s' is NULL", #ptr)

// Bounds check
#define ASSERT_IN_RANGE(value, min, max)                                                         \
    ASSERT((value) >= (min) && (value) <= (max), "Value %d out of range [%d, %d]", (int)(value), \
           (int)(min), (int)(max))
