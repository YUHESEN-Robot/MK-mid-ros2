#include "yhs_can_control/yhs_can_control_node.hpp"

namespace yhs
{

  CanControl::CanControl(rclcpp::Node::SharedPtr node)
      : node_(node), if_name_("can0"), odom_frame_("odom"), base_link_frame_("base_link"),
        tf_used_(false), wheel_base_(0.6), can_socket_(-1),
        io_param_enable_(false), io_param_lower_beam_(false), io_param_upper_beam_(false),
        io_param_turn_lamp_(0), io_param_braking_lamp_(false), io_param_clearance_lamp_(false),
        io_param_fog_lamp_(false), io_param_speaker_(false), io_param_discharge_(false),
        imu_roll_(0.0), imu_pitch_(0.0), imu_yaw_(0.0), imu_active_(false)
  {
    node_->declare_parameter<std::string>("if_name", if_name_);
    node_->get_parameter("if_name", if_name_);

    node_->declare_parameter<std::string>("odom_frame", odom_frame_);
    node_->get_parameter("odom_frame", odom_frame_);

    node_->declare_parameter<std::string>("base_link_frame", base_link_frame_);
    node_->get_parameter("base_link_frame", base_link_frame_);

    node_->declare_parameter<bool>("tf_used", tf_used_);
    node_->get_parameter("tf_used", tf_used_);

    node_->declare_parameter<double>("wheel_base", wheel_base_);
    node_->get_parameter("wheel_base", wheel_base_);

    node_->declare_parameter<bool>("io_cmd.enable", io_param_enable_);
    node_->get_parameter("io_cmd.enable", io_param_enable_);

    node_->declare_parameter<bool>("io_cmd.lower_beam", io_param_lower_beam_);
    node_->get_parameter("io_cmd.lower_beam", io_param_lower_beam_);

    node_->declare_parameter<bool>("io_cmd.upper_beam", io_param_upper_beam_);
    node_->get_parameter("io_cmd.upper_beam", io_param_upper_beam_);

    node_->declare_parameter<int>("io_cmd.turn_lamp", io_param_turn_lamp_);
    node_->get_parameter("io_cmd.turn_lamp", io_param_turn_lamp_);

    node_->declare_parameter<bool>("io_cmd.braking_lamp", io_param_braking_lamp_);
    node_->get_parameter("io_cmd.braking_lamp", io_param_braking_lamp_);

    node_->declare_parameter<bool>("io_cmd.clearance_lamp", io_param_clearance_lamp_);
    node_->get_parameter("io_cmd.clearance_lamp", io_param_clearance_lamp_);

    node_->declare_parameter<bool>("io_cmd.fog_lamp", io_param_fog_lamp_);
    node_->get_parameter("io_cmd.fog_lamp", io_param_fog_lamp_);

    node_->declare_parameter<bool>("io_cmd.speaker", io_param_speaker_);
    node_->get_parameter("io_cmd.speaker", io_param_speaker_);

    node_->declare_parameter<bool>("io_cmd.discharge", io_param_discharge_);
    node_->get_parameter("io_cmd.discharge", io_param_discharge_);

    node_->declare_parameter<std::vector<int64_t>>("ultrasonic_number", std::vector<int64_t>{});
    node_->get_parameter("ultrasonic_number", ultrasonic_number_);

    current_io_cmd_.io_cmd_enable = io_param_enable_;
    current_io_cmd_.io_cmd_lower_beam_headlamp = io_param_lower_beam_;
    current_io_cmd_.io_cmd_upper_beam_headlamp = io_param_upper_beam_;
    current_io_cmd_.io_cmd_turn_lamp = io_param_turn_lamp_;
    current_io_cmd_.io_cmd_braking_lamp = io_param_braking_lamp_;
    current_io_cmd_.io_cmd_clearance_lamp = io_param_clearance_lamp_;
    current_io_cmd_.io_cmd_fog_lamp = io_param_fog_lamp_;
    current_io_cmd_.io_cmd_speaker = io_param_speaker_;
    current_io_cmd_.io_cmd_dis_charge = io_param_discharge_;

    last_imu_time_ = rclcpp::Time(0, 0, RCL_ROS_TIME);

    io_cmd_subscriber_ = node_->create_subscription<yhs_can_interfaces::msg::IoCmd>(
        "io_cmd",
        10,
        std::bind(&CanControl::io_cmd_callback, this, std::placeholders::_1));

    ctrl_cmd_subscriber_ = node_->create_subscription<yhs_can_interfaces::msg::CtrlCmd>(
        "ctrl_cmd",
        10,
        std::bind(&CanControl::ctrl_cmd_callback, this, std::placeholders::_1));

    imu_subscriber_ = node_->create_subscription<sensor_msgs::msg::Imu>(
        "imu_data",
        10,
        std::bind(&CanControl::imu_callback, this, std::placeholders::_1));

    // ctrl_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::CtrlFb>("ctrl_fb", 10);
    chassis_info_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::ChassisInfoFb>("chassis_info_fb", 10);
    // lr_wheel_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::LrWheelFb>("lr_wheel_fb", 10);
    // rr_wheel_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::RrWheelFb>("rr_wheel_fb", 10);
    // io_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::IoFb>("io_fb", 10);
    odo_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::OdoFb>("odo_fb", 10);
    // bms_info_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::BmsInfoFb>("bms_info_fb", 10);
    // bms_flag_info_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::BmsFlagInfoFb>("bms_flag_info_fb", 10);
    // veh_diag_fb_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::VehDiagFb>("veh_diag_fb", 10);
    // ultrasonic_publisher_ = node_->create_publisher<yhs_can_interfaces::msg::Ultrasonic>("ultrasonic", 10);
    odom_pub_ = node_->create_publisher<nav_msgs::msg::Odometry>("odom", 10);

    timer_ = node_->create_wall_timer(std::chrono::milliseconds(10), std::bind(&CanControl::timer_callback, this));
  }

