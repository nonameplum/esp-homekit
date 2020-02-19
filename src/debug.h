#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char byte;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef HOMEKIT_DEBUG

#define DEBUG(message, ...) printf(">>> [%s:%d] %s: " message "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#else

#define DEBUG(message, ...)

#endif

#ifdef HOMEKIT_LOG_DISABLE

#define INFO(message, ...)
#define ERROR(message, ...)

#else

#define INFO(message, ...) printf(">>> [%s:%d] HomeKit: " message "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define ERROR(message, ...) printf("!!! HomeKit: " message "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

#endif

#ifdef ESP_IDF

#define DEBUG_HEAP() DEBUG("Free heap: %d", esp_get_free_heap_size());

#else

#define DEBUG_HEAP() DEBUG("Free heap: %d", xPortGetFreeHeapSize());

#endif

char *binary_to_string(const byte *data, size_t size);
void print_binary(const char *prompt, const byte *data, size_t size);

#endif // __DEBUG_H__
