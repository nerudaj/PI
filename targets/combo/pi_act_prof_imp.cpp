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

#include <PI/int/pi_int.h>
#include <PI/int/serialize.h>
#include <PI/p4info.h>
#include <PI/pi.h>

extern "C" {

pi_status_t _pi_act_prof_mbr_create(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t act_prof_id, const pi_action_data_t *action_data, pi_indirect_handle_t *mbr_handle) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(action_data);
	COMBO_UNUNSED(mbr_handle);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_mbr_delete(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t act_prof_id, pi_indirect_handle_t mbr_handle) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_id);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(mbr_handle);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_mbr_modify(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t act_prof_id, pi_indirect_handle_t mbr_handle, const pi_action_data_t *action_data) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_id);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(mbr_handle);
	COMBO_UNUNSED(action_data);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_grp_create(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t act_prof_id, size_t max_size, pi_indirect_handle_t *grp_handle) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_tgt);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(max_size);
	COMBO_UNUNSED(grp_handle);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_grp_delete(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t act_prof_id, pi_indirect_handle_t grp_handle) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_id);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(grp_handle);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_grp_add_mbr(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t act_prof_id, pi_indirect_handle_t grp_handle, pi_indirect_handle_t mbr_handle) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_id);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(grp_handle);
	COMBO_UNUNSED(mbr_handle);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_grp_remove_mbr(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t act_prof_id, pi_indirect_handle_t grp_handle, pi_indirect_handle_t mbr_handle) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_id);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(grp_handle);
	COMBO_UNUNSED(mbr_handle);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_entries_fetch(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t act_prof_id, pi_act_prof_fetch_res_t *res) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(dev_id);
	COMBO_UNUNSED(act_prof_id);
	COMBO_UNUNSED(res);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

pi_status_t _pi_act_prof_entries_fetch_done(pi_session_handle_t session_handle, pi_act_prof_fetch_res_t *res) {
	COMBO_UNUNSED(session_handle);
	COMBO_UNUNSED(res);
	return PI_STATUS_NOT_IMPLEMENTED_BY_TARGET;
}

} // end extern C
