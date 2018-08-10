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

#include <PI/p4info.h>
#include <PI/pi.h>
#include <PI/target/pi_counter_imp.h>
#include <p4dev.h>
#include "devices.hpp"
#include "helpers.hpp"

bool isThisBigEndianSystem() {
	uint32_t var = 1;
	return ((uint8_t*)(&var))[0] == 1;
}

void flipEndianness(uint64_t &num) {
	uint8_t* data = (uint8_t*)(&num);
	for (int i = 0; i < sizeof(uint64_t) / 2; i++) {
		std::swap(data[i], data[sizeof(uint64_t) - i - 1]);
	}
}

extern "C" {

/*
NOTE: This could come in handy later (as defined in  PI/include/PI/pi_counter.h):
#define PI_COUNTER_FLAGS_NONE 0
// do a sync with the hw when reading a counter
#define PI_COUNTER_FLAGS_HW_SYNC (1 << 0)
*/

pi_status_t _pi_counter_read(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t counter_id, size_t index, int flags, pi_counter_data_t *counter_data) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(counter_id);
	COMBO_UNUNSED(index);
	COMBO_UNUNSED(flags);
	COMBO_UNUNSED(counter_data);
	
	/*const pi_p4info_t *info = infos[dev_tgt.dev_id];
	assert(info != NULL);
	
	const char *registerName = pi_p4info_counter_name_from_id(info, counter_id);
	p4::RegisterPtr reg = devices[dev_tgt.dev_id].getRegister(registerName);
	if (reg == NULL) {
		std::cerr << "Cannot get register with name: " << registerName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}*/
	
	/*
	NOTE: Combo card so far supports only registers. To use them, we're pretending that those registers are in fact
	counters, working in BOTH mode, but the value of packets and bytes is equal to value of the register.
	Data read from the register are stored in LSB, so MSB based machines should invert them.
	*/
	/*uint8_t data[sizeof(uint64_t)] = {0};
	uint32_t status = reg->read(data, sizeof(uint64_t), index);
	if (status != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}
	
	counter_data->valid = PI_COUNTER_UNIT_PACKETS | PI_COUNTER_UNIT_BYTES;
	counter_data->bytes = *((uint64_t*)(data));
	counter_data->packets = *((uint64_t*)(data));
	
	if (isThisBigEndianSystem()) {
		flipEndianness(counter_data->bytes);
		flipEndianness(counter_data->packets);
	}*/
	
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_counter_write(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t counter_id, size_t index, const pi_counter_data_t *counter_data) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(counter_id);
	COMBO_UNUNSED(index);
	COMBO_UNUNSED(counter_data);
	
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_counter_read_direct(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t counter_id, pi_entry_handle_t entry_handle, int flags, pi_counter_data_t *counter_data) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(counter_id);
	COMBO_UNUNSED(entry_handle);
	COMBO_UNUNSED(flags);
	COMBO_UNUNSED(counter_data);
	
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_counter_write_direct(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t counter_id, pi_entry_handle_t entry_handle, const pi_counter_data_t *counter_data) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(counter_id);
	COMBO_UNUNSED(entry_handle);
	COMBO_UNUNSED(counter_data);
	
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_counter_hw_sync(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t counter_id, PICounterHwSyncCb cb, void *cb_cookie) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(counter_id);
	COMBO_UNUNSED(cb);
	COMBO_UNUNSED(cb_cookie);
	
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

}
