#pragma once

#define DESCRIBE_JSON_SIZE 4096
#define ACTION_STATUS_JSON_SIZE 1024
#define PROPERTY_STATUS_JSON_SIZE 512
#define SMALL_MESSAGE_JSON_SIZE 256
#define INCOMING_JSON_CAPACITY 2048

#ifndef LIGHTHOUSE_CLIENT_MAX
#define LIGHTHOUSE_CLIENT_MAX 5
#endif

#ifndef LIGHTHOUSE_DEFAULT_MIN_PROPERTY_UPDATES_PERIOD_MS
#define LIGHTHOUSE_DEFAULT_MIN_PROPERTY_UPDATES_PERIOD_MS 500
#endif

#ifndef LIGHTHOUSE_CLIENT_KEEPALIVE_TIMEOUT
#define LIGHTHOUSE_CLIENT_KEEPALIVE_TIMEOUT 15000
#endif

#ifndef LIGHTHOUSE_NEW_CLIENT_AUTOMATICALLY_SUBSCRIBED_TO_ALL_PROPERTIES
//TODO should be false as default but leaved for backward compatibility
#define LIGHTHOUSE_NEW_CLIENT_AUTOMATICALLY_SUBSCRIBED_TO_ALL_PROPERTIES true
#endif

#define WIFI_CONNECTING_MAX_TIME 20000 // 20000ms = 20s