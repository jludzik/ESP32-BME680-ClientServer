#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "myunp_esp32.h"
#include "bme680.h"

//----LED--------------------------
#define Led 23 

//----BME680-----------------------
#define I2C_MASTER_SDA 18
#define I2C_MASTER_SCL 19
#define PORT 0

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

//-----Server-----------------------
#define SERV_PORT 9999
#define LISTENQ 4
#define MAX_CLIENTS 10

//-----Wi-Fi------------------------
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"
#define STATIC_IP "STATIC_IP"
#define GATEWAY_IP "GATEWAY_IP"
#define NETMASK "NETMASK"


volatile static bool wifiConnectedFlag = false;
volatile static bool waitingForMeasureFlag = false;

static SemaphoreHandle_t queueMutex;
volatile uint8_t clientQueueCounter = 0;
static int clientQueue[MAX_CLIENTS];

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI("WIFI_CONNECT", "Wi-Fi started, connecting...");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI("WIFI_CONNECT", "Wi-Fi disconnected, retrying...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        wifiConnectedFlag = true;
        ESP_LOGI("WIFI_CONNECT", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void getMeasurementsBME680(void* buf, size_t buf_size)
{
    bme680_t sensor;
    memset(&sensor, 0, sizeof(bme680_t));

    ESP_ERROR_CHECK(bme680_init_desc(&sensor, BME680_I2C_ADDR_1, PORT, I2C_MASTER_SDA, I2C_MASTER_SCL));
    ESP_ERROR_CHECK(bme680_init_sensor(&sensor));

    bme680_set_oversampling_rates(&sensor, BME680_OSR_4X, BME680_OSR_2X, BME680_OSR_2X);
    bme680_set_filter_size(&sensor, BME680_IIR_SIZE_7);
    bme680_set_heater_profile(&sensor, 0, 200, 100);
    bme680_use_heater_profile(&sensor, 0);

    uint32_t duration;
    bme680_get_measurement_duration(&sensor, &duration);
    TickType_t last_wakeup = xTaskGetTickCount();
    bme680_values_float_t values;
    uint8_t numberOfMeasures = 0;

    do
    {
        if (bme680_force_measurement(&sensor) == ESP_OK)
        {
            vTaskDelay(duration);

            if (bme680_get_results_float(&sensor, &values) == ESP_OK)
            {
                printf("[%d] BME680 Sensor: %.2f °C, %.2f %%, %.2f hPa, %.2f Ohm\n",numberOfMeasures,values.temperature, values.humidity, values.pressure, values.gas_resistance);
                numberOfMeasures++;
            }
            else
            {
                printf("BME680 Sensor: error\n\r");
            }
        }

        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(1000));

    }while(numberOfMeasures < 2);

    snprintf(buf,buf_size,"BME680 Sensor: %.2f °C, %.2f %%, %.2f hPa, %.2f Ohm\n",values.temperature, values.humidity, values.pressure, values.gas_resistance);
    return;
}

void clientTask(void* arg)
{
    int _connfd = *((int*) arg);
    free(arg);

    char buff[MAXLINE];

    while(1)
    {
        if(clientQueue[0] == _connfd)
        {
            for(int i=0;i<10;i++)
            {
                if(!waitingForMeasureFlag)
                {
                    gpio_set_level(Led,1);
                    ESP_LOGI("clientTask","Starting BME680...");
                    waitingForMeasureFlag = true;

                    getMeasurementsBME680(buff, sizeof(buff));
                    Writen(_connfd,buff,sizeof(buff));

                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    waitingForMeasureFlag = false;
                    close(_connfd);

                    xSemaphoreTake(queueMutex, portMAX_DELAY);
                    for(int i=0;i<MAX_CLIENTS;i++)
                    {
                        if(i<MAX_CLIENTS-1) clientQueue[i] = clientQueue[i+1];
                        else if(i==(MAX_CLIENTS-1)) clientQueue[i] = 0;
                    }
                    clientQueueCounter--;
                    xSemaphoreGive(queueMutex);

                    gpio_set_level(Led,0);
                    vTaskDelete(NULL);
                }
                else
                {
                    ESP_LOGI("clientTask","Waiting for BME680...");
                    snprintf(buff,sizeof(buff),"Waiting for BME680\n");
                    Writen(_connfd,buff,sizeof(buff));
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
            }

            ESP_LOGI("clientTask","Unable to get BME680 measurments, disconnecting...");
            snprintf(buff,sizeof(buff),"Unable to get BME680 measurments, disconnecting...\n\r");
            Writen(_connfd,buff,sizeof(buff));

            close(_connfd);

            xSemaphoreTake(queueMutex, portMAX_DELAY);
            for(int i=0;i<MAX_CLIENTS;i++)
            {
                if(i<MAX_CLIENTS-1) clientQueue[i] = clientQueue[i+1];
                else if(i==(MAX_CLIENTS-1)) clientQueue[i] = 0;
             }
            clientQueueCounter--;
            xSemaphoreGive(queueMutex);

            vTaskDelete(NULL);
        }
        else
        {
            xSemaphoreTake(queueMutex, portMAX_DELAY);
            uint8_t taskQueuePos = 0;
            for(int i=0;i<MAX_CLIENTS;i++)
            {
                if(clientQueue[i] == _connfd) taskQueuePos = i;
            }
            xSemaphoreGive(queueMutex);

            ESP_LOGI("clientTask","Client %d is waiting in queue at pos %d",_connfd,taskQueuePos);
            snprintf(buff,sizeof(buff),"Waiting in queue: %d\n\r",taskQueuePos);
            Writen(_connfd,buff,sizeof(buff));
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }   
}

void mainServerTask(void* arg)
{
    //LISTEN SOCKET  
    int listenfd;
    struct sockaddr_in listensock;
    socklen_t listensock_len = sizeof(listensock);
    memset(&listensock,0,listensock_len);

    listensock.sin_family = AF_INET;
    listensock.sin_port = htons(SERV_PORT);
    listensock.sin_addr.s_addr = htonl(INADDR_ANY);


    //CONNECTION SOCKET
    int* connfd;
    struct sockaddr_in connsock;
    socklen_t connsock_len = sizeof(connsock);
    memset(&connsock,0,connsock_len);
    

    listenfd = Socket(AF_INET,SOCK_STREAM,0);
    ESP_LOGI("TCP_SERVER","Socket created!");
    Bind(listenfd, (struct sockaddr*) &listensock, listensock_len); 
    Listen(listenfd, LISTENQ);
    ESP_LOGI("TCP_SERVER","Server start listening!");


    while(1)
    {
        if(clientQueueCounter < MAX_CLIENTS)
        {
            connfd = malloc(sizeof(int));
            *connfd = Accept(listenfd, (struct sockaddr*) &connsock, &connsock_len);
            
            xSemaphoreTake(queueMutex, portMAX_DELAY);
            clientQueue[clientQueueCounter] = *connfd;
            ESP_LOGI("TCP_SERVER","[%d] Client connected!",clientQueueCounter);
            clientQueueCounter++;
            xSemaphoreGive(queueMutex);

            xTaskCreate(clientTask,"clientTask",4096,connfd,5,NULL);
            vTaskDelay(100);
        }
        else
        {
            ESP_LOGI("TCP_SERVER","Max clients connected!");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}



void app_main(void)
{
    //wifi init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *netif = esp_netif_create_default_wifi_sta();

    esp_netif_ip_info_t ip_info;
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(STATIC_IP, &ip_info.ip));
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(GATEWAY_IP, &ip_info.gw));
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(NETMASK, &ip_info.netmask));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));

    ESP_ERROR_CHECK(esp_wifi_start());
    
    while(!wifiConnectedFlag)
    {
        ESP_LOGI("WIFI_CONNECT","Connecting...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    //queue init
    queueMutex = xSemaphoreCreateMutex();
    memset(clientQueue,0,sizeof(clientQueue));

    //i2c init
    ESP_ERROR_CHECK(i2cdev_init());

    //led pin init
    gpio_reset_pin(Led);
    gpio_set_direction(Led,GPIO_MODE_OUTPUT);
    gpio_set_level(Led,0);

    //server task
    xTaskCreate(mainServerTask, "mainServerTask", 4096, NULL, 5, NULL);

}