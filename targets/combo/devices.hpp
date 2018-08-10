#ifndef DEVICES_HPP_
#define DEVICES_HPP_

#include <PI/pi.h>
#include <PI/target/pi_imp.h>
#include <vector>
#include <ciso646>
#include <p4dev.h>

typedef std::vector<p4device_t> DeviceArray;
typedef std::vector<const pi_p4info_t*> DeviceInfo;

extern DeviceArray devices; ///< Used device database. Index with indices obtained from p4::DeviceAlloc
extern DeviceInfo infos; 

class DeviceManager {
public:
	static std::size_t getDeviceCount();
	static bool reserveDevice(std::size_t index);
	static bool freeDevice(std::size_t index);
};

#endif
