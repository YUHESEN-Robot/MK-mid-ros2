#ifndef __YHS_CANCONTROL_NODE_H__
#define __YHS_CANCONTROL_NODE_H__

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <mutex>

#include "rclcpp/rclcpp.hpp"

#include "geometry_msgs/msg/pose_with_covariance.hpp"
#include "geometry_msgs/msg/twist_with_covariance.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/imu.hpp"

#include "yhs_can_interfaces/msg/io_cmd.hpp"
#include "yhs_can_interfaces/msg/ctrl_cmd.hpp"
#include "yhs_can_interfaces/msg/chassis_info_fb.hpp"
#include "yhs_can_interfaces/msg/ctrl_fb.hpp"
#include "yhs_can_interfaces/msg/io_fb.hpp"
#include "yhs_can_interfaces/msg/lr_wheel_fb.hpp"
#include "yhs_can_interfaces/msg/rr_wheel_fb.hpp"
#include "yhs_can_interfaces/msg/odo_fb.hpp"
#include "yhs_can_interfaces/msg/bms_info_fb.hpp"
#include "yhs_can_interfaces/msg/bms_flag_info_fb.hpp"
#include "yhs_can_interfaces/msg/veh_diag_fb.hpp"
#include "yhs_can_interfaces/msg/ultrasonic.hpp"

#define READ_PARAM(TYPE, NAME, VAR, VALUE)  \
  VAR = VALUE;                              \
  node_->declare_parameter<TYPE>(NAME, VAR); \
  node_->get_parameter(NAME, VAR);

namespace yhs
{
  class CanControl
  {

  public:
    CanControl(rclcpp::Node::SharedPtr);
    ~CanControl();

    bool run();
    void stop();

  private:
    rclcpp::Node::SharedPtr node_;

    // Parameters
    std::string if_name_;
    std::string odom_frame_;
    std::string base_link_frame_;
    bool tf_used_;
    double wheel_base_;
    int can_socket_;
    std::thread recv_thread_;

    // IMU related
    std::mutex mutex_;
    double imu_roll_;
    double imu_pitch_;
    double imu_yaw_;
    rclcpp::Time last_imu_time_;
    bool imu_active_;

    // IO parameters from config
    bool io_param_enable_;
    bool io_param_lower_beam_;
    bool io_param_upper_beam_;
    int io_param_turn_lamp_;
    bool io_param_braking_lamp_;
    bool io_param_clearance_lamp_;
    bool io_param_fog_lamp_;
    bool io_param_speaker_;
    bool io_param_discharge_;

    // Current command state
    yhs_can_interfaces::msg::IoCmd current_io_cmd_;
    yhs_can_interfaces::msg::CtrlCmd current_ctrl_cmd_;

    // Chassis info feedback (aggregated)
    yhs_can_interfaces::msg::ChassisInfoFb current_chassis_info_;

    // Ultrasonic config
    std::vector<int64_t> ultrasonic_number_;

    // CAN frame buffers
    can_frame send_frames_[2];
    can_frame recv_frames_[1];

    // Subscriptions
    rclcpp::Subscription<yhs_can_interfaces::msg::IoCmd>::SharedPtr io_cmd_subscriber_;
    rclcpp::Subscription<yhs_can_interfaces::msg::CtrlCmd>::SharedPtr ctrl_cmd_subscriber_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscriber_;

    // Publishers
    rclcpp::Publisher<yhs_can_interfaces::msg::ChassisInfoFb>::SharedPtr chassis_info_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::CtrlFb>::SharedPtr ctrl_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::IoFb>::SharedPtr io_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::LrWheelFb>::SharedPtr lr_wheel_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::RrWheelFb>::SharedPtr rr_wheel_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::OdoFb>::SharedPtr odo_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::BmsInfoFb>::SharedPtr bms_info_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::BmsFlagInfoFb>::SharedPtr bms_flag_info_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::VehDiagFb>::SharedPtr veh_diag_fb_publisher_;
    rclcpp::Publisher<yhs_can_interfaces::msg::Ultrasonic>::SharedPtr ultrasonic_publisher_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;

    // Timer for periodic command sending
    rclcpp::TimerBase::SharedPtr timer_;

    // Callbacks
    void io_cmd_callback(const yhs_can_interfaces::msg::IoCmd::SharedPtr io_cmd_msg);
    void ctrl_cmd_callback(const yhs_can_interfaces::msg::CtrlCmd::SharedPtr ctrl_cmd_msg);
    void imu_callback(const sensor_msgs::msg::Imu::SharedPtr imu_msg);
    void timer_callback();

    bool wait_for_can_frame();
    void recvData();
    void sendIoCommand();
    void sendCtrlCommand();
    void publish_odom(const double velocity, const double steering);
  };

}

#endif
