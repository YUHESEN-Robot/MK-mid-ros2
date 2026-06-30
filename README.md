### English | [中文](README(中文).md)

# MK-mid-ros2

## 1. Environment confirmation:
####      1.1. System:
            Ubuntu 22.04
####      1.2. ROS version:
            Humble

## 2.Computer and Industrial PC Boot Configuration
####      2.1. ensure rc.local file:
    2.1.1. rc.local File Verification and Setup
        Check if the rc.local file exists in the /etc/ directory:
    2.1.2. If not present, execute:
                 sudo cp rc.local /etc  
    2.1.3. If present, insert the following content above the exit 0 line in the rc.local file, then save:
                 sleep 2  
                 sudo ip link set can0 type can bitrate 500000  
                 sudo ip link set can0 up        
####      2.2. Pre-Boot CAN Card Connection Verification​
　　Ensure the CAN card is physically connected to the USB port of the industrial PC prior to system startup or reboot.      
####      2.3. Configuration Success Indication​
　　A successful setup is confirmed when both the RX (Receive) and TX (Transmit) indicators illuminate, validating proper communication establishment.

### 3.The subsequent section will address the process of establishing a connection between the CAN card and the system.
####      3.1. Chassis CAN card connection:
    Specifically, Chassis CAN-H is to be connected to CANH of the CAN card connector plug, while Chassis CAN-L is to be connected to CANL of the CAN card connector plug.
    The following diagram illustrates the recommended configuration:
            
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/CAN_Connection.png?raw=true)

####      3.2. Computer and robot communication connection:
    Subsequent to the completion of the aforementioned connection, the USB end of the CAN card cable should be inserted directly into the computer's USB port, thereby establishing a connection between the CAN card and the computer. Upon completion of the connection, the PWR light will illuminate.
####      3.3. Methodology for checking the success of USB connection:
    To initiate this process, one must access the terminal and enter the command: 
            lsusb
    If the output information displays content resembling that depicted in the following image, it can be deduced that CAN can be utilized without issue.

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/terminal_state.png?raw=true)

####      3.4. Communication initiation:
    To initiate this process, it is necessary to open a new terminal and enter the following commands: 
            sudo ip link set can0 type can bitrate 500000 & sudo ip link set can0 up
    If the chassis is in the power-up state, the blue RX light will blink, and the red TX light will be on at the same time.The blinking of the RX light indicates that the Can0 card receives data, and the blinking of the TX light indicates that it sends data.

## 4. Communication Data Viewing
####      4.1 To install the can-utils tool, the following command should be executed:
            sudo apt-get install can-utils
####      4.2 View data command:
            candump can0
####      4.3. Terminal print the information normally:

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/candump_print.png?raw=true)

## 5. ROS2 program operation
####      5.1. File structure:

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/doc_tree.png?raw=true)

####      5.2.Compile workspace:  
    Open the terminal in your workspace/enter your workspace in the terminal, let the terminal be in the workspace level, then execute the command: 
            colcon build
            
####      5.3.Setting temporary environment variables:
    Execute the command in the terminal where you want to run the program:  
            source ~/your workspace (example: 5.1-ros2_ws)/install/setup.bash  
    Or, to keep the terminal where you want to run the program at the workspace level, execute the command:    
            source install/setup.bash  

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/source.png?raw=true)

####      5.4. Run:  
    Execute the following command, in the terminal where you have just entered the temporary environment variable. 
            ros2 launch yhs_can_control yhs_can_control.launch.py   
####      5.5. The terminal prints out a successful run message:  
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/node_print.png?raw=true)

## 6. Motion Test:
####      6.1. Before testing, it is recommended to lift the chassis or send a minimal speed command to the chassis (e.g., 0.03).
####      6.2. Open a new terminal, navigate to your workspace directory (e.g., 5.1-ros2_ws), and execute:
            source install/setup.bash
    OR (without entering the workspace directory):
            source ~/your workspace (example: 5.1-ros2_ws)/install/setup.bash
