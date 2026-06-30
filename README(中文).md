### [English](README.md) | 中文

# MK-mid-ros2

## 1.环境确认：
####      1.1、系统：
            Ubuntu 22.04
####      1.2、ROS版本：
            Humble

## 2.电脑，工控机开机设置
####      2.1、rc.local文件确认：
            查看/etc/目录下有没有 rc.local文件，如果没有 sudo cp rc.local /etc
            如果存在rc.local文件，则在 rc.local的 exit 0上面加入 以下内容后保存。

            sleep 2
            sudo ip link set can0 type can bitrate 500000
            sudo ip link set can0 up

####      2.2、开机前CAN卡连接确认：
            开机或者重启之前，CAN卡已经接到工控机的USB口。

####      2.3、设置成功信号：
            看到RX和TX灯亮了就表示设置成功。
            
## 3.CAN卡连接与设置
####      3.1、底盘CAN卡连接：
            底盘的CAN-H连接到CAN卡接插头的CANH，底盘的CAN-L连接到Can卡接插头的CANL，如下图：
            
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/CAN_Connection.png?raw=true)

####      3.2、电脑与机器人通信连接：
            底盘CAN卡连接完成后，将CAN卡usb线一端直接插到电脑usb口，完成CAN卡与电脑的连接，连接完成后，PWR灯亮。
####      3.3、USB是否连接成功检查：
            打开终端，输入指令：  lsusb  在输出信息中看到以下图片所示内容，则表示CAN能够正常使用。

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/terminal_state.png?raw=true)  

####      3.4、通信启动：
            打开一个新的终端，输入指令：
            sudo ip  link  set  can0  type  can  bitrate  500000 & sudo ip  link  set  can0  up
            此时，若底盘处于上电状态，蓝色RX灯会闪烁，同时红色TX灯也会亮，RX灯闪烁表示Can卡接收到数据，TX灯闪烁表示发送数据。

## 4.通信数据查看
####      4.1 安装can-utils工具，请执行以下指令：
            sudo apt-get install can-utils
####      4.2 查看数据指令：
            candump can0
####      4.3、正常打印信息：
      

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/candump_print.png?raw=true)

## 5.ROS2驱动包运行
####      5.1、文件结构：
      
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/doc_tree.png?raw=true)

####      5.2、编译工作空间：
            在你的工作空间中打开终端/在终端中进入你的工作空间，让终端处于工作空间级，再执行指令：
            colcon build
####      5.3、设置临时环境变量：
            在要运行程序的终端中执行指令：
            source ~/你的工作空间(例：5.1-ros2_ws)/install/setup.bash
            
            或者让要运行程序的终端处于工作空间级，执行指令：
            source install/setup.bash
            
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/source.png?raw=true)

####      5.4、ROS2驱动启动：
            在你刚才输入了临时环境变量的终端中执行指令：
            ros2 launch yhs_can_control yhs_can_control.launch.py
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/launch.png?raw=true)
            
####      5.5、终端打印出的运行成功信息：

![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/node_print.png?raw=true)  

## 6.运动测试：
####       6.1、在测试之前，建议先把车架起来，或者向底盘下发较小的速度，例如0.03。
####       6.2、打开新的终端，进入你的工作空间目录(例：5.1-ros2_ws)，执行指令
                        source install/setup.bash
                   或者不进人工作空间目录，执行指令
                        source ~/你的工作空间(例：5.1-ros2_ws)/install/setup.bash
####       6.3、下发指令控制底盘运动
              6.5.1、打开新的终端，进入你的工作空间目录，设置临时环境变量：
                        source install/setup.bash
                     或者不进人工作空间目录，执行指令
                        source ~/你的工作空间名(例：5.1-ros2_ws)/install/setup.bash
              6.5.2、在设置了临时环境变量的终端，使用rqt_publisher工具，遥控器要切换到指令控制模式
                        ros2 run rqt_publisher rqt_publisher
