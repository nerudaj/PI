#include "devices.hpp"

DeviceArray devices = { p4::Device(), p4::Device() };
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
}