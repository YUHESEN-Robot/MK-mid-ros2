#include <rclcpp/rclcpp.hpp>
#include <yhs_can_interfaces/msg/ctrl_cmd.hpp>
#include <yhs_can_interfaces/msg/io_cmd.hpp>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define KEYCODE_W 0x77
#define KEYCODE_A 0x61
#define KEYCODE_S 0x73
#define KEYCODE_D 0x64
#define KEYCODE_SPACE 0x20

#define KEYCODE_1 0x31
#define KEYCODE_2 0x32
#define KEYCODE_3 0x33

int kfd = 0;
struct termios cooked, raw;

void quit(int sig)
{
  (void)sig;
  tcsetattr(kfd, TCSANOW, &cooked);
  rclcpp::shutdown();
  exit(0);
}

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("yhs_keyboard_node");

  auto ctrl_pub = node->create_publisher<yhs_can_interfaces::msg::CtrlCmd>("ctrl_cmd", 10);
  auto io_pub = node->create_publisher<yhs_can_interfaces::msg::IoCmd>("io_cmd", 10);

  signal(SIGINT, quit);

  tcgetattr(kfd, &cooked);
  memcpy(&raw, &cooked, sizeof(struct termios));

  raw.c_lflag &= ~(ICANON | ECHO);

  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(kfd, TCSANOW, &raw);

  puts("Reading from keyboard");
  puts("---------------------------");
  puts("Use 'WASD' to move");
  puts("   W   ");
  puts(" A S D ");
  puts("Space to FORCE STOP");
  puts("Release keys to auto-stop");
  puts("1: Toggle Lower Beam");
  puts("2: Turn Left | 3: Turn Right");

  char c;

  float target_speed = 0.0;
  float target_turn = 0.0;

  float max_speed = 0.5;
  float max_turn = 10.0;
  float speed_step = 0.1;
  float turn_step = 5.0;

  yhs_can_interfaces::msg::IoCmd io_msg;
  io_msg.io_cmd_enable = true;

  bool is_moving = false;
  rclcpp::Time last_key_time = node->get_clock()->now();
  rclcpp::Duration timeout_duration(std::chrono::milliseconds(200));

  while (rclcpp::ok())
  {
    int n = read(kfd, &c, 1);

    bool dirty_motion = false;
    bool dirty_io = false;

    if (n > 0)
    {
      last_key_time = node->get_clock()->now();

      switch (c)
      {
      case KEYCODE_W:
        target_speed = max_speed;
        target_turn = 0;
        dirty_motion = true;
        break;
      case KEYCODE_S:
        target_speed = -max_speed;
        target_turn = 0;
        dirty_motion = true;
        break;
      case KEYCODE_A:
        target_speed = 0.1;
        target_turn = max_turn;
        dirty_motion = true;
        break;
      case KEYCODE_D:
        target_speed = 0.1;
        target_turn = -max_turn;
        dirty_motion = true;
        break;
      case KEYCODE_SPACE:
        target_speed = 0;
        target_turn = 0;
        dirty_motion = true;
        break;

      case KEYCODE_1:
        io_msg.io_cmd_lower_beam_headlamp = !io_msg.io_cmd_lower_beam_headlamp;
        dirty_io = true;
        break;
      case KEYCODE_2:
        io_msg.io_cmd_turn_lamp = (io_msg.io_cmd_turn_lamp == 1) ? 0 : 1;
        dirty_io = true;
        break;
      case KEYCODE_3:
        io_msg.io_cmd_turn_lamp = (io_msg.io_cmd_turn_lamp == 2) ? 0 : 2;
        dirty_io = true;
        break;
      }
    }

    if (is_moving && (node->get_clock()->now() - last_key_time) > timeout_duration)
    {
      target_speed = 0.0;
      target_turn = 0.0;
      dirty_motion = true;
      is_moving = false;
    }

    if (dirty_io)
    {
      io_pub->publish(io_msg);
      RCLCPP_INFO(node->get_logger(), "IO Updated: Beam=%d, Turn=%d", io_msg.io_cmd_lower_beam_headlamp, io_msg.io_cmd_turn_lamp);
    }

    if (dirty_motion)
    {
      yhs_can_interfaces::msg::CtrlCmd cmd_msg;
      cmd_msg.ctrl_cmd_velocity = target_speed;
      cmd_msg.ctrl_cmd_steering = target_turn;

      if (target_speed > 0.001)
        cmd_msg.ctrl_cmd_gear = 0x04;
      else if (target_speed < -0.001)
        cmd_msg.ctrl_cmd_gear = 0x02;
      else
        cmd_msg.ctrl_cmd_gear = 0x03;

      ctrl_pub->publish(cmd_msg);

      if (std::abs(target_speed) > 0.001 || std::abs(target_turn) > 0.001)
      {
        is_moving = true;
      }
    }

    rclcpp::spin_some(node);
  }

  return 0;
}
