#include "P4Dev.hpp"

using p4::Device::RegisterArray; ///< Declared in the protected section of Device

uint32_t RegisterArray::initialize(p4dev_t *deviceInfo) {
	uint32_t status = p4dev_registers_get(deviceInfo, &registerArray, &registerCount);
	if (status != P4DEV_OK) return status;
	
	for (uint32_t i = 0; i < registerCount; i++) {
		p4_register_t &reg = registerArray[i];
		registers[reg.name].initialize(deviceInfo, registerArray + i);
	}
	
	return P4DEV_OK;
}

p4::Register *RegisterArray::getRegister(const char *name) {
	auto itr = registers.find(name);
	if (itr == registers.end()) return NULL;
	return &(itr->second);
}

void RegisterArray::deinitialize() {
	p4dev_registers_free(registerArray, registerCount);
}