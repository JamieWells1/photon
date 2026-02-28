// Find size of array
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Matrix bounds
#define MAX_X 31
#define MAX_Y 7
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) <= MAX_X && (y) >= 0 && (y) <= MAX_Y)

// Helper macro to convert debug level to string
#define LOG_LEVEL_STR(level)      \
    ((level) == ERROR   ? "ERROR" \
     : (level) == WARN  ? "WARN"  \
     : (level) == INFO  ? "INFO"  \
     : (level) == DEBUG ? "DEBUG" \
     : (level) == TRACE ? "TRACE" \
                        : "UNKNOWN")

// Debug macro that takes a level parameter
#define debug(level, fmt, ...)                                                        \
    do                                                                                \
    {                                                                                 \
        if ((int)LOG_LEVEL >= (int)(level))                                           \
        {                                                                             \
            printf("[%s] %s:%d: " fmt "\n", LOG_LEVEL_STR(level), __FILE__, __LINE__, \
                   ##__VA_ARGS__);                                                    \
        }                                                                             \
    } while (0)

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