####      6.5. Send motion commands to control the chassis:
    6.5.1. Open a new terminal, navigate to your workspace directory, and set temporary environment variables:
            source install/setup.bash
    OR (without entering the workspace directory):
            source ~/your workspace(example: 5.1-ros2_ws)/install/setup.bash
    6.5.2. In a terminal with temporary environment variables configured, execute the rqt_publisher tool and ensure the remote controller is switched to command control mode.
            ros2 run rqt_publisher rqt_publisher
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/rqt_tool.png?raw=true)  

## 7. Topic Description
> Note: Some IO and sensor fields are reserved functions, which will not take effect without corresponding hardware on chassis. Motion control topic requires publish frequency ≥30Hz.

### 7.1 Chassis Motion Control (Command Publish)
- Message Type: `yhs_can_interfaces/msg/CtrlCmd`
- Topic Name: `/ctrl_cmd`
- Recommended Publish Frequency: ≥30Hz

| Variable Name | Data Type | Description |
|---------------|-----------|-------------|
| ctrl_cmd_gear | uint8 | Target gear<br>0: disable<br>1: P (Park)<br>2: R (Reverse)<br>3: N (Neutral)<br>4: D (Drive)<br>Other values are invalid, chassis defaults to park mode |
| ctrl_cmd_velocity | float32 | Target vehicle speed, unit: m/s |
| ctrl_cmd_steering | float32 | Target steering angle, unit: ° |
| ctrl_cmd_brake | uint8 | Target brake force ratio, unit: % |

### 7.2 IO Peripheral Control (Command Publish)
- Message Type: `yhs_can_interfaces/msg/IoCmd`
- Topic Name: `/io_cmd`

| Variable Name | Data Type | Description |
|---------------|-----------|-------------|
| io_cmd_enable | bool | Global IO enable switch<br>0: Off, lights & turn signals controlled by VCU<br>1: On, lights & turn signals controlled by CAN command |
| io_cmd_lower_beam_headlamp | bool | Low beam headlight (reserved)<br>0=Off, 1=On |
| io_cmd_upper_beam_headlamp | bool | High beam headlight (reserved)<br>0=Off, 1=On |
| io_cmd_turn_lamp | uint8 | Turn signal / hazard warning light<br>0: All off<br>1: Left turn signal<br>2: Right turn signal<br>3: Hazard warning light<br>Priority: Single turn signal > hazard light |
| io_cmd_braking_lamp | bool | Brake light switch (reserved) |
| io_cmd_clearance_lamp | bool | Marker light switch (reserved) |
| io_cmd_fog_lamp | bool | Fog light switch (reserved) |
| io_cmd_speaker | bool | Horn speaker switch (reserved) |
| io_cmd_dis_charge | bool | Forced HV power-up flag during charging<br>When enabled while charging, 48V high voltage can be activated for driving; vehicle reverse is prohibited when charging + this flag is active |

### 7.3 Chassis Feedback Subscription (Receive Chassis Status)
- Message Type: `yhs_can_interfaces/msg/ChassisInfoFb`
- Topic Name: `/chassis_info_fb`

