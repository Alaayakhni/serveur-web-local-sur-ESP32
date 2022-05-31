/*
 * http.c
 *
 *  Created on: 19 avr. 2022
 *      Author: Alaa_YAKHNI
 */
#include "http.h"
#include "script.h"

 httpd_handle_t server_httpd = NULL;

// Gestionnaire d'évènements
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        if(demandeConnexion)
        {
            ESP_LOGI(TAG, "Connexion ...");
            esp_wifi_connect();
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // reconnexion ?
        ESP_LOGI(TAG, "Deconnexion !");
        stop_webserver(server_httpd);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Adresse IP : " IPSTR, IP2STR(&event->ip_info.ip));
        if (server_httpd == NULL)
        {
            ESP_LOGI(TAG, "Demarrage serveur HTTP");
            server_httpd = start_webserver();
        }
    }
}

 esp_err_t init_wifi()
{
    // initialise la pile TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());
    // crée une boucle d'évènements
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // crée
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // initialise la configuration Wifi par défaut
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // initialise le WiFi avec la configuration par défaut et démarre également la tâche WiFi.
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // installe le gestionnaire d'évènements Wifi
    /*
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
    */
    // version obsolète !
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // définit le mode de fonctionnement station pour le WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // démarre le WiFi selon la configuration
    esp_err_t ret = esp_wifi_start();
    ESP_ERROR_CHECK(ret);

    return ret;
}

 bool scan_wifi()
{
    bool present = false;
    uint16_t number = NB_RESEAUX_MAX;
    wifi_ap_record_t ap_info[NB_RESEAUX_MAX];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    // démarre le scan
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    ESP_LOGI(TAG, "Nb reseaux trouves = %u", ap_count);
    for (int i = 0; (i < NB_RESEAUX_MAX) && (i < ap_count); i++)
    {
        // cf. https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html?highlight=wifi_ap_record_t#_CPPv416wifi_ap_record_t
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
        if(strcmp((char *)ap_info[i].ssid, ESP_WIFI_SSID) == 0)
            present = true;
    }

    return present;
}


static void restart_wifi()
{
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_start());
}

 void connect_wifi()
{
    // configure la connexion Wifi du point d'accès (AP)
    wifi_config_t wifi_config = {
        .sta = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASSWORD,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .pmf_cfg = {
            .capable = true,
            .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    demandeConnexion = true;
    restart_wifi();
}





// Gestionnaire d'URI pour une requête GET
esp_err_t get_handler(httpd_req_t *req)
{httpd_resp_set_type(req, "text/html");
httpd_resp_send_chunk(req, "<head><title>Sensor's Page</title></head>", -1);
//httpd_resp_send_chunk(req, "<body style=\"text-align: center;\">", -1);
httpd_resp_send_chunk(req, (char*)style, -1);
httpd_resp_send_chunk(req, "<div style=\"display: inline-table;width: 90%;\">", -1);
httpd_resp_send_chunk(req, "<div class=\"TempCard\">Temperature&nbsp;is:<br> <span id=\"temp_value\">---</span></div>", -1);
httpd_resp_send_chunk(req, "<div style=\"width: 2%;\">&nbsp;</div>", -1);
//httpd_resp_send_chunk(req, "</div>", -1);
httpd_resp_send_chunk(req, (char*)refresh, -1);

return ESP_OK;
}
///////////////////temp
esp_err_t temp_get_handler(httpd_req_t *req)
{
	char body[100];
	sprintf(body,"%0.2f &#176;C", temperature_degC);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send_chunk(req, body, -1);
    httpd_resp_send_chunk(req, "", 0);
    return ESP_OK;
}

// Gestionnaire d'URI pour une requête POST /test
esp_err_t post_handler(httpd_req_t *req)
{
    char content[128];

    ESP_LOGI(TAG, "Longueur contenu POST : %d", req->content_len);

    // tronque si la longueur du contenu est supérieure au buffer
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);

    // déconnecté ?
    if (ret <= 0)
    {
        // timeout ?
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            // retourne une erreur HTTP 408 (Request Timeout)
            httpd_resp_send_408(req);
        }

        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Contenu POST : %s", content);

    // envoie une réponse
    const char resp[] = "Test Reponse POST";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}
httpd_handle_t start_webserver()
{
    // initialise la configuration par défaut du serveur HTTP
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_SERVER_PORT;

    // pour une requête GET /test
    httpd_uri_t uri_get = {
        .uri      = "/",
        .method   = HTTP_GET,
        .handler  = get_handler,
        .user_ctx = NULL
    };

    // pour une requête POST /test
    httpd_uri_t uri_post = {
        .uri      = "/",
        .method   = HTTP_POST,
        .handler  = post_handler,
        .user_ctx = NULL
    };
     // température
    const httpd_uri_t temp = {
        .uri       = "/temp",
        .method    = HTTP_GET,
        .handler   = temp_get_handler
    };



    ESP_LOGI(TAG, "Port serveur HTTP : '%d'", config.server_port);

    httpd_handle_t server = NULL;
    // Démarre le serveur HTTP
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // enregistre les gestionnaires d'URI
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
        httpd_register_uri_handler(server, &temp);
    }

    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server)
    {
        // arrête le serveur HTTP
        httpd_stop(server);
    }
}
