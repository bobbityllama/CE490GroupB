/*********************************************************************
 *
 *      Project DeathAlert Base Station Code
 *
 *********************************************************************
 * FileName:        rf22_router_base_station.ino
 *
 * Dependencies:    Arduino & RFM22B hardware, and the below files & compilers
 *
 * Processor:       ATmega2560
 *
 * Compiler:        Arduino 1.6.8
 *
 * Company:         Kettering University
 *
 * Author:          NinjaNife
 *
 * Modified:        May 1, 2016
 *
 * Platform:        Arduino Mega2560
 *
 * Features utilized:
 *    - Serial configuration and usage
 *    - SPI configuration and usage
 *    - Timer configuration and usage
 *    - RadioHead transmission and routing
 *
 * Description:
 *   This file contains the Base Station code for the DeathAlert system.
 *   In order to fully function, all cluster heads and sensor nodes
 *   must be properly connected to RFM22B transceivers and sensors.
 ********************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions, Library Includes, Struct and Variable Declarations
// *****************************************************************************
// *****************************************************************************

/*****************************************************
 * Definitions
 *****************************************************/
#define RH_RF22_MAX_MESSAGE_length 255
#define CHA_ADDRESS 7
#define CHC_ADDRESS 8
#define CHB_ADDRESS 9
#define BS_ADDRESS 10
#define NUM_SECTORS 4
#define BEACON_TIME_BYTE_LENGTH 2
#define RECEIVE_TIMEOUT_MS 500
#define MY_ADDRESS BS_ADDRESS


/*****************************************************
 * Include drivers and other startup files
 *****************************************************/
#include <RHRouter.h>
#include <RH_RF22.h>
#include <SPI.h>


/*****************************************************
 * Data types
 *****************************************************/
struct _message {
  uint8_t data[13]; // Holds sensor ID (1 byte), lat (4 bytes), long (4 bytes), and time (4 bytes)
  uint8_t dataLength;
} message;

struct _sectorSettings {
  float frequency; // Channel frequency
  float afcPullIn; // Desired AFC pull-in for above frequency
  uint8_t numDivisions; // The number of time divisions in this sector
  uint8_t sectorTime[2]; // The amount of time to remain in each sector in ms (16 bits)
} sectorSettings[NUM_SECTORS];


/*****************************************************
 * Variables
 *****************************************************/
