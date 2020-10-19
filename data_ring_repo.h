#pragma once

#include <mutex>
#include <memory>
#include <cmath>
#include <vector>


template<class DataType>
class DataRingRepo
{
typedef std::shared_ptr<DataType> DataTypePtr;
typedef std::pair<double, DataTypePtr> DataPair;

private:
    size_t _max_size = 0;
    std::mutex _data_mutex;
    DataPair *_data = nullptr;

    size_t _size = 0;
    int _head_at = -1;
    bool _is_full = false;

public:
    DataRingRepo(const size_t &max_size);
    ~DataRingRepo();

private:
    void bubble_sort();
    size_t binary_search(const double &timestamp, size_t low, size_t high);
    bool get_latest_data_lock_free(DataTypePtr &data_ptr);
    // Returns true if the timestamp is in data range.
    // Though the timestamp is not in data range, there are always give the closest data and it's idx in data[], except data[] is empty.
    // When data[] is empty, returns false, and output: data_ptr = nullptr, data_idx = -1.
    bool get_closest_data_lock_free(const double &timestamp, int &data_idx);

public:
    DataType operator[](const size_t &idx);
    inline size_t max_size() { return _max_size; }
    inline size_t size() { std::lock_guard<std::mutex> lock(_data_mutex); return _size; }
    inline bool is_empty() { std::lock_guard<std::mutex> lock(_data_mutex); return _head_at < 0; }
    inline bool is_full() { std::lock_guard<std::mutex> lock(_data_mutex); return _is_full; }

    void clear();
    void insert_data(const DataType &data);
    bool get_latest_data(DataTypePtr &data_ptr);
    bool get_latest_data(DataType &data);
    bool get_closest_data(const double &timestamp, DataTypePtr &data_ptr);
    bool get_closest_data(const double &timestamp, DataType &data);
    bool get_period_data(const double &start_timestamp, const double &end_timestamp, std::vector<DataTypePtr> &data_ptr_vec);
    bool get_period_data(const double &start_timestamp, const double &end_timestamp, std::vector<DataType> &data_vec);
    bool get_latest_period_data(const double &start_timestamp, std::vector<DataTypePtr> &data_ptr_vec);
    bool get_latest_period_data(const double &start_timestamp, std::vector<DataType> &data_vec);
};

template<class DataType>
DataRingRepo<DataType>::DataRingRepo(const size_t &max_size)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    _data = new DataPair[max_size];
    _max_size = max_size;
}

template<class DataType>
DataRingRepo<DataType>::~DataRingRepo()
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    delete[] _data;
    _max_size = 0;
}

template<class DataType>
void DataRingRepo<DataType>::bubble_sort()
{
    bool is_bubble_move = true;
    size_t bubble_at = _head_at;
    size_t top_at  = _is_full ? (_head_at + 1) % _max_size : 0;

    while (is_bubble_move)
    {
        if (bubble_at == top_at) {
            break;
        }

        size_t comp_at = (bubble_at - 1) % _max_size;
        if (_data[bubble_at].first < _data[comp_at].first) {
            DataPair tmp = _data[bubble_at];
            _data[bubble_at] = _data[comp_at];
            _data[comp_at] = tmp;
            bubble_at = comp_at;
            is_bubble_move = true;
        } else {
            is_bubble_move = false;
        }
    }
}

template<class DataType>
DataType DataRingRepo<DataType>::operator[](const size_t &idx)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    size_t index = idx;
    if (_is_full) {
        index = (_head_at + index + 1) % _max_size;
    } else {
        index = std::min(index, _size);
    }
    return *(_data[index].second);
}

template<class DataType>
void DataRingRepo<DataType>::clear()
{
    std::lock_guard<std::mutex> lock(_data_mutex);

    _size = 0;
    _head_at = -1;
    _is_full = false;
}

template<class DataType>
void DataRingRepo<DataType>::insert_data(const DataType &data)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    
    _head_at++;
    _size = std::min(++_size, _max_size);

    if (!_is_full && _head_at >= _max_size - 1)
    {
        _is_full = true;
    }

    _head_at = _head_at % _max_size;
    _data[_head_at].first = data.timestamp;
    _data[_head_at].second = std::make_shared<DataType>(data);

    bubble_sort();
}

template<class DataType>
bool DataRingRepo<DataType>::get_latest_data_lock_free(DataTypePtr &data_ptr)
{
    if (_head_at < 0) {
        data_ptr = nullptr;
        return false;
    }
    
    data_ptr = _data[_head_at].second;
    return true;
}

template<class DataType>
bool DataRingRepo<DataType>::get_latest_data(DataTypePtr &data_ptr)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    return get_latest_data_lock_free(data_ptr);
}

template<class DataType>
bool DataRingRepo<DataType>::get_latest_data(DataType &data)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    DataTypePtr data_ptr = nullptr;
    bool ret = get_latest_data_lock_free(data_ptr);
    if (ret) {
        data = *data_ptr;
    }
    return ret;
}

