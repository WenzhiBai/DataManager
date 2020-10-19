#include "data_manager.h"

DataManager* DataManager::_instance = nullptr;
std::mutex DataManager::_instance_mutex;
DataManager::InstanceRelease DataManager::_instane_release;

