#include "P4Dev.hpp"

uint32_t p4::Register::read(uint8_t* read_data, const uint32_t length, uint32_t reg_ind) {
	return p4dev_register_read(deviceInfo, regInfo, read_data, length, reg_ind);
}

uint32_t p4::Register::write(uint8_t* data_to_write, const uint32_t length, uint32_t reg_ind) {
	return p4dev_register_write(deviceInfo, regInfo, data_to_write, length, reg_ind);
}

void p4::Register::initialize(p4dev_t *deviceInfo, p4_register_t *registerInfo) {
	p4::Register::deviceInfo = deviceInfo;
	p4::Register::regInfo = registerInfo;
	// ??? p4dev_initialize_registers
}