![](https://github.com/YUHESEN-Robot/MK-mid-ros2/blob/main/images/rqt_tool.png?raw=true)

## 7. 话题说明
> 说明：部分IO、传感器字段为预留功能，若底盘无对应硬件则无法生效；速度控制话题发布频率建议≥30Hz。

### 7.1 底盘运动控制话题（下发指令）
- 消息类型：`yhs_can_interfaces/msg/CtrlCmd`
- 话题名称：`/ctrl_cmd`
- 推荐发布频率：≥30Hz

| 变量名 | 数据类型 | 说明 |
|--------|----------|------|
| ctrl_cmd_gear | uint8 | 目标档位<br>0：disable（禁用）<br>1：P档（驻车）<br>2：R档（倒车）<br>3：N档（空挡）<br>4：D档（前进）<br>其余数值无效，底盘默认驻车 |
| ctrl_cmd_velocity | float32 | 目标行驶速度，单位：m/s |
| ctrl_cmd_steering | float32 | 目标转向角度，单位：° |
| ctrl_cmd_brake | uint8 | 目标制动力大小，单位：% |

### 7.2 IO外设控制话题（下发指令）
- 消息类型：`yhs_can_interfaces/msg/IoCmd`
- 话题名称：`/io_cmd`

| 变量名 | 数据类型 | 说明 |
|--------|----------|------|
| io_cmd_enable | bool | IO总控制使能<br>0：关闭，灯光/转向灯由底盘VCU自主控制<br>1：开启，灯光/转向灯由CAN指令控制 |
| io_cmd_lower_beam_headlamp | bool | 近光灯开关（预留）<br>0=关闭，1=开启 |
| io_cmd_upper_beam_headlamp | bool | 远光灯开关（预留）<br>0=关闭，1=开启 |
| io_cmd_turn_lamp | uint8 | 转向灯/危险报警灯<br>0：全关<br>1：左转向灯<br>2：右转向灯<br>3：双闪危险报警灯<br>优先级：单侧转向灯 > 双闪 |
| io_cmd_braking_lamp | bool | 制动灯开关（预留） |
| io_cmd_clearance_lamp | bool | 示廓灯开关（预留） |
| io_cmd_fog_lamp | bool | 雾灯开关（预留） |
| io_cmd_speaker | bool | 扬声器开关（预留） |
| io_cmd_dis_charge | bool | 充电强制高压上电标志<br>充电时开启可48V高压上电恢复行驶；充电+该标志同时生效时禁止倒车 |

### 7.3 底盘状态反馈订阅话题（接收底盘上报数据）
- 消息类型：`yhs_can_interfaces/msg/ChassisInfoFb`
- 话题名称：`/chassis_info_fb`

| 分组字段 | 变量名 | 数据类型 | 说明 |
|----------|--------|----------|------|
| ctrl_fb | ctrl_fb_gear | uint8 | 当前实际档位，同7.1档位定义 |
| ctrl_fb | ctrl_fb_velocity | float32 | 当前车体实时速度，单位：m/s |
| ctrl_fb | ctrl_fb_steering | float32 | 当前实时转向角，单位：°，分辨率0.01°/bit |
| ctrl_fb | ctrl_fb_brake | uint8 | 当前制动状态（预留），单位：% |
| ctrl_fb | ctrl_fb_remote_st | bool | 遥控器连接状态：0断开，1在线 |
| ctrl_fb | ctrl_fb_mode | uint8 | 车辆运行模式<br>0x0：auto自动模式<br>0x1：remote遥控模式<br>0x2：stop停机模式 |
| io_fb | io_fb_enable | bool | IO控制总使能状态反馈 |
| io_fb | io_fb_lower_beam_headlamp | bool | 近光灯状态反馈（预留） |
| io_fb | io_fb_upper_beam_headlamp | bool | 远光灯状态反馈（预留） |
| io_fb | io_fb_turn_lamp | int8 | 转向灯状态反馈，数值定义同下发指令 |
| io_fb | io_fb_braking_lamp | bool | 制动灯状态反馈 |
| io_fb | io_fb_clearance_lamp | bool | 示廓灯状态反馈（预留） |
| io_fb | io_fb_fog_lamp | bool | 雾灯状态反馈（预留） |
| io_fb | io_fb_speaker | bool | 扬声器状态反馈（预留） |
| io_fb | io_fb_fm_impact_sensor | bool | 前中防撞条触发反馈：0未触发，1触发 |
| io_fb | io_fb_rm_impact_sensor | bool | 后中防撞条触发反馈 |
| io_fb | io_fb_fl_impact_sensor | bool | 前左防撞条（预留） |
| io_fb | io_fb_rl_impact_sensor | bool | 后左防撞条（预留） |
| io_fb | io_fb_fr_impact_sensor | bool | 前右防撞条（预留） |
| io_fb | io_fb_rr_impact_sensor | bool | 后右防撞条（预留） |
| io_fb | io_fb_fl_drop_sensor | bool | 前左跌落传感器（预留） |
| io_fb | io_fb_rl_drop_sensor | bool | 后左跌落传感器（预留） |
| io_fb | io_fb_fm_drop_sensor | bool | 前中跌落传感器（预留） |
| io_fb | io_fb_rm_drop_sensor | bool | 后中跌落传感器（预留） |
| io_fb | io_fb_fr_drop_sensor | bool | 前右跌落传感器（预留） |
| io_fb | io_fb_rr_drop_sensor | bool | 后右跌落传感器（预留） |
| io_fb | io_fb_dis_charge | bool | 充电强制上电标志反馈 |
| io_fb | io_fb_charge_en | bool | 充电桩接触检测标志：1=车辆触达充电桩 |
| io_fb | io_fb_scram_st | bool | 急停状态：0正常，1急停按下 |
| lr_wheel_fb | lr_wheel_fb_velocity | float32 | 左后轮实时速度，单位：m/s |
| lr_wheel_fb | lr_wheel_fb_pulse | int32 | 左后轮累计脉冲；单圈200000脉冲（2500线编码器，4倍频，20减速比） |
| rr_wheel_fb | rr_wheel_fb_velocity | float32 | 右后轮实时速度，单位：m/s |
| rr_wheel_fb | rr_wheel_fb_pulse | int32 | 右后轮累计脉冲，参数同左轮 |
| odo_fb | odo_fb_accumulative_mileage | float32 | 整车累计行驶里程 |
| odo_fb | odo_fb_accumulative_angular | float32 | 累计转向角度（预留） |
| bms_flag_info_fb | bms_flag_info_soc | uint8 | 电池剩余电量SOC，单位%，1%/bit |
| bms_flag_info_fb | bms_flag_info_single_ov | bool | 电芯过压保护标志 |
| bms_flag_info_fb | bms_flag_info_single_uv | bool | 电芯欠压保护标志 |
| bms_flag_info_fb | bms_flag_info_ov | bool | 整包过压保护标志 |
| bms_flag_info_fb | bms_flag_info_uv | bool | 整包欠压保护标志 |
| bms_flag_info_fb | bms_flag_info_charge_ot | bool | 充电过温保护 |
| bms_flag_info_fb | bms_flag_info_charge_ut | bool | 充电低温保护 |
| bms_flag_info_fb | bms_flag_info_discharge_ot | bool | 放电过温保护 |
| bms_flag_info_fb | bms_flag_info_discharge_ut | bool | 放电低温保护 |
| bms_flag_info_fb | bms_flag_info_charge_oc | bool | 充电过流保护 |
| bms_flag_info_fb | bms_flag_info_discharge_oc | bool | 放电过流保护 |
| bms_flag_info_fb | bms_flag_info_short | bool | 电池短路保护 |
| bms_flag_info_fb | bms_flag_info_ic_error | bool | BMS前端检测IC故障 |
| bms_flag_info_fb | bms_flag_info_lock_mos | bool | MOS管软件锁定保护 |
| bms_flag_info_fb | bms_flag_info_charge_st | uint8 | 充电状态<br>0：未充电<br>1：手动充电<br>2：前桩充电<br>3：后桩充电 |
| bms_flag_info_fb | bms_flag_info_soc_warning | bool | 低电量预警 |
| bms_flag_info_fb | bms_flag_info_soc_low_protection | bool | 低电量停机保护 |
| bms_flag_info_fb | bms_flag_info_hight_temperature | float32 | 电池最高温度，单位℃ |
| bms_flag_info_fb | bms_flag_info_low_temperature | float32 | 电池最低温度，单位℃ |
| bms_info_fb | bms_info_voltage | float32 | 电池总电压，单位V |
| bms_info_fb | bms_info_current | float32 | 电池实时电流，单位A |
| bms_info_fb | bms_info_remaining_capacity | float32 | 电池剩余容量，单位Ah |
| veh_diag_fb | veh_fb_fault_level | uint8 | 整车故障等级<br>0无故障 /1/2/3级故障 |
| veh_diag_fb | veh_fb_auto_can_ctrl_cmd | bool | Auto运动CAN通信故障 |
| veh_diag_fb | veh_fb_auto_io_can_cmd | bool | Auto IO CAN通信故障 |
| veh_diag_fb | veh_fb_eps_dis_on_line | bool | EPS转向驱动器掉线 |
| veh_diag_fb | veh_fb_eps_fault | bool | EPS通用故障 |
| veh_diag_fb | veh_fb_eps_mosf_et_ot | bool | EPS功率管过温 |
| veh_diag_fb | veh_fb_eps_warning | bool | EPS预警故障 |
| veh_diag_fb | veh_fb_eps_dis_work | bool | EPS工作异常故障 |
| veh_diag_fb | veh_fb_eps_over_current | bool | EPS过流故障 |
| veh_diag_fb | veh_fb_st_reserve | bool | 转向系统预留故障位 |
| veh_diag_fb | veh_fb_ehb_ecu_fault | bool | EHB制动ECU故障 |
| veh_diag_fb | veh_fb_ehb_dis_on_line | bool | EHB掉线故障 |
| veh_diag_fb | veh_fb_ehb_work_model_fault | bool | EHB运行模式异常 |
| veh_diag_fb | veh_fb_ehb_dis_en | bool | EHB未使能故障 |
| veh_diag_fb | veh_fb_ehb_anguler_fault | bool | EHB角度传感器故障 |
| veh_diag_fb | veh_fb_ehb_ot | bool | EHB控制器超温 |
| veh_diag_fb | veh_fb_ehb_power_fault | bool | EHB电源故障 |
| veh_diag_fb | veh_fb_ehb_sensor_abnomal | bool | EHB传感器可信度异常 |
| veh_diag_fb | veh_fb_ehb_motor_fault | bool | EHB电机故障 |
| veh_diag_fb | veh_fb_ehb_oil_press_sensor_fault | bool | EHB油压传感器故障 |
| veh_diag_fb | veh_fb_ehb_oil_fault | bool | EHB油路故障 |
| veh_diag_fb | veh_fb_bra_reserve | uint8 | 制动故障预留位 |
| veh_diag_fb | veh_fb_ld_rv_mcu_fault | uint8 | 左驱动电机故障 |
| veh_diag_fb | veh_fb_rd_rv_mcu_fault | uint8 | 右驱动电机故障 |
| veh_diag_fb | veh_fb_aux_bms_dis_on_line | bool | BMS CAN掉线故障 |
| veh_diag_fb | veh_fb_aux_remote_dis_on_line | bool | 遥控器接收机掉线 |
| veh_diag_fb | veh_fb_aux_reserve | uint8 | 辅件故障预留位 |
| ultrasonic | ultrasonic_fb_01~08 | uint16 | 8路超声波雷达测距反馈 |
