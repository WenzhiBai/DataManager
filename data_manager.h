#pragma once

#include "data_ring_repo.h"
#include "data_interface.h"


class DataManager
{
public:
    // singleton pattern
    static DataManager* get_instance()
    {
        if (nullptr == _instance)
        {
            std::lock_guard<std::mutex> lg(_instance_mutex);
            if (nullptr == _instance)
            { 
                DataManager* ptmp = new DataManager();
                _instance = ptmp;
            }
        }
        return _instance;
    }
    //Public deleted functions can give better error msg
    DataManager(DataManager const&) = delete;
    void operator=(DataManager const&) = delete;

private:
    DataManager() : _imu_data_repo(500),
                    _wheel_speed_data_repo(500),
                    _steering_data_repo(500),
                    _gearbox_data_repo(500), 
                    _distance_data_repo(500),
                    _wheel_odo_data_repo(500),
                    _pose_state_repo(500)
    {
        _instance = this;
    };

    ~DataManager() {};

    class InstanceRelease 
    { 
    public: 
        ~InstanceRelease() 
        { 
            if (DataManager::_instance) 
            { 
                delete DataManager::_instance; 
                DataManager::_instance = nullptr; 
            } 
        } 
    };

public:
    // data repository
    DataRingRepo<ImuData> _imu_data_repo;
    DataRingRepo<WheelOdoData> _wheel_speed_data_repo;
    DataRingRepo<SteeringInfo> _steering_data_repo;
    DataRingRepo<GearboxInfo> _gearbox_data_repo;
    DataRingRepo<PoseState> _pose_state_repo;

private: 
    static InstanceRelease _instane_release;
    static DataManager* _instance; 
    static std::mutex _instance_mutex;
};

