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

#include <iostream>
#include "devices.hpp"

p4rule_t *createRule(const char *tableName, const pi_p4info_t *info, pi_p4_id_t table_id) {
	// NOTE: Based on whether frontend or backend checks validity of incoming rules, this might get rewritten

	std::size_t matchFieldsSize;
	auto matchFields = pi_p4info_table_get_match_fields(info, table_id, &matchFieldsSize);
	
	// Determine engine of rule
	p4engine_type_t engineType = P4ENGINE_TCAM;
	bool lpmSet = true;
	for (std::size_t i = 0; i < matchFieldsSize; i++) {
		if (matchFields[i] != PI_P4INFO_MATCH_TYPE_LPM) lpmSet = false;
	}
	if (lpmSet) engineType = P4ENGINE_LPM;

	p4rule_t *result = p4rule_create(tableName, engineType);
	return result;
}

uint32_t addKeys(const pi_p4info_t *info, pi_p4_id_t table_id, const pi_match_key_t *match_key, p4rule_t *rule) {
	const char *data = match_key->data;

	p4key_elem_t *key;
	uint8_t *value;
	uint8_t *mask;
	uint32_t status;

	size_t matchFieldsSize = pi_p4info_table_num_match_fields(info, table_id);
	for (std::size_t i = 0; i < matchFieldsSize; i++) {
		const pi_p4info_match_field_info_t *fieldInfo = pi_p4info_table_match_field_info(info, table_id, i);
		size_t bitwidth = fieldInfo->bitwidth;
		size_t bytewidth = (bitwidth + 7) / 8;
		uint32_t prefixLen;
		const char *keyName = pi_p4info_table_match_field_name_from_id(info, table_id, fieldInfo->mf_id);

		switch (fieldInfo->match_type) {
		case PI_P4INFO_MATCH_TYPE_VALID:
			return P4DEV_NOT_IMPLEMENTED;
			break;

		case PI_P4INFO_MATCH_TYPE_EXACT:
			return P4DEV_NOT_IMPLEMENTED;
			break;

		case PI_P4INFO_MATCH_TYPE_LPM:
			value = new uint8_t[bytewidth];
			if (value == NULL) return P4DEV_ALLOCATE_ERROR;
			memcpy(value, data, bytewidth);
			data += bytewidth;
			data += retrieve_uint32(data, &prefixLen);
			key = bstlpm_p4key_create(keyName, bytewidth, value, prefixLen);
			if ((status = p4rule_add_key_element(rule, key)) != P4DEV_OK) return P4DEV_ALLOCATE_ERROR;
			break;

		case PI_P4INFO_MATCH_TYPE_TERNARY:
			value = new uint8_t[bytewidth];
			mask = new uint8_t[bytewidth];

			if (value == NULL or mask == NULL) return P4DEV_ALLOCATE_ERROR;

			memcpy(value, data, bytewidth);
			data += bytewidth;
			memcpy(mask, data, bytewidth);
			data += bytewidth;
			key = tcam_p4key_create(keyName, bytewidth, value, mask);
			if ((status = p4rule_add_key_element(rule, key)) != P4DEV_OK) return P4DEV_ALLOCATE_ERROR;

			//*requires_priority = true;
			break;

		case PI_P4INFO_MATCH_TYPE_RANGE:
			return P4DEV_NOT_IMPLEMENTED;
			break;

		default:
			assert(0);
			break;
		}
	}

	return P4DEV_OK;
}

