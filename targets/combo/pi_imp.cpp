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

#include "api/P4Dev.hpp"

extern "C" {

pi_status_t _pi_init(void *extra) {
	(void) extra;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_assign_device(pi_dev_id_t dev_id, const pi_p4info_t *p4info, pi_assign_extra_t *extra) {
	(void)dev_id;
	(void)p4info;
	(void)extra;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_update_device_start(pi_dev_id_t dev_id, const pi_p4info_t *p4info, const char *device_data, size_t device_data_size) {
	(void)dev_id;
	(void)p4info;
	(void)device_data;
	(void)device_data_size;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_update_device_end(pi_dev_id_t dev_id) {
	(void)dev_id;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_remove_device(pi_dev_id_t dev_id) {
	(void)dev_id;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_destroy() {
	return PI_STATUS_SUCCESS;
}

// bmv2 does not support transaction and has no use for the session_handle
pi_status_t _pi_session_init(pi_session_handle_t *session_handle) {
	(void)session_handle;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_session_cleanup(pi_session_handle_t session_handle) {
	(void) session_handle;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_batch_begin(pi_session_handle_t session_handle) {
	(void) session_handle;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_batch_end(pi_session_handle_t session_handle, bool hw_sync) {
	(void) session_handle;
	(void) hw_sync;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_packetout_send(pi_dev_id_t dev_id, const char *pkt, size_t size) {
	(void)dev_id;
	(void)pkt;
	(void)size;
	return PI_STATUS_SUCCESS;
}

}