template<class DataType>
size_t DataRingRepo<DataType>::binary_search(const double &timestamp, size_t low, size_t high)
{
    high = high < low ? high + _max_size : high;

    if (low == high - 1) {
        return std::fabs(_data[low].first - timestamp) <= std::fabs(_data[high].first - timestamp) ? low : high;
    }

    size_t mid = ((low + high) / 2) % _max_size;
    if (_data[mid].first > timestamp)
        return binary_search(timestamp, low, mid);
    else
        return binary_search(timestamp, mid, high % _max_size);
}

template<class DataType>
bool DataRingRepo<DataType>::get_closest_data_lock_free(const double &timestamp, int &data_idx)
{
    if (_head_at < 0) {
        data_idx = -1;
        return false;
    }

    size_t low = _is_full ? (_head_at + 1) % _max_size : 0;
    size_t high = _head_at;
    
    if (_data[low].first > timestamp) {
        data_idx = low;
        return false;
    } else if (_data[high].first < timestamp) {
        data_idx = high;
        return false;
    }

    data_idx = binary_search(timestamp, low, high);
    return true;
}

template<class DataType>
bool DataRingRepo<DataType>::get_closest_data(const double &timestamp, DataTypePtr &data_ptr)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    data_ptr = nullptr;
    int data_idx = -1;
    if (get_closest_data_lock_free(timestamp, data_idx)) {
        data_ptr = _data[data_idx].second;
        return true;
    } else {
        return false;
    }
}

template<class DataType>
bool DataRingRepo<DataType>::get_closest_data(const double &timestamp, DataType &data)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    DataTypePtr data_ptr = nullptr;
    int data_idx = -1;
    if (get_closest_data_lock_free(timestamp, data_idx)) {
        data = *_data[data_idx].second;
        return true;
    } else {
        return false;
    }
}

template<class DataType>
bool DataRingRepo<DataType>::get_period_data(const double &start_timestamp, const double &end_timestamp, std::vector<DataTypePtr> &data_ptr_vec)
{
    std::vector<DataTypePtr>().swap(data_ptr_vec);
    std::lock_guard<std::mutex> lock(_data_mutex);
    DataTypePtr data_ptr = nullptr;

    int start_at = -1;
    int end_at = -1;
    if (get_closest_data_lock_free(start_timestamp, start_at) && get_closest_data_lock_free(end_timestamp, end_at)) {
        size_t search_at = start_at;
        size_t search_end = start_at > end_at ? _max_size + end_at : end_at;
		
        while (search_at <= search_end) {
            size_t idx = search_at % _max_size;
            data_ptr_vec.emplace_back(_data[idx].second);
            search_at++;
        }
    }
	
    return (data_ptr_vec.size() > 0);
}

template<class DataType>
bool DataRingRepo<DataType>::get_period_data(const double &start_timestamp, const double &end_timestamp, std::vector<DataType> &data_vec)
{
    std::vector<DataType>().swap(data_vec);
    std::lock_guard<std::mutex> lock(_data_mutex);
    DataTypePtr data_ptr = nullptr;
    
    int start_at = -1;
    int end_at = -1;
    if (get_closest_data_lock_free(start_timestamp, start_at) && get_closest_data_lock_free(end_timestamp, end_at)) {
        size_t search_at = start_at;
        size_t search_end = start_at > end_at ? _max_size + end_at : end_at;
		
        while (search_at <= search_end) {
            size_t idx = search_at % _max_size;
            data_vec.emplace_back(*_data[idx].second);
            search_at++;
        }
    }
	
    return (data_vec.size() > 0);
}

template<class DataType>
bool DataRingRepo<DataType>::get_latest_period_data(const double &start_timestamp, std::vector<DataTypePtr> &data_ptr_vec)
{
    std::vector<DataTypePtr>().swap(data_ptr_vec);
    std::lock_guard<std::mutex> lock(_data_mutex);

    int start_at = -1;
    if (get_closest_data_lock_free(start_timestamp, start_at)) {
        size_t search_at = start_at;
        size_t search_end = start_at > _head_at ? _max_size + _head_at : _head_at;
		
        while (search_at <= search_end) {
            size_t idx = search_at % _max_size;
            data_ptr_vec.emplace_back(_data[idx].second);
            search_at++;
        }
    }
	
    return (data_ptr_vec.size() > 0);
}

template<class DataType>
bool DataRingRepo<DataType>::get_latest_period_data(const double &start_timestamp, std::vector<DataType> &data_vec)
{
    std::vector<DataType>().swap(data_vec);
    std::lock_guard<std::mutex> lock(_data_mutex);

    int start_at = -1;
    if (get_closest_data_lock_free(start_timestamp, start_at)) {
        size_t search_at = start_at;
        size_t search_end = start_at > _head_at ? _max_size + _head_at : _head_at;
		
        while (search_at <= search_end) {
            size_t idx = search_at % _max_size;
            data_vec.emplace_back(*_data[idx].second);
            search_at++;
        }
    }
	
    return (data_vec.size() > 0);
}