uint32_t addAction(const pi_p4info_t *info, const pi_action_data_t *action_data, p4rule_t *rule) {
	assert(info);
	assert(action_data);

	pi_p4_id_t actionID = action_data->action_id;
	const char *actionData = action_data->data;
	const char *actionName = pi_p4info_action_name_from_id(info, actionID);

	uint32_t status;
	if ((status = p4rule_add_action(rule, actionName)) != P4DEV_OK) return status;

	size_t paramIdsSize;
	const pi_p4_id_t *paramIds = pi_p4info_action_get_params(info, actionID, &paramIdsSize);
	for (size_t i = 0; i < paramIdsSize; i++) {
		size_t paramBitwidth = pi_p4info_action_param_bitwidth(info, actionID, paramIds[i]);
		size_t paramBytewidth = (paramBitwidth + 7) / 8;

		uint8_t *data = new uint8_t[paramBytewidth];
		if (data == NULL) return P4DEV_ALLOCATE_ERROR;

		memcpy(data, actionData, paramBytewidth);
		const char *paramName = pi_p4info_action_param_name_from_id(info, actionID, paramIds[i]);

		p4param_t *param = p4param_create(paramName, paramBytewidth, data); // No need to test it, next call catches the error
		if ((status = p4rule_add_param(rule, param)) != P4DEV_OK) return status;

		actionData += paramBytewidth;
	}

	return P4DEV_OK;
}

extern "C" {

pi_status_t _pi_table_entry_add(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t table_id, const pi_match_key_t *match_key, const pi_table_entry_t *table_entry, int overwrite, pi_entry_handle_t *entry_handle) {
	(void) overwrite; ///< We cannot determine if the rule is overwritten, yet
	(void) session_handle; ///< No support for sessions
	
	const pi_p4info_t *info = infos[dev_tgt.dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_tgt.dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}
	
	// Initialize rule object
	p4rule_t *rule = createRule(tableName, info, table_id);
	if (rule == NULL) {
		std::cerr << "Cannot create rule\n";
		return pi_status_t(PI_STATUS_TARGET_ERROR);
	}
	uint32_t status;
	if ((status = addKeys(info, table_id, match_key, rule)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}
	if ((status = addAction(info, table_entry->entry.action_data, rule)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}
	
	// Insert rule to table
	uint32_t ruleIndex;
	if ((status = table->insertRule(rule, ruleIndex)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	*entry_handle = ruleIndex;
	
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_default_action_set(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t table_id, const pi_table_entry_t *table_entry) {
	(void) session_handle;
	(void) dev_tgt;
	(void) table_id;
	(void) table_entry;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_default_action_reset(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t table_id) {
	(void) session_handle;
	(void) dev_tgt;
	(void) table_id;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_default_action_get(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_table_entry_t *table_entry) {
	(void) session_handle;
	(void) dev_id;
	(void) table_id;
	(void) table_entry;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_default_action_done(pi_session_handle_t session_handle, pi_table_entry_t *table_entry) {
	(void) session_handle;
	(void) table_entry;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_entry_delete(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_entry_handle_t entry_handle) {
	(void) session_handle;
	(void) dev_id;
	(void) table_id;
	(void) entry_handle;
	return PI_STATUS_SUCCESS;
}
pi_status_t _pi_table_entry_delete_wkey(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, const pi_match_key_t *match_key) {
	(void) session_handle;
	(void) dev_id;
	(void) table_id;
	(void) match_key;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_entry_modify(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_entry_handle_t entry_handle, const pi_table_entry_t *table_entry) {
	(void) session_handle;
	(void) dev_id;
	(void) table_id;
	(void) entry_handle;
	(void) table_entry;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_entry_modify_wkey(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, const pi_match_key_t *match_key, const pi_table_entry_t *table_entry) {
	(void) session_handle;
	(void) dev_id;
	(void) table_id;
	(void) match_key;
	(void) table_entry;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_entries_fetch(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_table_fetch_res_t *res) {
	(void) session_handle;
	(void) dev_id;
	(void) table_id;
	(void) res;
	return PI_STATUS_SUCCESS;
}

pi_status_t _pi_table_entries_fetch_done(pi_session_handle_t session_handle, pi_table_fetch_res_t *res) {
	(void) session_handle;
	(void) res;
	return PI_STATUS_SUCCESS;
}

}
