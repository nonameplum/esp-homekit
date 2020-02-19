#include <stdarg.h>

#ifdef ESP_OPEN_RTOS

#include <string.h>
#include <esp/hwrand.h>
#include <espressif/esp_common.h>
#include <esplibs/libmain.h>

#include "debug.h"
#include "mdns.h"

#ifndef MDNS_TTL
#define MDNS_TTL 4500
#endif

uint32_t homekit_random() {
    return hwrand();
}

void homekit_random_fill(uint8_t *data, size_t size) {
    hwrand_fill(data, size);
}

void homekit_system_restart() {
    sdk_system_restart();
}

void homekit_overclock_start() {
    sdk_system_overclock();
}

void homekit_overclock_end() {
    sdk_system_restoreclock();
}

#define MDNS_TXT_COUNT 15
#define MDNS_TXT_REC_LENGHT 128

int mdns_txt_count = 0;
char **mdns_txt_records = NULL;
static char mdns_instance_name[65] = {0};
static int mdns_port = 80;

static void srv_txt(struct mdns_service *service, void *txt_userdata) {
    LWIP_UNUSED_ARG(txt_userdata);

    DEBUG("mdns_txt count: %d", mdns_txt_count);
    for (int i=0; i<mdns_txt_count; i++) {
        err_t res;
        DEBUG("mdns add txt: %s, len: %d", mdns_txt_records[i], strlen(mdns_txt_records[i]));
        res = mdns_resp_add_service_txtitem(service, mdns_txt_records[i], strlen(mdns_txt_records[i]));
        if (res != ERR_OK) {
            DEBUG("mdns add service txt failed '%s'", mdns_txt_records[i]);
        }
    }
}

static void mdns_report(struct netif* netif, u8_t result, s8_t service) {
    DEBUG("mdns status[netif %d][service %d]: %d\n", netif->num, service, result);
}

static void free_mdns_txt_records() {
    DEBUG("Free mDNS txt records %d", mdns_txt_count);
    if (mdns_txt_records != NULL) {
        for (int i=0; i<mdns_txt_count; i++) {
            free(mdns_txt_records[i]);
        }
        free(mdns_txt_records);
    }
    mdns_txt_count = 0;
}

void homekit_mdns_init() {
    // mDNS initialization done in the final step
}

void homekit_mdns_configure_init(const char *instance_name, int port) {
    strncpy(mdns_instance_name, instance_name, sizeof(mdns_instance_name));
    mdns_port = port;
    DEBUG("Free mdns txt records");
    free_mdns_txt_records();
    mdns_txt_records = (char**) malloc(sizeof(char*) * MDNS_TXT_COUNT);
}

void homekit_mdns_add_txt(const char *key, const char *format, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, format);

    char value[128];
    int value_len = vsnprintf(value, sizeof(value), format, arg_ptr);

    va_end(arg_ptr);

    if (value_len && value_len < sizeof(value)-1) {
        char buffer[128];
        int buffer_len = snprintf(buffer, sizeof(buffer), "%s=%s", key, value);

        if (buffer_len < sizeof(buffer)-1) {
            int count = mdns_txt_count;
            mdns_txt_records[count] = (char *)malloc(sizeof(char) * (buffer_len));
            strcpy(mdns_txt_records[count], buffer);
            mdns_txt_count += 1;
            DEBUG("homekit_mdns_add_txt | count: %d | txt: %s | buffer: %s | buffer_len: %d", 
            count, mdns_txt_records[count], buffer, buffer_len);
        }
    }
}

void homekit_mdns_configure_finalize() {
    LOCK_TCPIP_CORE();
    mdns_resp_register_name_result_cb(mdns_report);
    mdns_resp_init();
    mdns_resp_add_netif(netif_default, "lwip");
    mdns_resp_add_service(netif_default, mdns_instance_name, "_hap", DNSSD_PROTO_TCP, mdns_port, srv_txt, NULL);
    mdns_resp_announce(netif_default);    
    UNLOCK_TCPIP_CORE();

    DEBUG("mDNS announcement: Name=%s Port=%d", mdns_instance_name, mdns_port);
}

#endif

#ifdef ESP_IDF

#include <string.h>
#include <mdns.h>

uint32_t homekit_random() {
    return esp_random();
}

void homekit_random_fill(uint8_t *data, size_t size) {
    uint32_t x;
    for (int i=0; i<size; i+=sizeof(x)) {
        x = esp_random();
        memcpy(data+i, &x, (size-i >= sizeof(x)) ? sizeof(x) : size-i);
    }
}

void homekit_system_restart() {
    esp_restart();
}

void homekit_overclock_start() {
}

void homekit_overclock_end() {
}

void homekit_mdns_init() {
    mdns_init();
}

void homekit_mdns_configure_init(const char *instance_name, int port) {
    mdns_hostname_set(instance_name);
    mdns_instance_name_set(instance_name);
    mdns_service_add(instance_name, "_hap", "_tcp", port, NULL, 0);
}

void homekit_mdns_add_txt(const char *key, const char *format, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, format);

    char value[128];
    int value_len = vsnprintf(value, sizeof(value), format, arg_ptr);

    va_end(arg_ptr);

    if (value_len && value_len < sizeof(value)-1) {
        mdns_service_txt_item_set("_hap", "_tcp", key, value);
    }
}

void homekit_mdns_configure_finalize() {
    /*
    printf("mDNS announcement: Name=%s %s Port=%d TTL=%d\n",
           name->value.string_value, txt_rec, PORT, 0);
    */
}

#endif
