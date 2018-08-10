/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Jakub Neruda (xnerud01@stud.fit.vutbr.cz)
 *
 */

#include <PI/pi.h>
#include <PI/target/pi_imp.h>
#include <iostream>
#include <p4dev.h>
#include "devices.hpp"
#include "logger.hpp"
#include "helpers.cpp"

extern "C" {

pi_status_t _pi_init(void *extra) {
	COMBO_UNUSED(extra);
	Logger::debug("PI_init");
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_assign_device(pi_dev_id_t dev_id, const pi_p4info_t *p4info, pi_assign_extra_t *extra) {
	COMBO_UNUSED(extra);
	Logger::debug("PI_assign_device - " + std::to_string(dev_id));
	
	// Try to reserve device
	if (dev_id > DeviceManager::getDeviceCount()) {
		return PI_STATUS_DEV_OUT_OF_RANGE;
	}
	else if (not DeviceManager::reserveDevice(dev_id)) {
		return PI_STATUS_DEV_ALREADY_ASSIGNED;
	}
	
	// Initialize device
	uint32_t status = p4device_init(&(devices[dev_id]), NULL, dev_id, P4DEVICE_DEFAULT_COMPONENT);
	if (status != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}
	
	// Reset device
	status = p4device_reset(&(devices[dev_id]));
	if (status != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}
	
	infos[dev_id] = p4info;
	
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_update_device_start(pi_dev_id_t dev_id, const pi_p4info_t *p4info, const char *device_data, size_t device_data_size) {
	(void)device_data;
	(void)device_data_size;
	Logger::debug("PI_update_device_start");

	if (dev_id > DeviceManager::getDeviceCount()) {
		return PI_STATUS_DEV_OUT_OF_RANGE;
	}

	Logger::debug("Ignoring new device data\n");

	infos[dev_id] = p4info;

	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_update_device_end(pi_dev_id_t dev_id) {
	(void)dev_id;
	Logger::debug("PI_update_device_end");
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_remove_device(pi_dev_id_t dev_id) {
	Logger::debug("PI_remove_device");
	
	if (dev_id > DeviceManager::getDeviceCount()) {
		return PI_STATUS_DEV_OUT_OF_RANGE;
	}
	
	DeviceManager::freeDevice(dev_id);
	
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_destroy() {
	Logger::debug("PI_destroy");
	return PI_STATUS_SUCCESS;
}

// Combo does not support transaction and has no use for the session_handle
pi_status_t _pi_session_init(pi_session_handle_t *session_handle) {
	(void)session_handle;
	Logger::debug("PI_session_init");
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_session_cleanup(pi_session_handle_t session_handle) {
	(void) session_handle;
	Logger::debug("PI_session_cleanup");
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_batch_begin(pi_session_handle_t session_handle) {
	(void) session_handle;
	Logger::debug("PI_batch_begin");
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_batch_end(pi_session_handle_t session_handle, bool hw_sync) {
	(void) session_handle;
	(void) hw_sync;
	Logger::debug("PI_batch_end");
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_packetout_send(pi_dev_id_t dev_id, const char *pkt, size_t size) {
	(void)dev_id;
	(void)pkt;
	(void)size;
	Logger::debug("PI_packetout_send");
	return PI_STATUS_SUCCESS;
}

}
