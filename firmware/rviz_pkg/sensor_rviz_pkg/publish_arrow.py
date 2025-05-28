import math
import rclpy
from rclpy.node import Node
from std_msgs.msg import UInt8
from visualization_msgs.msg import Marker

class MarkerPublisher(Node):
    def __init__(self):
        super().__init__('arrow_publisher')
        self.publisher_ = self.create_publisher(Marker, 'arrow_topic', 10)
        self.subscription = self.create_subscription(UInt8, '/sensor_data', self.sensor_callback, 10)
        self.subscription

    def sensor_callback(self, msg):
        yaw = 0.0
        
        if msg.data == 1:
            yaw = math.pi
        elif msg.data == 2:
            yaw = math.pi / 2
        elif msg.data == 3:
            yaw = -math.pi / 2
        elif msg.data == 4:
            yaw = math.pi / 4
        elif msg.data == 5:
            yaw = -math.pi / 4

        qz = math.sin(yaw / 2.0)
        qw = math.cos(yaw / 2.0)

        marker = Marker()
        marker.header.frame_id = "map"
        marker.header.stamp = self.get_clock().now().to_msg()
        marker.ns = "arrow"
        marker.id = 0

        if msg.data < 6:
            marker.type = Marker.ARROW
            marker.action = Marker.ADD

            marker.pose.position.x = 0.0
            marker.pose.position.y = 0.0
            marker.pose.position.z = 0.0

            marker.pose.orientation.x = 0.0
            marker.pose.orientation.y = 0.0
            marker.pose.orientation.z = float(qz)
            marker.pose.orientation.w = float(qw)

            marker.scale.x = 3.0
            marker.scale.y = 2.0
            marker.scale.z = 0.5

            marker.color.a = 1.0
            marker.color.r = 1.0
            marker.color.g = 0.0
            marker.color.b = 1.0
        else:
            marker.action = Marker.DELETE

        self.publisher_.publish(marker)

def main(args=None):
    rclpy.init(args=args)
    node = MarkerPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
