/** Beispiel fuer das Auslesen von RFID Tags mittels RFID Reader
*/
#include "mbed.h"
#include "MFRC522.h"
#include "OLEDDisplay.h"

// UI
OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );

// NFC/RFID Reader (SPI)
MFRC522    rfidReader( MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST ); 

int main()
{



                    printf("Maxim");

                    
    // OLED Display
    oled.clear();
    //oled.printf( "RFID Demo\n" );
        
    printf("RFID Reader MFRC522 Test V3\n");

    rfidReader.PCD_Init();

    while   ( 1 ) 
    {
        // RFID Reader
        if ( rfidReader.PICC_IsNewCardPresent())
            if ( rfidReader.PICC_ReadCardSerial()) 
            {
                oled.cursor( 1, 0 );                
                // Print Card UID (2-stellig mit Vornullen, Hexadecimal)
                printf("UID: ");
                for ( int i = 0; i < rfidReader.uid.size; i++ )
                    printf("%02X:", rfidReader.uid.uidByte[i]);
                printf("\r\n");
                oled.printf("BITTE EINE 6");
                // Print Card type
                int piccType = rfidReader.PICC_GetType(rfidReader.uid.sak);
                printf("PICC Type: %s \r\n", rfidReader.PICC_GetTypeName(piccType) );
                
            }
        thread_sleep_for( 200 );
    }
}