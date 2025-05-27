#include "uROS_Comms.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/header.h>

//#include <micro_ros_platformio.h>
#include <geometry_msgs/msg/twist.h>

rcl_publisher_t sensor_publisher;
// rcl_subscription_t cmd_vel_subscriber;

std_msgs__msg__Bool sensor_data;
// geometry_msgs__msg__Twist cmd_vel_msg;

rcl_node_t node;
rcl_allocator_t allocator;
rclc_support_t support;
//rclc_executor_t executor;

#define RCCHECK(fn) \
{\
    rcl_ret_t temp_rc = fn;\
    if (RCL_RET_OK != temp_rc) {\
        printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc);\
        return 1;\
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

static void uRosComms::CMD_Vel_Callback (const void *message);

void uRosComms::Init (){
    Serial.begin(115200);
    Serial.println("ROS Communication node started");
    setupDisplay(display);

    IPAddress agent_ip(192, 168, 100, 21);
    uint16_t agent_port = 8888;

    char ssid[] = "RobotPlatform";
    char password[]= "321654987";

    set_microros_wifi_transports(ssid, psk, agent_ip, agent_port);

    delay(2000);

    allocator = rcl_get_default_allocator();

    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, "robot_node", "", &support));

    //RCCHECK(rclc_subscription_init_default(&cmd_vel_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "/cmd_vel"));
    RCCHECK(rclc_publisher_init_default(&sensor_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, sensor_data, Bool), "/sensor_data"));

    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    //RCCHECK(rclc_executor_add_subscription(&executor, &cmd_vel_subscriber, &cmd_vel_msg, &uRosComms::CMD_Vel_Callback, ON_NEW_DATA));

    RCCHECK(rcl_publisher_fini(&sensor_publisher, &node));
    //RCCHECK(rcl_subscription_fini(&cmd_vel_subscriber, &node));
    RCCHECK(rcl_node_fini(&node));
}

void uRosComms::Receive (){
    RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
    
    delay(100);
}

void uRosComms::Publish (const bool sensor_status) {
    sensor_data.data = sensor_status;

    RCSOFTCHECK(rcrl_publish(&sensor_publisher, (const void*) &sensor_data, NULL));

    delay(100);
}

static void uRosComms::CMD_Vel_Callback (const void *message) {
    geometry_msgs__msg__Twist *recieved_data = (geometry_msgs__msg__Twist *) message;

}