  void CanControl::sendIoCommand()
  {
    static unsigned char count_1 = 0;
    unsigned char sendData_u_io[8];

    memset(sendData_u_io, 0, 8);

    sendData_u_io[0] = current_io_cmd_.io_cmd_enable;

    sendData_u_io[1] = 0;
    if (current_io_cmd_.io_cmd_lower_beam_headlamp)
      sendData_u_io[1] |= 0x01;

    if (current_io_cmd_.io_cmd_upper_beam_headlamp)
      sendData_u_io[1] |= 0x02;

    sendData_u_io[1] |= (current_io_cmd_.io_cmd_turn_lamp & 0x03) << 2;

    if (current_io_cmd_.io_cmd_braking_lamp)
      sendData_u_io[1] |= 0x10;

    if (current_io_cmd_.io_cmd_clearance_lamp)
      sendData_u_io[1] |= 0x20;

    if (current_io_cmd_.io_cmd_fog_lamp)
      sendData_u_io[1] |= 0x40;

    if (current_io_cmd_.io_cmd_speaker)
      sendData_u_io[2] |= 0x01;

    sendData_u_io[3] = 0;
    sendData_u_io[4] = 0;

    if (current_io_cmd_.io_cmd_dis_charge)
      sendData_u_io[5] |= 0x01;

    count_1++;
    if (count_1 == 16)
      count_1 = 0;

    sendData_u_io[6] = count_1 << 4;

    sendData_u_io[7] = sendData_u_io[0] ^ sendData_u_io[1] ^ sendData_u_io[2] ^
                       sendData_u_io[3] ^ sendData_u_io[4] ^ sendData_u_io[5] ^ sendData_u_io[6];

    send_frames_[0].can_id = 0x18C4D7D0 | CAN_EFF_FLAG;
    send_frames_[0].can_dlc = 8;

    memcpy(send_frames_[0].data, sendData_u_io, 8);

    int ret = write(can_socket_, &send_frames_[0], sizeof(send_frames_[0]));
    if (ret <= 0)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), "send IO message failed, error code: " << ret);
    }
  }

  void CanControl::sendCtrlCommand()
  {
    static unsigned char count_ctrl = 0;
    unsigned char sendData_u_vel[8];
    memset(sendData_u_vel, 0, 8);

    unsigned short vel = 0;
    if (current_ctrl_cmd_.ctrl_cmd_velocity < 0)
      vel = 0;
    else
      vel = (unsigned short)(current_ctrl_cmd_.ctrl_cmd_velocity * 1000);

    short angular = (short)(current_ctrl_cmd_.ctrl_cmd_steering * 100);

    sendData_u_vel[0] = (0x0f & current_ctrl_cmd_.ctrl_cmd_gear);
    sendData_u_vel[0] |= (unsigned char)((vel & 0x0f) << 4);

    sendData_u_vel[1] = (unsigned char)((vel >> 4) & 0xff);

    sendData_u_vel[2] = (unsigned char)((vel >> 12) & 0x0f);
    sendData_u_vel[2] |= (unsigned char)((angular & 0x0f) << 4);

    sendData_u_vel[3] = (unsigned char)((angular >> 4) & 0xff);

    sendData_u_vel[4] = (unsigned char)((angular >> 12) & 0x0f);
    sendData_u_vel[4] |= (unsigned char)((current_ctrl_cmd_.ctrl_cmd_brake & 0x0f) << 4);

    sendData_u_vel[5] = (unsigned char)((current_ctrl_cmd_.ctrl_cmd_brake >> 4) & 0x0f);

    count_ctrl++;
    if (count_ctrl > 15)
      count_ctrl = 0;

    sendData_u_vel[6] = count_ctrl << 4;

    sendData_u_vel[7] = sendData_u_vel[0] ^ sendData_u_vel[1] ^ sendData_u_vel[2] ^
                        sendData_u_vel[3] ^ sendData_u_vel[4] ^ sendData_u_vel[5] ^ sendData_u_vel[6];

    send_frames_[0].can_id = 0x18C4D2D0 | CAN_EFF_FLAG;
    send_frames_[0].can_dlc = 8;
    memcpy(send_frames_[0].data, sendData_u_vel, 8);

    int ret = write(can_socket_, &send_frames_[0], sizeof(send_frames_[0]));
    if (ret <= 0)
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), "Send Ctrl ID failed: " << ret);
  }

  void CanControl::io_cmd_callback(const yhs_can_interfaces::msg::IoCmd::SharedPtr io_cmd_msg)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    current_io_cmd_ = *io_cmd_msg;
    sendIoCommand();
  }

  void CanControl::ctrl_cmd_callback(const yhs_can_interfaces::msg::CtrlCmd::SharedPtr ctrl_cmd_msg)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    current_ctrl_cmd_ = *ctrl_cmd_msg;
  }

  void CanControl::imu_callback(const sensor_msgs::msg::Imu::SharedPtr imu_msg)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    last_imu_time_ = node_->get_clock()->now();
    imu_active_ = true;

    double qx = imu_msg->orientation.x;
    double qy = imu_msg->orientation.y;
    double qz = imu_msg->orientation.z;
    double qw = imu_msg->orientation.w;
    
    imu_roll_ = atan2(2.0 * (qw * qx + qy * qz), 1.0 - 2.0 * (qx * qx + qy * qy));
    imu_pitch_ = asin(2.0 * (qw * qy - qz * qx));
    imu_yaw_ = atan2(2.0 * (qw * qz + qx * qy), 1.0 - 2.0 * (qy * qy + qz * qz));
  }

  void CanControl::timer_callback()
  {
    static int loop_count = 0;
    std::lock_guard<std::mutex> lock(mutex_);
    sendCtrlCommand();
    if (loop_count % 5 == 0)
    {
      sendIoCommand();
    }
    loop_count++;
  }

  bool CanControl::wait_for_can_frame()
  {
    struct timeval tv;
    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(can_socket_, &rdfs);
    tv.tv_sec = 0;
    tv.tv_usec = 30000;

    int ret = select(can_socket_ + 1, &rdfs, NULL, NULL, &tv);
    if (ret == -1)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), "Error waiting for CAN frame: " << std::strerror(errno));
      return false;
    }
    else if (ret == 0)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), "Timeout waiting for CAN frame! Please check whether the can0 setting is correct, \
whether the can line is connected correctly, and whether the chassis is powered on.");
      return false;
    }
    else
    {
      return true;
    }
  }

  void CanControl::recvData()
  {
    while (rclcpp::ok())
    {
      if (!wait_for_can_frame())
        continue;

      if (read(can_socket_, &recv_frames_[0], sizeof(recv_frames_[0])) >= 0)
      {
        switch (recv_frames_[0].can_id)
        {
        case 0x18C4D2EF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::CtrlFb msg;
          msg.ctrl_fb_gear = 0x0f & recv_frames_[0].data[0];

          msg.ctrl_fb_velocity = (float)((unsigned short)((recv_frames_[0].data[2] & 0x0f) << 12 | recv_frames_[0].data[1] << 4 | (recv_frames_[0].data[0] & 0xf0) >> 4)) / 1000;

          msg.ctrl_fb_steering = (float)((short)((recv_frames_[0].data[4] & 0x0f) << 12 | recv_frames_[0].data[3] << 4 | (recv_frames_[0].data[2] & 0xf0) >> 4)) / 100;

          msg.ctrl_fb_brake = (recv_frames_[0].data[4] & 0x30) >> 4;

          msg.ctrl_fb_mode = (recv_frames_[0].data[4] & 0xc0) >> 6;

          msg.ctrl_fb_remote_st = (recv_frames_[0].data[5] & 0x01) >> 7;

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // ctrl_fb_publisher_->publish(msg);
            if (msg.ctrl_fb_gear == 2)
              msg.ctrl_fb_velocity = -msg.ctrl_fb_velocity;
            publish_odom(msg.ctrl_fb_velocity, msg.ctrl_fb_steering / 180 * 3.1415);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.ctrl_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        case 0x18C4D7EF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::LrWheelFb msg;
          msg.lr_wheel_fb_velocity = (float)((short)(recv_frames_[0].data[1] << 8 | recv_frames_[0].data[0])) / 1000;

          msg.lr_wheel_fb_pulse = (int)(recv_frames_[0].data[5] << 24 | recv_frames_[0].data[4] << 16 | recv_frames_[0].data[3] << 8 | recv_frames_[0].data[2]);

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // lr_wheel_fb_publisher_->publish(msg);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.lr_wheel_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        case 0x18C4D8EF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::RrWheelFb msg;
          msg.rr_wheel_fb_velocity = (float)((short)(recv_frames_[0].data[1] << 8 | recv_frames_[0].data[0])) / 1000;

          msg.rr_wheel_fb_pulse = (int)(recv_frames_[0].data[5] << 24 | recv_frames_[0].data[4] << 16 | recv_frames_[0].data[3] << 8 | recv_frames_[0].data[2]);

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // rr_wheel_fb_publisher_->publish(msg);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.rr_wheel_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        case 0x18C4DAEF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::IoFb msg;

          if (0x01 & recv_frames_[0].data[0])
            msg.io_fb_enable = true;
          else
            msg.io_fb_enable = false;

          if (0x01 & recv_frames_[0].data[1])
            msg.io_fb_lower_beam_headlamp = true;
          else
            msg.io_fb_lower_beam_headlamp = false;

          if (0x02 & recv_frames_[0].data[1])
            msg.io_fb_upper_beam_headlamp = true;
          else
            msg.io_fb_upper_beam_headlamp = false;

          msg.io_fb_turn_lamp = (0x0c & recv_frames_[0].data[1]) >> 2;

          if (0x10 & recv_frames_[0].data[1])
            msg.io_fb_braking_lamp = true;
          else
            msg.io_fb_braking_lamp = false;

          if (0x20 & recv_frames_[0].data[1])
            msg.io_fb_clearance_lamp = true;
          else
            msg.io_fb_clearance_lamp = false;

          if (0x40 & recv_frames_[0].data[1])
            msg.io_fb_fog_lamp = true;
          else
            msg.io_fb_fog_lamp = false;

          if (0x01 & recv_frames_[0].data[2])
            msg.io_fb_speaker = true;
          else
            msg.io_fb_speaker = false;

          if (0x01 & recv_frames_[0].data[3])
            msg.io_fb_fl_impact_sensor = true;
          else
            msg.io_fb_fl_impact_sensor = false;

          if (0x02 & recv_frames_[0].data[3])
            msg.io_fb_fm_impact_sensor = true;
          else
            msg.io_fb_fm_impact_sensor = false;

          if (0x04 & recv_frames_[0].data[3])
            msg.io_fb_fr_impact_sensor = true;
          else
            msg.io_fb_fr_impact_sensor = false;

          if (0x08 & recv_frames_[0].data[3])
            msg.io_fb_rl_impact_sensor = true;
          else
            msg.io_fb_rl_impact_sensor = false;

          if (0x10 & recv_frames_[0].data[3])
            msg.io_fb_rm_impact_sensor = true;
          else
            msg.io_fb_rm_impact_sensor = false;

          if (0x20 & recv_frames_[0].data[3])
            msg.io_fb_rr_impact_sensor = true;
          else
            msg.io_fb_rr_impact_sensor = false;

          if (0x01 & recv_frames_[0].data[4])
            msg.io_fb_fl_drop_sensor = true;
          else
            msg.io_fb_fl_drop_sensor = false;

          if (0x02 & recv_frames_[0].data[4])
            msg.io_fb_fm_drop_sensor = true;
          else
            msg.io_fb_fm_drop_sensor = false;

          if (0x04 & recv_frames_[0].data[4])
            msg.io_fb_fr_drop_sensor = true;
          else
            msg.io_fb_fr_drop_sensor = false;

          if (0x08 & recv_frames_[0].data[4])
            msg.io_fb_rl_drop_sensor = true;
          else
            msg.io_fb_rl_drop_sensor = false;

          if (0x10 & recv_frames_[0].data[4])
            msg.io_fb_rm_drop_sensor = true;
          else
            msg.io_fb_rm_drop_sensor = false;

          if (0x20 & recv_frames_[0].data[4])
            msg.io_fb_rr_drop_sensor = true;
          else
            msg.io_fb_rr_drop_sensor = false;

          msg.io_fb_dis_charge = 0x01 & recv_frames_[0].data[5];

          msg.io_fb_charge_en = 0x02 & recv_frames_[0].data[1];

          if (0x10 & recv_frames_[0].data[5])
            msg.io_fb_scram_st = true;
          else
            msg.io_fb_scram_st = false;

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // io_fb_publisher_->publish(msg);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.io_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        case 0x18C4DEEF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::OdoFb msg;
          msg.odo_fb_accumulative_mileage = (float)((int)(recv_frames_[0].data[3] << 24 | recv_frames_[0].data[2] << 16 | recv_frames_[0].data[1] << 8 | recv_frames_[0].data[0])) / 1000;

          msg.odo_fb_accumulative_angular = (float)((int)(recv_frames_[0].data[7] << 24 | recv_frames_[0].data[6] << 16 | recv_frames_[0].data[5] << 8 | recv_frames_[0].data[4])) / 1000;

          odo_fb_publisher_->publish(msg);

          break;
        }

        case 0x18C4E1EF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::BmsInfoFb msg;
          msg.bms_info_voltage = (float)((unsigned short)(recv_frames_[0].data[1] << 8 | recv_frames_[0].data[0])) / 100;

          msg.bms_info_current = (float)((short)(recv_frames_[0].data[3] << 8 | recv_frames_[0].data[2])) / 100;

          msg.bms_info_remaining_capacity = (float)((unsigned short)(recv_frames_[0].data[5] << 8 | recv_frames_[0].data[4])) / 100;

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // bms_info_fb_publisher_->publish(msg);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.bms_info_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        case 0x18C4E2EF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::BmsFlagInfoFb msg;
          msg.bms_flag_info_soc = recv_frames_[0].data[0];

          if (0x01 & recv_frames_[0].data[1])
            msg.bms_flag_info_single_ov = true;
          else
            msg.bms_flag_info_single_ov = false;

          if (0x02 & recv_frames_[0].data[1])
            msg.bms_flag_info_single_uv = true;
          else
            msg.bms_flag_info_single_uv = false;

          if (0x04 & recv_frames_[0].data[1])
            msg.bms_flag_info_ov = true;
          else
            msg.bms_flag_info_ov = false;

          if (0x08 & recv_frames_[0].data[1])
            msg.bms_flag_info_uv = true;
          else
            msg.bms_flag_info_uv = false;

          if (0x10 & recv_frames_[0].data[1])
            msg.bms_flag_info_charge_ot = true;
          else
            msg.bms_flag_info_charge_ot = false;

          if (0x20 & recv_frames_[0].data[1])
            msg.bms_flag_info_charge_ut = true;
          else
            msg.bms_flag_info_charge_ut = false;

          if (0x40 & recv_frames_[0].data[1])
            msg.bms_flag_info_discharge_ot = true;
          else
            msg.bms_flag_info_discharge_ot = false;

          if (0x80 & recv_frames_[0].data[1])
            msg.bms_flag_info_discharge_ut = true;
          else
            msg.bms_flag_info_discharge_ut = false;

          if (0x01 & recv_frames_[0].data[2])
            msg.bms_flag_info_charge_oc = true;
          else
            msg.bms_flag_info_charge_oc = false;

          if (0x02 & recv_frames_[0].data[2])
            msg.bms_flag_info_discharge_oc = true;
          else
            msg.bms_flag_info_discharge_oc = false;

          if (0x04 & recv_frames_[0].data[2])
            msg.bms_flag_info_short = true;
          else
            msg.bms_flag_info_short = false;

          if (0x08 & recv_frames_[0].data[2])
            msg.bms_flag_info_ic_error = true;
          else
            msg.bms_flag_info_ic_error = false;

          if (0x10 & recv_frames_[0].data[2])
            msg.bms_flag_info_lock_mos = true;
          else
            msg.bms_flag_info_lock_mos = false;

          msg.bms_flag_info_charge_st = (recv_frames_[0].data[2] >> 5) & 0x03;

          if (0x80 & recv_frames_[0].data[2])
            msg.bms_flag_info_soc_warning = true;
          else
            msg.bms_flag_info_soc_warning = false;

          if (0x01 & recv_frames_[0].data[3])
            msg.bms_flag_info_soc_low_protection = true;
          else
            msg.bms_flag_info_soc_low_protection = false;

          msg.bms_flag_info_hight_temperature = (float)((short)(recv_frames_[0].data[4] << 4 | recv_frames_[0].data[3] >> 4)) / 10;

          msg.bms_flag_info_low_temperature = (float)((short)((recv_frames_[0].data[6] & 0x0f) << 8 | recv_frames_[0].data[5])) / 10;

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // bms_flag_info_fb_publisher_->publish(msg);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.bms_flag_info_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        case 0x18C4EAEF | CAN_EFF_FLAG:
        {
          yhs_can_interfaces::msg::VehDiagFb msg;
          msg.veh_fb_fault_level = 0x0f & recv_frames_[0].data[0];

          if (0x10 & recv_frames_[0].data[0])
            msg.veh_fb_auto_can_ctrl_cmd = true;
          else
            msg.veh_fb_auto_can_ctrl_cmd = false;

          if (0x20 & recv_frames_[0].data[0])
            msg.veh_fb_auto_io_can_cmd = true;
          else
            msg.veh_fb_auto_io_can_cmd = false;

          if (0x01 & recv_frames_[0].data[1])
            msg.veh_fb_eps_dis_on_line = true;
          else
            msg.veh_fb_eps_dis_on_line = false;

          if (0x02 & recv_frames_[0].data[1])
            msg.veh_fb_eps_fault = true;
          else
            msg.veh_fb_eps_fault = false;

          if (0x04 & recv_frames_[0].data[1])
            msg.veh_fb_eps_mosf_et_ot = true;
          else
            msg.veh_fb_eps_mosf_et_ot = false;

          if (0x08 & recv_frames_[0].data[1])
            msg.veh_fb_eps_warning = true;
          else
            msg.veh_fb_eps_warning = false;

          if (0x10 & recv_frames_[0].data[1])
            msg.veh_fb_eps_dis_work = true;
          else
            msg.veh_fb_eps_dis_work = false;

          if (0x20 & recv_frames_[0].data[1])
            msg.veh_fb_eps_over_current = true;
          else
            msg.veh_fb_eps_over_current = false;

          msg.veh_fb_st_reserve = ((recv_frames_[0].data[1] & 0x03) << 4) | (recv_frames_[0].data[2] & 0x0f);

          if (0x10 & recv_frames_[0].data[2])
            msg.veh_fb_ehb_ecu_fault = true;
          else
            msg.veh_fb_ehb_ecu_fault = false;

          if (0x20 & recv_frames_[0].data[2])
            msg.veh_fb_ehb_dis_on_line = true;
          else
            msg.veh_fb_ehb_dis_on_line = false;

          if (0x40 & recv_frames_[0].data[2])
            msg.veh_fb_ehb_work_model_fault = true;
          else
            msg.veh_fb_ehb_work_model_fault = false;

          if (0x80 & recv_frames_[0].data[2])
            msg.veh_fb_ehb_dis_en = true;
          else
            msg.veh_fb_ehb_dis_en = false;

          if (0x01 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_anguler_fault = true;
          else
            msg.veh_fb_ehb_anguler_fault = false;

          if (0x02 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_ot = true;
          else
            msg.veh_fb_ehb_ot = false;

          if (0x04 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_power_fault = true;
          else
            msg.veh_fb_ehb_power_fault = false;

          if (0x08 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_sensor_abnomal = true;
          else
            msg.veh_fb_ehb_sensor_abnomal = false;

          if (0x10 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_motor_fault = true;
          else
            msg.veh_fb_ehb_motor_fault = false;

          if (0x20 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_oil_press_sensor_fault = true;
          else
            msg.veh_fb_ehb_oil_press_sensor_fault = false;

          if (0x40 & recv_frames_[0].data[3])
            msg.veh_fb_ehb_oil_fault = true;
          else
            msg.veh_fb_ehb_oil_fault = false;

          if (0x80 & recv_frames_[0].data[3])
            msg.veh_fb_bra_reserve = true;
          else
            msg.veh_fb_bra_reserve = false;

          msg.veh_fb_ld_rv_mcu_fault = 0x3f & recv_frames_[0].data[4];
          msg.veh_fb_rd_rv_mcu_fault = (recv_frames_[0].data[5] & 0x0f << 2) | (recv_frames_[0].data[4] >> 6);

          if (0x10 & recv_frames_[0].data[5])
            msg.veh_fb_aux_bms_dis_on_line = true;
          else
            msg.veh_fb_aux_bms_dis_on_line = false;

          if (0x80 & recv_frames_[0].data[5])
            msg.veh_fb_aux_remote_dis_on_line = true;
          else
            msg.veh_fb_aux_remote_dis_on_line = false;

          msg.veh_fb_aux_reserve = recv_frames_[0].data[6] & 0x0f;

          unsigned char crc = recv_frames_[0].data[0] ^ recv_frames_[0].data[1] ^ recv_frames_[0].data[2] ^
                              recv_frames_[0].data[3] ^ recv_frames_[0].data[4] ^ recv_frames_[0].data[5] ^ recv_frames_[0].data[6];

          if (crc == recv_frames_[0].data[7])
          {
            // veh_diag_fb_publisher_->publish(msg);

            std::lock_guard<std::mutex> lock(mutex_);
            current_chassis_info_.header.stamp = node_->get_clock()->now();
            current_chassis_info_.veh_diag_fb = msg;
            chassis_info_fb_publisher_->publish(current_chassis_info_);
          }

          break;
        }

        static unsigned short ultra_data[8] = {0};
        case 0x18C4E8EF | CAN_EFF_FLAG:
        {
          ultra_data[0] = (unsigned short)((recv_frames_[0].data[1] & 0x0f) << 8 | recv_frames_[0].data[0]);
          ultra_data[1] = (unsigned short)(recv_frames_[0].data[2] << 4 | ((recv_frames_[0].data[1] & 0xf0) >> 4));

          ultra_data[2] = (unsigned short)((recv_frames_[0].data[4] & 0x0f) << 8 | recv_frames_[0].data[3]);
          ultra_data[3] = (unsigned short)(recv_frames_[0].data[5] << 4 | ((recv_frames_[0].data[4] & 0xf0) >> 4));
          break;
        }

        case 0x18C4E9EF | CAN_EFF_FLAG:
        {
          ultra_data[4] = (unsigned short)((recv_frames_[0].data[1] & 0x0f) << 8 | recv_frames_[0].data[0]);
          ultra_data[5] = (unsigned short)(recv_frames_[0].data[2] << 4 | ((recv_frames_[0].data[1] & 0xf0) >> 4));

          ultra_data[6] = (unsigned short)((recv_frames_[0].data[4] & 0x0f) << 8 | recv_frames_[0].data[3]);
          ultra_data[7] = (unsigned short)(recv_frames_[0].data[5] << 4 | ((recv_frames_[0].data[4] & 0xf0) >> 4));

          yhs_can_interfaces::msg::Ultrasonic ultra_msg;

          if (ultrasonic_number_.size() >= 1)
            ultra_msg.ultrasonic_fb_01 = ultra_data[ultrasonic_number_[0]];
          if (ultrasonic_number_.size() >= 2)
            ultra_msg.ultrasonic_fb_02 = ultra_data[ultrasonic_number_[1]];
          if (ultrasonic_number_.size() >= 3)
            ultra_msg.ultrasonic_fb_03 = ultra_data[ultrasonic_number_[2]];
          if (ultrasonic_number_.size() >= 4)
            ultra_msg.ultrasonic_fb_04 = ultra_data[ultrasonic_number_[3]];
          if (ultrasonic_number_.size() >= 5)
            ultra_msg.ultrasonic_fb_05 = ultra_data[ultrasonic_number_[4]];
          if (ultrasonic_number_.size() >= 6)
            ultra_msg.ultrasonic_fb_06 = ultra_data[ultrasonic_number_[5]];
          if (ultrasonic_number_.size() >= 7)
            ultra_msg.ultrasonic_fb_07 = ultra_data[ultrasonic_number_[6]];
          if (ultrasonic_number_.size() >= 8)
            ultra_msg.ultrasonic_fb_08 = ultra_data[ultrasonic_number_[7]];

          // ultrasonic_publisher_->publish(ultra_msg);

          std::lock_guard<std::mutex> lock(mutex_);
          current_chassis_info_.header.stamp = node_->get_clock()->now();
          current_chassis_info_.ultrasonic = ultra_msg;
          chassis_info_fb_publisher_->publish(current_chassis_info_);
        }

        default:
          break;
        }
      }
    }
  }

  void CanControl::publish_odom(const double velocity, const double steering)
  {
    static double x = 0.0;
    static double y = 0.0;
    static double th = 0.0;

    double x_mid = 0.0;
    double y_mid = 0.0;

    static rclcpp::Time last_time = rclcpp::Time(0, 0, RCL_ROS_TIME);
    rclcpp::Time current_time = node_->get_clock()->now();

    double vx = velocity;
    double vth = vx * tan(steering) / wheel_base_;

    double dt = (current_time - last_time).seconds();

    bool is_imu_active = (current_time - last_imu_time_).seconds() < 0.2;
    if (is_imu_active)
    {
      std::lock_guodomard<std::mutex> lock(mutex_);
      th = imu_yaw_;
    }
    else
    {
      th += vth * dt;
    }

    double delta_x = (vx * cos(th)) * dt;
    double delta_y = (vx * sin(th)) * dt;

    x += delta_x;
    y += delta_y;

    x_mid = x + wheel_base_ / 2 * cos(th);
    y_mid = y + wheel_base_ / 2 * sin(th);

    tf2::Quaternion quat;
    if (is_imu_active)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      quat.setRPY(imu_roll_, imu_pitch_, th);
    }
    else
    {
      quat.setRPY(0, 0, th);
    }

    nav_msgs::msg::Odometry odom;
    odom.header.stamp = current_time;
    odom.header.frame_id = odom_frame_;

    odom.pose.pose.position.x = x_mid;
    odom.pose.pose.position.y = y_mid;
    odom.pose.pose.position.z = 0.0;
    odom.pose.pose.orientation.x = quat.x();
    odom.pose.pose.orientation.y = quat.y();
    odom.pose.pose.orientation.z = quat.z();
    odom.pose.pose.orientation.w = quat.w();
    
    odom.child_frame_id = base_link_frame_;
    odom.twist.twist.linear.x = vx;
    odom.twist.twist.linear.y = 0.0;
    odom.twist.twist.angular.z = vth;

    odom.pose.covariance[0] = 0.1;
    odom.pose.covariance[7] = 0.1;
    odom.pose.covariance[35] = 0.2;

    odom.pose.covariance[14] = 1e10;
    odom.pose.covariance[21] = 1e10;
    odom.pose.covariance[28] = 1e10;

    odom_pub_->publish(odom);

    last_time = current_time;
  }

  CanControl::~CanControl()
  {
  }

  bool CanControl::run()
  {
    can_socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket_ < 0)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), ">>open can device error!");
      return false;
    }
    else
    {
      RCLCPP_INFO_STREAM(rclcpp::get_logger("yhs_can_control_node"), ">>open can device success!");
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, if_name_.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(can_socket_, SIOCGIFINDEX, &ifr) < 0)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), ">>get interface index error!");
      return false;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(can_socket_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("yhs_can_control_node"), ">>bind device handler error!");
      return false;
    }

    recv_thread_ = std::thread(&CanControl::recvData, this);

    RCLCPP_INFO_STREAM(rclcpp::get_logger("yhs_can_control_node"), "yhs_can_control_node initialized successfully");

    return true;
  }

  void CanControl::stop()
  {
    if (can_socket_ >= 0)
    {
      close(can_socket_);
      can_socket_ = -1;
    }

    if (recv_thread_.joinable())
    {
      recv_thread_.join();
    }
  }
}

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("yhs_can_control_node");

  yhs::CanControl cancontrol(node);
  if (!cancontrol.run())
  {
    RCLCPP_ERROR(node->get_logger(), "Failed to initialize yhs_can_control_node");
    return 0;
  }

  rclcpp::spin(node);

  cancontrol.stop();
  RCLCPP_INFO(node->get_logger(), "yhs_can_control_node stopped");

  rclcpp::shutdown();

  return 0;
}
