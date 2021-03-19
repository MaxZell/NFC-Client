/**
    M242 Module Project 
    @autors: Maxim Zelensky & Jason Banyer
    @date: 12.03.2021
    @version: 
    NFC Sensor gets UID and send to Node backend -> will be saved in MongoDB
    @todo: change wifi pref.
*/
#include "mbed.h"
#include <string>
#include "MFRC522.h"
#include "http_request.h"
#include "MbedJSONValue.h"
#include "OLEDDisplay.h"

// UI
OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );

// NFC/RFID Reader (SPI)
MFRC522 rfidReader( MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST ); 

// I/O Buffer
char message[6000];

DigitalOut myled( MBED_CONF_IOTKIT_LED1 );

int main()
{      
    // OLED Display
    oled.clear();
    printf("RFID HTTP\n");
    rfidReader.PCD_Init();

    //connect to wifi
    WiFiInterface* network = WiFiInterface::get_default_instance();
    if (!network) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = network->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }
    printf("Success\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP: %s\n", a.get_ip_address());  
    //end of wifi connect

    while   ( 1 ) 
    {
        myled = 1;
        // RFID Reader
        if ( rfidReader.PICC_IsNewCardPresent())
            if ( rfidReader.PICC_ReadCardSerial()) 
            {
                oled.cursor( 1, 0 );                
                // Print Card UID (2-stellig mit Vornullen, Hexadecimal)
                printf("UID: ");
                for ( int i = 0; i < rfidReader.uid.size; i++ )
                    printf("%02X:", rfidReader.uid.uidByte[i]);
                    //send uid to backend
                    HttpRequest* get_req = new HttpRequest(network, HTTP_GET, "https://m242-backend.herokuapp.com/ajax/get-data");
                    HttpResponse* get_res = get_req->send();
                    //success response
                    if (get_res){
                        MbedJSONValue parser;
                        // HTTP GET (JSON) parsen
                        parse( parser, get_res->get_body_as_string().c_str() );
                        
                        // std::string sunrise;
                        // std::string sunset;            
                        // sunrise = parser["results"]["sunrise"].get<std::string>();
                        // sunset  = parser["results"]["sunset"] .get<std::string>(); 
                        // // Umwandlung nach int. Damit die Zeiten besser verglichen werden können.
                        // int rh, rm, rs, sh, sm, ss;
                        // sscanf( sunrise.c_str(), "%d:%d:%d AM", &rh, &rm, &rs );
                        // sscanf( sunset .c_str(), "%d:%d:%d PM", &sh, &sm, &ss );
                        // oled.cursor( 1, 0 );
                        // oled.printf( "auf   %02d.%02d.%02d\nunter %02d.%02d.%02d\n", rh+2, rm, rs, sh+2+12, sm, ss );
                        // printf("parcesr: %s \n", parser);
                        sdt::string answer;
                        answer = parser["results"].get<std::string>();
                        printf("%s \n", answer);
                    }
                    // Error
                    else{
                        printf("HttpRequest failed (error code %d)\n", get_req->get_error());
                        return 1;
                    }
                    delete get_req;
                    myled = 0;

                printf("\r\n");
                // Print Card type
                int piccType = rfidReader.PICC_GetType(rfidReader.uid.sak);
                printf("PICC Type: %s \r\n", rfidReader.PICC_GetTypeName(piccType) );
                
            }
        thread_sleep_for( 200 );
    }
}