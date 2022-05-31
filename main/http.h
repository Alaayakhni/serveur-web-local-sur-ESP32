/*
 * http.h
 *
 *  Created on: 19 avr. 2022
 *      Author: Alaa_YAKHNI
 */

#ifndef MAIN_HTTP_H_
#define MAIN_HTTP_H_
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <sys/param.h>

#include <esp_http_server.h>

#define ESP_WIFI_SSID       CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASSWORD  CONFIG_ESP_WIFI_PASSWORD
#define NB_RESEAUX_MAX      16

#define WEB_SERVER_PORT    12443
/*Ce port est ouvert dans le pare-feu
 lorsque l'accès à distance est activé*/




static const char *TAG = "Test Serveur HTTP";
static bool demandeConnexion = false;
char adresseIP[24];
extern float temperature_degC;

httpd_handle_t start_webserver();
void stop_webserver(httpd_handle_t server);

 bool scan_wifi();
 void connect_wifi();
 esp_err_t init_wifi();
 esp_err_t temp_get_handler(httpd_req_t *req);
#endif /* MAIN_HTTP_H_ */