| Group Field | Variable Name | Data Type | Description |
|-------------|---------------|-----------|-------------|
| ctrl_fb | ctrl_fb_gear | uint8 | Actual current gear, same definition as 7.1 |
| ctrl_fb | ctrl_fb_velocity | float32 | Real-time vehicle speed, unit: m/s |
| ctrl_fb | ctrl_fb_steering | float32 | Real-time steering angle, unit: °, resolution 0.01°/bit |
| ctrl_fb | ctrl_fb_brake | uint8 | Current brake status (reserved), unit: % |
| ctrl_fb | ctrl_fb_remote_st | bool | Remote controller status: 0 disconnected, 1 connected |
| ctrl_fb | ctrl_fb_mode | uint8 | Vehicle operation mode<br>0x0: auto<br>0x1: remote<br>0x2: stop |
| io_fb | io_fb_enable | bool | Global IO enable feedback status |
| io_fb | io_fb_lower_beam_headlamp | bool | Low beam status feedback (reserved) |
| io_fb | io_fb_upper_beam_headlamp | bool | High beam status feedback (reserved) |
| io_fb | io_fb_turn_lamp | int8 | Turn signal status feedback, same value definition as publish command |
| io_fb | io_fb_braking_lamp | bool | Brake light status feedback |
| io_fb | io_fb_clearance_lamp | bool | Marker light status feedback (reserved) |
| io_fb | io_fb_fog_lamp | bool | Fog light status feedback (reserved) |
| io_fb | io_fb_speaker | bool | Horn speaker status feedback (reserved) |
| io_fb | io_fb_fm_impact_sensor | bool | Front-middle collision strip trigger feedback: 0 no trigger, 1 triggered |
| io_fb | io_fb_rm_impact_sensor | bool | Rear-middle collision strip trigger feedback |
| io_fb | io_fb_fl_impact_sensor | bool | Front-left collision strip (reserved) |
| io_fb | io_fb_rl_impact_sensor | bool | Rear-left collision strip (reserved) |
| io_fb | io_fb_fr_impact_sensor | bool | Front-right collision strip (reserved) |
| io_fb | io_fb_rr_impact_sensor | bool | Rear-right collision strip (reserved) |
| io_fb | io_fb_fl_drop_sensor | bool | Front-left drop sensor (reserved) |
| io_fb | io_fb_rl_drop_sensor | bool | Rear-left drop sensor (reserved) |
| io_fb | io_fb_fm_drop_sensor | bool | Front-middle drop sensor (reserved) |
| io_fb | io_fb_rm_drop_sensor | bool | Rear-middle drop sensor (reserved) |
| io_fb | io_fb_fr_drop_sensor | bool | Front-right drop sensor (reserved) |
| io_fb | io_fb_rr_drop_sensor | bool | Rear-right drop sensor (reserved) |
| io_fb | io_fb_dis_charge | bool | Forced HV power-up flag feedback |
| io_fb | io_fb_charge_en | bool | Charger contact detection flag: 1 = vehicle touches charging pile |
| io_fb | io_fb_scram_st | bool | E-stop status: 0 normal, 1 emergency stop pressed |
| lr_wheel_fb | lr_wheel_fb_velocity | float32 | Real-time left rear wheel speed, unit: m/s |
| lr_wheel_fb | lr_wheel_fb_pulse | int32 | Cumulative left rear wheel pulse; 200000 pulses per wheel revolution (2500 line encoder, 4x interpolation, 20 reduction ratio) |
| rr_wheel_fb | rr_wheel_fb_velocity | float32 | Real-time right rear wheel speed, unit: m/s |
| rr_wheel_fb | rr_wheel_fb_pulse | int32 | Cumulative right rear wheel pulse, same parameters as left wheel |
| odo_fb | odo_fb_accumulative_mileage | float32 | Total accumulated mileage |
| odo_fb | odo_fb_accumulative_angular | float32 | Total accumulated steering angle (reserved) |
| bms_flag_info_fb | bms_flag_info_soc | uint8 | Battery remaining SOC, unit %, 1% per bit |
| bms_flag_info_fb | bms_flag_info_single_ov | bool | Cell overvoltage protection flag |
| bms_flag_info_fb | bms_flag_info_single_uv | bool | Cell undervoltage protection flag |
| bms_flag_info_fb | bms_flag_info_ov | bool | Pack overvoltage protection flag |
| bms_flag_info_fb | bms_flag_info_uv | bool | Pack undervoltage protection flag |
| bms_flag_info_fb | bms_flag_info_charge_ot | bool | Charging overtemperature protection |
| bms_flag_info_fb | bms_flag_info_charge_ut | bool | Charging low temperature protection |
| bms_flag_info_fb | bms_flag_info_discharge_ot | bool | Discharge overtemperature protection |
| bms_flag_info_fb | bms_flag_info_discharge_ut | bool | Discharge low temperature protection |
| bms_flag_info_fb | bms_flag_info_charge_oc | bool | Charging overcurrent protection |
| bms_flag_info_fb | bms_flag_info_discharge_oc | bool | Discharge overcurrent protection |
| bms_flag_info_fb | bms_flag_info_short | bool | Battery short-circuit protection |
| bms_flag_info_fb | bms_flag_info_ic_error | bool | BMS front-end detection IC fault |
| bms_flag_info_fb | bms_flag_info_lock_mos | bool | MOSFET software lock protection |
| bms_flag_info_fb | bms_flag_info_charge_st | uint8 | Charging state<br>0: Not charging<br>1: Manual charge<br>2: Front pile charge<br>3: Rear pile charge |
| bms_flag_info_fb | bms_flag_info_soc_warning | bool | Low SOC warning flag |
| bms_flag_info_fb | bms_flag_info_soc_low_protection | bool | Low SOC cut-off protection |
| bms_flag_info_fb | bms_flag_info_hight_temperature | float32 | Max battery temperature, unit ℃ |
| bms_flag_info_fb | bms_flag_info_low_temperature | float32 | Min battery temperature, unit ℃ |
| bms_info_fb | bms_info_voltage | float32 | Total battery pack voltage, unit V |
| bms_info_fb | bms_info_current | float32 | Real-time battery current, unit A |
| bms_info_fb | bms_info_remaining_capacity | float32 | Remaining battery capacity, unit Ah |
| veh_diag_fb | veh_fb_fault_level | uint8 | Vehicle fault level<br>0: No fault / 1/2/3: Fault severity |
| veh_diag_fb | veh_fb_auto_can_ctrl_cmd | bool | Auto motion CAN communication fault |
| veh_diag_fb | veh_fb_auto_io_can_cmd | bool | Auto IO CAN communication fault |
| veh_diag_fb | veh_fb_eps_dis_on_line | bool | EPS steering driver offline fault |
| veh_diag_fb | veh_fb_eps_fault | bool | General EPS fault |
| veh_diag_fb | veh_fb_eps_mosf_et_ot | bool | EPS power MOSFET over-temperature |
| veh_diag_fb | veh_fb_eps_warning | bool | EPS warning fault |
| veh_diag_fb | veh_fb_eps_dis_work | bool | EPS abnormal operation fault |
| veh_diag_fb | veh_fb_eps_over_current | bool | EPS over-current fault |
| veh_diag_fb | veh_fb_st_reserve | bool | Steering system reserved fault bit |
| veh_diag_fb | veh_fb_ehb_ecu_fault | bool | EHB brake ECU fault |
| veh_diag_fb | veh_fb_ehb_dis_on_line | bool | EHB offline fault |
| veh_diag_fb | veh_fb_ehb_work_model_fault | bool | EHB abnormal operation mode |
| veh_diag_fb | veh_fb_ehb_dis_en | bool | EHB not enabled fault |
| veh_diag_fb | veh_fb_ehb_anguler_fault | bool | EHB angle sensor fault |
| veh_diag_fb | veh_fb_ehb_ot | bool | EHB controller over-temperature |
| veh_diag_fb | veh_fb_ehb_power_fault | bool | EHB power supply fault |
| veh_diag_fb | veh_fb_ehb_sensor_abnomal | bool | EHB sensor credibility anomaly |
| veh_diag_fb | veh_fb_ehb_motor_fault | bool | EHB motor fault |
| veh_diag_fb | veh_fb_ehb_oil_press_sensor_fault | bool | EHB oil pressure sensor fault |
| veh_diag_fb | veh_fb_ehb_oil_fault | bool | EHB oil circuit fault |
| veh_diag_fb | veh_fb_bra_reserve | uint8 | Brake system reserved fault bit |
| veh_diag_fb | veh_fb_ld_rv_mcu_fault | uint8 | Left drive motor driver fault |
| veh_diag_fb | veh_fb_rd_rv_mcu_fault | uint8 | Right drive motor driver fault |
| veh_diag_fb | veh_fb_aux_bms_dis_on_line | bool | BMS CAN bus offline fault |
| veh_diag_fb | veh_fb_aux_remote_dis_on_line | bool | Remote receiver offline fault |
| veh_diag_fb | veh_fb_aux_reserve | uint8 | Auxiliary device reserved fault bit |
| ultrasonic | ultrasonic_fb_01~08 | uint16 | Distance feedback of 8 ultrasonic radar sensors |
      
