/**
    M242 Module Project 
    @autors: Maxim Zelensky & Jason Banyer
    @date: 12.03.2021
    @version: 
    @desctiption: NFC Sensor gets UID and send to Node backend -> will be saved in MongoDB
    @todo: change wifi pref.
*/
#include "mbed.h"
#include <cstdio>
#include <string>
#include "MFRC522.h"
#include "http_request.h"
#include "MbedJSONValue.h"
#include "OLEDDisplay.h"
#include "config.cpp"

// UI
OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );

// NFC/RFID Reader (SPI)
MFRC522 rfidReader( MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST ); 

// I/O Buffer
char message[6000];
char uUID[2000];

int main()
{      
    // OLED Display
    oled.clear();
    printf("RFID HTTP\n");
    oled.printf("RFID HTTP\n");
    rfidReader.PCD_Init();

    //connect to wifi
    oled.printf("wifi connection...\n");
    WiFiInterface* network = WiFiInterface::get_default_instance();
    if (!network) {
        printf("ERROR: No WiFiInterface found.\n");
        oled.printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }
    printf("\nConnecting to %s...\n", WIFI_SSID);
    int ret = network->connect(WIFI_SSID, WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        oled.printf("\nConnection error: %d\n", ret);
        return -1;
    }
    printf("Success wifi conn\n\n");
    oled.printf("Success wifi conn\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP: %s\n", a.get_ip_address());  
    //end of wifi connect

    oled.clear();
    oled.printf("Attach NFC-Card\n");
    while   ( 1 )
    {
        // RFID Reader
        if ( rfidReader.PICC_IsNewCardPresent())
            if ( rfidReader.PICC_ReadCardSerial()) 
            {
                oled.cursor( 1, 0 );                
                // Print Card UID (2-stellig mit Vornullen, Hexadecimal)
                printf("UID: ");
                //clear old uid
                sprintf(uUID, "%s:", "");
                for ( int i = 0; i < rfidReader.uid.size; i++ )
                    //get uid
                    // printf("%02X:", rfidReader.uid.uidByte[i]);
                    sprintf(uUID, "%s%02X:", uUID, rfidReader.uid.uidByte[i]);
                    printf("\nUID: %s", uUID);
                    oled.clear();

                    HttpRequest* post_req = new HttpRequest(network, HTTP_POST, "http://m242-backend.herokuapp.com/ajax/save-data");
                    // HttpResponse* get_res = get_req->send();
                    char body[] = "";
                    // sprintf( body, "%s", "{\"uid\":\"12345\"}\n",);
                    sprintf( body, "%s%s%s", "{\"uid\":\"", uUID, "\"}\n");
                    printf("BODY: %s\n", body);
                    post_req->set_header("Content-Type", "application/json");
                    HttpResponse* post_res = post_req->send(body, strlen(body));
                    
                    // send request
                    oled.printf("sending request...\n");

                    //success response
                    if (post_res){
                        printf( "request: %s\n", (char*) post_res->get_body() );
                        oled.printf( "request: %s\n", (char*) post_res->get_body() );
                        printf("SUCCESS REQUEST");
                        oled.printf("SUCCESS REQUEST\n");
                    }
                    // Error
                    else{
                        printf("HttpRequest failed (error code %d)\n", post_req->get_error());
                        oled.printf("HttpRequest failed (error code %d)\n", post_req->get_error());
                        return 1;
                    }
                    delete post_req;
            }
        thread_sleep_for( 200 );
    }
}