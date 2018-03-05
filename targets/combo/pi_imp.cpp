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

#include "api/P4Dev.hpp"
#include "devices.hpp"

extern "C" {

pi_status_t _pi_init(void *extra) {
	(void) extra;
	std::cout << "PI_init\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_assign_device(pi_dev_id_t dev_id, const pi_p4info_t *p4info, pi_assign_extra_t *extra) {
	(void)p4info;
	(void)extra;
	std::cout << "PI_assign_device - " << dev_id << "\n";
	
	if (dev_id > DeviceManager::getDeviceCount()) {
		return PI_STATUS_DEV_OUT_OF_RANGE;
	}
	else if (not DeviceManager::reserveDevice(dev_id)) {
		return PI_STATUS_DEV_ALREADY_ASSIGNED;
	}
	
	p4dev_name_t DEVICE_NAMES[] = { P4DEV_ID0, P4DEV_ID1 };
	uint32_t status = devices[dev_id].initialize(DEVICE_NAMES[dev_id]);
	if (status != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}
	
	infos[dev_id] = p4info;
	
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_update_device_start(pi_dev_id_t dev_id, const pi_p4info_t *p4info, const char *device_data, size_t device_data_size) {
	(void)dev_id;
	(void)p4info;
	(void)device_data;
	(void)device_data_size;
	std::cout << "PI_update_device_start\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_update_device_end(pi_dev_id_t dev_id) {
	(void)dev_id;
	std::cout << "PI_update_device_start\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_remove_device(pi_dev_id_t dev_id) {
	std::cout << "PI_remove_device\n";
	
	if (dev_id > DeviceManager::getDeviceCount()) {
		return PI_STATUS_DEV_OUT_OF_RANGE;
	}
	
	devices[dev_id].deinitialize();
	DeviceManager::freeDevice(dev_id);
	
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_destroy() {
	std::cout << "PI_destroy\n";
	return PI_STATUS_SUCCESS;
}

// Combo does not support transaction and has no use for the session_handle
pi_status_t _pi_session_init(pi_session_handle_t *session_handle) {
	(void)session_handle;
	std::cout << "PI_session_init\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_session_cleanup(pi_session_handle_t session_handle) {
	(void) session_handle;
	std::cout << "PI_session_cleanup\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_batch_begin(pi_session_handle_t session_handle) {
	(void) session_handle;
	std::cout << "PI_batch_begin\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_batch_end(pi_session_handle_t session_handle, bool hw_sync) {
	(void) session_handle;
	(void) hw_sync;
	std::cout << "PI_batch_end\n";
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_packetout_send(pi_dev_id_t dev_id, const char *pkt, size_t size) {
	(void)dev_id;
	(void)pkt;
	(void)size;
	std::cout << "PI_packetout_send\n";
	return PI_STATUS_SUCCESS;
}

}
