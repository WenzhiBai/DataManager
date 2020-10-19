#pragma once

#include <memory>
#include <Eigen/Core>
#include <Eigen/Geometry>


struct PoseState
{
    PoseState(const Eigen::Vector3d& __position, const Eigen::Quaterniond& __rotation, const Eigen::Vector3d& __velocity,
              const Eigen::Vector3d& __acc_bias, const Eigen::Vector3d& __gyr_bias, const double __timestamp)
    : position(__position), rotation(__rotation), velocity(__velocity), acc_bias(__acc_bias), gyr_bias(__gyr_bias), timestamp(__timestamp)
    {
    }
    PoseState(const Eigen::Vector3d& __position, const Eigen::Quaterniond& __rotation, const double __timestamp)
    : position(__position), rotation(__rotation), timestamp(__timestamp)
    {
    }
    PoseState()
    {
    }

    Eigen::Vector3d position;
    Eigen::Quaterniond rotation;
    Eigen::Vector3d velocity;
    Eigen::Vector3d acc_bias;
    Eigen::Vector3d gyr_bias;
    double timestamp;
};
typedef std::shared_ptr <PoseState> PoseStatePtr;

struct ImuData
{
    ImuData(const Eigen::Vector3d& __acc, const Eigen::Vector3d& __gyr, const double __timestamp)
        : acc(__acc), gyr(__gyr), timestamp(__timestamp)
    {
    }

    Eigen::Vector3d acc;
    Eigen::Vector3d gyr;
    double timestamp;
};
typedef std::shared_ptr <ImuData> ImuDataPtr;

struct WheelOdoData
{
    WheelOdoData(const double left_wheel, const double right_wheel, const double __timestamp)
	: wheel_speed(left_wheel, right_wheel), timestamp(__timestamp)
    {
    }

    Eigen::Vector2d wheel_speed;
    double timestamp;
};
typedef std::shared_ptr <WheelOdoData> WheelOdoDataPtr;

struct SteeringInfo
{
    SteeringInfo(const double __steer_angle, const double __timestamp)
	: steer_angle(__steer_angle), timestamp(__timestamp)
    {
    }

    double steer_angle;
    double timestamp;
};
typedef std::shared_ptr <SteeringInfo> SteeringInfoPtr;

struct GearboxInfo
{
    GearboxInfo(const int __gearbox_position_display, const double __timestamp)
	: gearbox_position_display(__gearbox_position_display), timestamp(__timestamp)
    {
    }
    GearboxInfo()
    {
       gearbox_position_display = 0;
       timestamp = -1.0; 
    }
    
    int gearbox_position_display;
    double timestamp;
};
typedef std::shared_ptr <GearboxInfo> GearboxInfoPtr;

}

