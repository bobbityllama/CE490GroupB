/*********************************************************************
 *
 *      Project DeathAlert Cluster Head Code
 *
 *********************************************************************
 * FileName:        rf22_router_cluster_head.ino
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
 * Modified:        May 1, 2016
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
 *   This file contains the Cluster Head code for the DeathAlert system.
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
#define RH_RF22_MAX_MESSAGE_LEN 255
#define CHA_ADDRESS 7
#define CHC_ADDRESS 8
#define CHB_ADDRESS 9
#define BS_ADDRESS 10
#define MY_ADDRESS CHC_ADDRESS
#define RECEIVE_TIMEOUT_MS 500
#define NUM_CLUSTER_HEADS 3
#define NUM_SENSOR_NODES 6


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
  bool hasData = false;
  uint8_t data[13]; // Holds sensor ID (1 byte), latitude (4 bytes), longitude (4 bytes), micTime (4 bytes)
  uint8_t dataLength;
} message[NUM_SENSOR_NODES];


/*****************************************************
 * Variables
 *****************************************************/
// RadioHead objects
RH_RF22 driver; // Singleton instance of the radio
RHRouter manager(driver, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above

// Miscellaneous variables
int transmissionStartTime = 0;


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
void setup() {
  Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36

  // Manually define the routes for this network
  switch(MY_ADDRESS) {
    case CHC_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHB_ADDRESS);
  	  break;
    case CHB_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, CHA_ADDRESS);
  	  break;
    case CHA_ADDRESS:
      manager.addRouteTo(BS_ADDRESS, BS_ADDRESS);
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
  // Transmit message if possible
  transmitMessage();
  
  // Check for received message and store as needed
  recieveMessage();
}


/*******************************************************************************
  Function:
 void transmitMessage()

  Summary:
  This function transmits messages to the next hop.
 */
void transmitMessage() {
  // Verify that this cluster head is inside its time block
  if(millis() >= transmissionStartTime) {
    // Transmit all available messages
    for(int i = 0; i < (NUM_SENSOR_NODES - 1); i++) {
      if(message[i].hasData == true) {
        // Message contains data; send this message to the next hop
        if (manager.sendtoWait(message[i].data, message[i].dataLength, BS_ADDRESS) != RH_ROUTER_ERROR_NONE)
          // Message was sent
          message[i].hasData = false;
      }
    }
  }
}


/*******************************************************************************
  Function:
 void recieveMessage()

  Summary:
  This function checks for waiting transmission and stores messages in structs.
 */
void recieveMessage() {
  // Declare local variables
  uint8_t srcID; // Holds the source address of the current message
  uint8_t msgID; // The ID of the current message struct
  
  // Determine which message struct is not full, if any
  for(int i = 0; i < (NUM_SENSOR_NODES - 1); i++) {
    if(message[i].hasData == false) {
      msgID = i;
      break;
    }
  }
  
  // Check if there is a message available
  if (manager.recvfromAckTimeout(message[msgID].data, &message[msgID].dataLength, RECEIVE_TIMEOUT_MS, &srcID)) {
    // Message was received; mark the struct as such
    message[msgID].hasData = true;

    // Check if the message is from the base station
    switch(srcID) {
      case 0xFF:
        // Set up the transmission time
        transmissionStartTime = millis() + (((((uint16_t)message[msgID].data[0] << 8) | (uint16_t)message[msgID].data[1]) / NUM_CLUSTER_HEADS) * (MY_ADDRESS - NUM_SENSOR_NODES));
        message[msgID].hasData = false;
        break;
      default:
        break;
    }
  }
}

