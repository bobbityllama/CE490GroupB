/*********************************************************************
 *
 *      Project DeathAlert Sensor Node Code
 *
 *********************************************************************
 * FileName:        rf22_router_sensor_node.ino
 *
 * Dependencies:    Arduino & RFM22B hardware, and the below files & compilers
 *
 * Processor:       ATmega328
 *
 * Compiler:        Arduino 1.6.8
 *
 * Company:         Kettering University
 *
 * Author:          NinjaNife
 *
 * Modified:        April 30, 2016
 *
 * Platform:        Arduino Pro Mini 3.3v
 *
 * Features utilized:
 *    - Serial configuration and usage
 *    - SPI configuration and usage
 *    - Timer configuration and usage
 *    - RadioHead transmission and routing
 *
 * Description:
 *   This file contains the Sensor Node code for the DeathAlert system.
 *   In order to fully function, all cluster heads and sensor nodes
 *   must be properly connected to RFM22B transceivers and sensors.
 ********************************************************************/
 
/*****************************************************
 * Definitions
 *****************************************************/
#define RH_RF22_MAX_MESSAGE_LEN 255
#define SN1_ADDRESS 1
#define SN2_ADDRESS 2
#define SN3_ADDRESS 3
#define SN4_ADDRESS 4
#define SN5_ADDRESS 5
#define SN6_ADDRESS 6
#define CHA_ADDRESS 7
#define CHC_ADDRESS 8
#define CHB_ADDRESS 9
#define BS_ADDRESS 10
#define MY_ADDRESS SN1_ADDRESS
#define RECEIVE_TIMEOUT_MS 500
#define RSSI_TRANSMIT_THRESHOLD 100


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
  uint8_t data[13]; // Holds sensor ID (1 byte), latitude (4 bytes), longitude (4 bytes), micTime (4 bytes)
  uint8_t dataLength = 13;
} message;


/*****************************************************
 * Variables
 *****************************************************/
// RadioHead objects
RH_RF22 driver; // Singleton instance of the radio
RHRouter manager(driver, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above

// Miscellaneous variables
uint32_t latitude, longitude, micTime;
bool micDataAvailable = false;
bool transmitReady = false;


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

  // Manually define the routes for this network
  switch(MY_ADDRESS) {
    case SN1_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHC_ADDRESS);
      break;
    case SN2_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHC_ADDRESS);
      break;
    case SN3_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHB_ADDRESS);
      break;
    case SN4_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHB_ADDRESS);
      break;
    case SN5_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHA_ADDRESS);
      break;
    case SN6_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHA_ADDRESS);
      break;
    default:
      break;
  }
  
}


/*******************************************************************************
  Function:
 void loop()

  Summary:
  This function contains the primary system code and function calls.
 */
void loop() {
  if(transmitReady) {
    // Transmit message if possible
    transmitMessage();
  }
  
  // Check for received message and store as needed
  recieveMessage();

  // Monitor sensors
  monitorSensors();
}


/*******************************************************************************
  Function:
 void transmitMessage()

  Summary:
  This function transmits messages to the next hop.
 */
void transmitMessage() {
  // Loop infinitely until transmission can occur
  while(driver.rssiRead() < RSSI_TRANSMIT_THRESHOLD) {
    // Delay for 1-5ms
    delay(random(1, 6));
  }
  
  // Transmit message to base station
  if(manager.sendtoWait(message.data, message.dataLength, BS_ADDRESS) == RH_ROUTER_ERROR_NONE) {
    // Reset mic data bool
    micDataAvailable = false;
    transmitReady = false;
  } else
    // Message delivery failed; add something here to change route and try again
    Serial.println("sendtoWait failed.");
}


/*******************************************************************************
  Function:
 void recieveMessage()

  Summary:
  This function checks for waiting transmission and creates new message structs.
 */
void recieveMessage() {
  // Check if there is a message available
  if(manager.recvfromAckTimeout(message.data, &message.dataLength, RECEIVE_TIMEOUT_MS)) {
    // Add the latest sensor data into the message structure
    message.data[0] = MY_ADDRESS;
    message.data[1] = latitude & 0xFF;
    message.data[2] = (latitude >> 8) & 0xFF;
    message.data[3] = (latitude >> 16) & 0xFF;
    message.data[4] = (latitude >> 24) & 0xFF;
    message.data[5] = longitude & 0xFF;
    message.data[6] = (longitude >> 8) & 0xFF;
    message.data[7] = (longitude >> 16) & 0xFF;
    message.data[8] = (longitude >> 24) & 0xFF;

    // Check if mic data is available and add to message
    if(micDataAvailable) {
      message.data[9] = micTime & 0xFF;
      message.data[10] = (micTime >> 8) & 0xFF;
      message.data[11] = (micTime >> 16) & 0xFF;
      message.data[12] = (micTime >> 24) & 0xFF;
    } else {
      message.data[9] = 0x00;
      message.data[10] = 0x00;
      message.data[11] = 0x00;
      message.data[12] = 0x00;
    }

    // Set the bool to true so that the system knows to transmit a message
    transmitReady = true;
  }
}


/*******************************************************************************
  Function:
 void monitorSensors()

  Summary:
  This function monitors the sensors and updates variables accordingly.
 */
void monitorSensors() {
  // Monitor sensors
  int endTime = millis() + RECEIVE_TIMEOUT_MS;
  while(millis() < endTime) {
    // Monitor the sensors and collect data

    // Mic data has been found; set bools
    //micDataAvailable = true;
  }
}