// RadioHead objects
RH_RF22 driver; // Singleton instance of the radio
RHRouter manager(driver, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above

// Miscellaneous variables
bool networkEnable = true; // Indicates if the network is currently enabled
unsigned int currentSector = 0; // Holds the ID of the sector currently being processed
unsigned int updateSector = 0; // Holds the ID of the sector to be updated


// *****************************************************************************
// *****************************************************************************
// Section: Primary System Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
  void setup()

  Summary:
  This function initializes the RFM22 hardware and all network settings
*/
void setup()
{
  // May need to change this to 19200 or 115200
  Serial.begin(9600);
  while(!manager.init()) {
    Serial.println("init failed");

    // Delay for 1000ms before trying to initialize again
    delay(1000);
  }
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36

  // Initialize sector 0
  sectorSettings[0].frequency = 434.0;
  sectorSettings[0].afcPullIn = 0.05;
  sectorSettings[0].numDivisions = 3;
  sectorSettings[0].sectorTime[0] = (5000 >> 8) & 0xFF;
  sectorSettings[0].sectorTime[1] = 5000 & 0xFF;
  
  // Initialize sector 1
  sectorSettings[1].frequency = 534.0;
  sectorSettings[1].afcPullIn = 0.10;
  sectorSettings[1].numDivisions = 3;
  sectorSettings[1].sectorTime[0] = (5000 >> 8) & 0xFF;
  sectorSettings[1].sectorTime[1] = 5000 & 0xFF;
  
  // Initialize sector 2
  sectorSettings[2].frequency = 634.0;
  sectorSettings[2].afcPullIn = 0.15;
  sectorSettings[2].numDivisions = 3;
  sectorSettings[2].sectorTime[0] = (5000 >> 8) & 0xFF;
  sectorSettings[2].sectorTime[1] = 5000 & 0xFF;
  
  // Initialize sector 3
  sectorSettings[3].frequency = 734.0;
  sectorSettings[3].afcPullIn = 0.20;
  sectorSettings[3].numDivisions = 3;
  sectorSettings[3].sectorTime[0] = (5000 >> 8) & 0xFF;
  sectorSettings[3].sectorTime[1] = 5000 & 0xFF;
}


/*******************************************************************************
  Function:
 void loop()

  Summary:
  This function contains the primary system code and function calls.
 */
void loop() {
  // Debugging
  Serial.println("Looping...");
  delay(1000);
  
  sendBeacon();
  
  checkSerial();
}


/*******************************************************************************
  Function:
 void sendBeacon()

  Summary:
  This function sends a beacon to each sector based on predefined settings.
 */
void sendBeacon() {
  // Check to see if the network should be enabled
  if(networkEnable) {
    // Send a beacon to the current sector
    manager.sendtoWait(sectorSettings[currentSector].sectorTime, BEACON_TIME_BYTE_LENGTH, RH_BROADCAST_ADDRESS, 0);
    
    // Start the timer
    int endTime = millis() + ((((uint16_t)sectorSettings[currentSector].sectorTime[0] << 8) | (uint16_t)sectorSettings[currentSector].sectorTime) / sectorSettings[currentSector].numDivisions);
    
    // Loop while the beacon waits for a message
    while(millis() < endTime) {
      //Check and see if there is a message waiting
      if(manager.recvfromAckTimeout(message.data, &message.dataLength, RECEIVE_TIMEOUT_MS)) {
        // Debugging
        Serial.println("Debugging: Sending you data...");
        
        // Immediately send the data to the laptop
        Serial.write(message.data, message.dataLength);
      }
    }
  
    // Debugging
    Serial.println("Debugging: Switching to the next sector...");
    
    // Update the radio settings for the next sector
    driver.setFrequency(sectorSettings[currentSector].frequency, sectorSettings[currentSector].afcPullIn);
    
    // Update the sector number
    if(currentSector < (NUM_SECTORS - 1))
    currentSector++;
    else
      currentSector = 0;
  }
}


/*******************************************************************************
  Function:
 void checkSerial()

  Summary:
  This function checks for serial data and updates system variables as needed.
 */
void checkSerial() {
  // Parse the serial message
  if(Serial.available() > 0) {
    switch(Serial.read()) {
      case 0xFF:
        // Loop while required data length is missing
        while(Serial.available() < 1);
        
        // Enable/Disable the network
        networkEnable = Serial.read();
        
        // Debugging
        Serial.print("Debugging: Network enable: ");
        Serial.println(networkEnable, DEC);
        break;
      case 0x0F:
        // Loop while required data length is missing
        while(Serial.available() < 1);
        
        // Update one of the sector's settings
        updateSector = Serial.read();
        
        // Debugging
        Serial.print("Debugging: Update Sector: ");
        Serial.println(updateSector, DEC);
        
        // Loop while required data length is missing
        while(Serial.available() < 4);

        // Update the freqency variable
        for (int i = sizeof(float) - 1; i >= 0 ; i--) {
           ((unsigned char *) &sectorSettings[updateSector].frequency)[i] = Serial.read();
        }
        
        // Debugging
        Serial.print("Debugging: Sector Frequency: ");
        Serial.println(sectorSettings[updateSector].frequency, 3);
        
        // Loop while required data length is missing
        while(Serial.available() < 4);

        // Update the AFC Pull-In variable
        for (int i = sizeof(float) - 1; i >= 0 ; i--) {
           ((unsigned char *) &sectorSettings[updateSector].afcPullIn)[i] = Serial.read();
        }
        
        // Debugging
        Serial.print("Debugging: Sector AFC Pull-In: ");
        Serial.println(sectorSettings[updateSector].afcPullIn, 3);
        
        // Loop while required data length is missing
        while(Serial.available() < 1);

        // Update the number of time divisions
        sectorSettings[updateSector].numDivisions = Serial.read();
        
        // Debugging
        Serial.print("Debugging: Sector Divisions: ");
        Serial.println(sectorSettings[updateSector].numDivisions, DEC);
        
        // Loop while required data length is missing
        while(Serial.available() < 2);

        // Update the time allotted to this sector
        sectorSettings[updateSector].sectorTime[0] = Serial.read();
        sectorSettings[updateSector].sectorTime[1] = Serial.read();
        
        // Debugging
        Serial.print("Debugging: Sector Time: ");
        Serial.println((((uint16_t)sectorSettings[updateSector].sectorTime[0] << 8) | ((uint16_t)sectorSettings[updateSector].sectorTime[1])), DEC);
        break;
      default:
        break;
    }
  }
}

