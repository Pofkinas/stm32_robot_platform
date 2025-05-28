#include "uROS_Comms.h"

#include <Arduino.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <micro_ros_platformio.h>

#include <std_msgs/msg/u_int8.h>
#include <geometry_msgs/msg/twist.h>
//#include <std_msgs/msg/header.h>

rcl_publisher_t sensor_publisher;
// rcl_subscription_t cmd_vel_subscriber;

std_msgs__msg__UInt8 sensor_data;
//geometry_msgs__msg__Twist cmd_vel_msg;

rcl_node_t node;
rcl_allocator_t allocator;
rclc_support_t support;
//rclc_executor_t executor;

#define RCCHECK(fn) \
{\
    rcl_ret_t temp_rc = fn;\
    if (RCL_RET_OK != temp_rc) {\
        printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc);\
    }\
}

#define RCSOFTCHECK(fn) \
{\
    rcl_ret_t temp_rc = fn;\
    if(RCL_RET_OK != temp_rc) {\
        printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc);\
    }\
}

uRosComms::uRosComms (){}

static void CMD_Vel_Callback (const void *message);

static void CMD_Vel_Callback (const void *message) {
    geometry_msgs__msg__Twist *recieved_data = (geometry_msgs__msg__Twist *) message;
}

void uRosComms::Init (){
    Serial.begin(115200);
    Serial.println("ROS Communication node started");

    IPAddress agent_ip(172, 20, 10, 5);
    uint16_t agent_port = 8888;

    char ssid[] = "";
    char password[] = "";

    set_microros_wifi_transports(ssid, password, agent_ip, agent_port);

    delay(2000);

    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    allocator = rcl_get_default_allocator();

    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, "robot_node", "", &support));

    //RCCHECK(rclc_subscription_init_default(&cmd_vel_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "/cmd_vel"));
    RCCHECK(rclc_publisher_init_default(&sensor_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt8), "/sensor_data"));

    //RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    //RCCHECK(rclc_executor_add_subscription(&executor, &cmd_vel_subscriber, &cmd_vel_msg, &uRosComms::CMD_Vel_Callback, ON_NEW_DATA));
}

void uRosComms::Receive (){
    //RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

void uRosComms::Publish (const uint8_t sensor_status) {
    sensor_data.data = sensor_status;

    RCSOFTCHECK(rcl_publish(&sensor_publisher, (const void*) &sensor_data, NULL));
}
