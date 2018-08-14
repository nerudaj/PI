#include "devices.hpp"

const int MAX_DEVICES = 2;

DeviceArray devices(MAX_DEVICES);
DeviceInfo infos = { NULL, NULL };
std::vector<bool> reserved = { false, false };

std::size_t DeviceManager::getDeviceCount() {
	return devices.size();
}

bool DeviceManager::reserveDevice(std::size_t index) {
	if (reserved[index]) return false;
	
	reserved[index] = true;
	
	return true;
}

bool DeviceManager::freeDevice(std::size_t index) {
	reserved[index] = false;
	infos[index] = NULL;
	p4device_free(&(devices[index]));
}