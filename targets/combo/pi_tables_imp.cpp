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
#include "helpers.hpp"

p4rule_t *createRule(const char *tableName, const pi_p4info_t *info, pi_p4_id_t table_id) {
	std::size_t matchFieldsSize = pi_p4info_table_num_match_fields(info, table_id);

	// Determine engine of rule
	p4engine_type_t engineType = P4ENGINE_UNKNOWN;
	for (std::size_t i = 0; i < matchFieldsSize; i++) {
		auto finfo = pi_p4info_table_match_field_info(info, table_id, i);
		if (engineType == P4ENGINE_UNKNOWN) engineType = translateEngine(finfo->match_type);
		else if (engineType != translateEngine(finfo->match_type)) return NULL;
	}
	if (engineType == P4ENGINE_UNKNOWN) return NULL;

	p4rule_t *result = p4rule_create(tableName, engineType);
	return result;
}

uint32_t createKeys(const pi_p4info_t *info, pi_p4_id_t table_id, const pi_match_key_t *match_key, p4key_elem_t **key) {
	const char *data = match_key->data;

	uint8_t *value;
	uint8_t *mask;
	uint32_t status;

	p4key_elem_t *last = NULL;

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
			value = new uint8_t[bytewidth];
			if (value == NULL) return P4DEV_ALLOCATE_ERROR;
			memcpy(value, data, bytewidth);
			data += bytewidth;

			if (last == NULL) {
				(*key) = cuckoo_p4key_create(keyName, bytewidth, value);
				last = *key;
			}
			else {
				last->next = cuckoo_p4key_create(keyName, bytewidth, value);
				last = last->next;
			}
			break;

		case PI_P4INFO_MATCH_TYPE_LPM:
			value = new uint8_t[bytewidth];
			if (value == NULL) return P4DEV_ALLOCATE_ERROR;
			memcpy(value, data, bytewidth);
			data += bytewidth;
			data += retrieve_uint32(data, &prefixLen);

			if (last == NULL) {
				(*key) = bstlpm_p4key_create(keyName, bytewidth, value, prefixLen);
				last = *key;
			}
			else {
				last->next = bstlpm_p4key_create(keyName, bytewidth, value, prefixLen);
				last = last->next;
			}
			break;

		case PI_P4INFO_MATCH_TYPE_TERNARY:
			value = new uint8_t[bytewidth];
			mask = new uint8_t[bytewidth];

			if (value == NULL or mask == NULL) return P4DEV_ALLOCATE_ERROR;

			memcpy(value, data, bytewidth);
			data += bytewidth;
			memcpy(mask, data, bytewidth);
			data += bytewidth;

			if (last == NULL) {
				(*key) = tcam_p4key_create(keyName, bytewidth, value, mask);
				last = *key;
			}
			else {
				last->next = tcam_p4key_create(keyName, bytewidth, value, mask);
				last = last->next;
			}

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

uint32_t addKeys(const pi_p4info_t *info, pi_p4_id_t table_id, const pi_match_key_t *match_key, p4rule_t *rule) {
	p4key_elem_t *key;
	uint32_t status;
	if ((status = createKeys(info, table_id, match_key, &key)) != P4DEV_OK) {
		return status;
	}

	if ((status = p4rule_add_key_element(rule, key)) != P4DEV_OK) return status;

	return P4DEV_OK;
}

uint32_t createParams(const pi_p4info_t *info, const pi_p4_id_t actionID, const char *actionData, p4param_t **param) {
	assert(info != NULL);
	assert(actionData != NULL);
	assert(param != NULL);
	assert(*param == NULL);

	uint32_t status;
	p4param_t *last = NULL;
	size_t paramIdsSize;
	const pi_p4_id_t *paramIds = pi_p4info_action_get_params(info, actionID, &paramIdsSize);
	for (size_t i = 0; i < paramIdsSize; i++) {
		size_t paramBitwidth = pi_p4info_action_param_bitwidth(info, actionID, paramIds[i]);
		size_t paramBytewidth = (paramBitwidth + 7) / 8;

		uint8_t *data = new uint8_t[paramBytewidth];
		if (data == NULL) return P4DEV_ALLOCATE_ERROR;

		memcpy(data, actionData, paramBytewidth);
		const char *paramName = pi_p4info_action_param_name_from_id(info, actionID, paramIds[i]);

		if (*param == NULL) {
			*param = p4param_create(paramName, paramBytewidth, data);
			if (*param == NULL) return P4DEV_ALLOCATE_ERROR;
			last = *param;
		}
		else {
			last->next = p4param_create(paramName, paramBytewidth, data);
			if (last->next == NULL) return P4DEV_ALLOCATE_ERROR;
			last = last->next;
		}

		actionData += paramBytewidth;
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

	p4param_t *param = NULL;
	if ((status = createParams(info, actionID, actionData, &param)) != P4DEV_OK) return status;

	if (param != NULL) {
		if ((status = p4rule_add_param(rule, param)) != P4DEV_OK) {
			return status;
		}
	}

	return P4DEV_OK;
}

extern "C" {

//! Adds an entry to a table. Trying to add an entry that already exists should
//! return an error, unless the \p overwrite flag is set.
pi_status_t _pi_table_entry_add(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t table_id, const pi_match_key_t *match_key, const pi_table_entry_t *table_entry, int overwrite, pi_entry_handle_t *entry_handle) {
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
	if ((status = table->insertRule(rule, ruleIndex, overwrite)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	*entry_handle = ruleIndex;
	
	return PI_STATUS_SUCCESS;
}

//! Sets the default entry for a table. Should return an error if the default
//! entry was statically configured and set as const in the P4 program.
pi_status_t _pi_table_default_action_set(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t table_id, const pi_table_entry_t *table_entry) {
	(void) session_handle;

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
	p4rule_mark_default(rule);
	uint32_t status;
	if ((status = addAction(info, table_entry->entry.action_data, rule)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	// Insert rule to table
	if ((status = table->insertDefaultRule(rule)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	return PI_STATUS_SUCCESS;
}

//! Resets the default entry for a table, as previously set with
//! pi_table_default_action_set, to the original default action (as specified in
//! the P4 program).
pi_status_t _pi_table_default_action_reset(pi_session_handle_t session_handle, pi_dev_tgt_t dev_tgt, pi_p4_id_t table_id) {
	(void) session_handle;

	const pi_p4info_t *info = infos[dev_tgt.dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_tgt.dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	uint32_t status;
	if ((status = table->resetDefaultRule()) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	return PI_STATUS_SUCCESS;
}

//! Retrieve the default entry for a table.
pi_status_t _pi_table_default_action_get(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_table_entry_t *table_entry) {
	(void) session_handle;

	const pi_p4info_t *info = infos[dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	p4rule_t *defaultRule = table->getDefaultRule();
	if (defaultRule == NULL) {
		std::cerr << "No default rule set\n";
		return PI_STATUS_TARGET_ERROR;
	}

	return retrieveEntry(info, defaultRule->action, defaultRule->param, table_entry);
}

//! Need to be called after pi_table_default_action_get, once you wish the
//! memory to be released.
pi_status_t _pi_table_default_action_done(pi_session_handle_t session_handle, pi_table_entry_t *table_entry) {
	(void) session_handle;

	if (table_entry->entry_type == PI_ACTION_ENTRY_TYPE_DATA) {
		pi_action_data_t *action_data = table_entry->entry.action_data;
		if (action_data) delete[] action_data;
	}

	return PI_STATUS_SUCCESS;
}

//! Delete an entry from a table using the entry handle. Should return an error
//! if entry does not exist.
pi_status_t _pi_table_entry_delete(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_entry_handle_t entry_handle) {
	(void) session_handle;

	const pi_p4info_t *info = infos[dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	uint32_t status;
	if ((status = table->deleteRule(entry_handle)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	return PI_STATUS_SUCCESS;
}

//! Delete an entry from a table using the match key. Should return an error
//! if entry does not exist.
pi_status_t _pi_table_entry_delete_wkey(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, const pi_match_key_t *match_key) {
	(void) session_handle;

	const pi_p4info_t *info = infos[dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	p4key_elem_t *key;
	uint32_t status, index;
	if ((status = createKeys(info, table_id, match_key, &key))) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	if ((status = table->findRule(key, index)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	if ((status = table->deleteRule(index)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	return PI_STATUS_SUCCESS;
}

//! Modify an existing entry using the entry handle. Should return an error if
//! entry does not exist.
pi_status_t _pi_table_entry_modify(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_entry_handle_t entry_handle, const pi_table_entry_t *table_entry) {
	(void) session_handle;

	const pi_p4info_t *info = infos[dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	pi_p4_id_t actionID = table_entry->entry.action_data->action_id;
	const char *actionData = table_entry->entry.action_data->data;
	const char *actionName = pi_p4info_action_name_from_id(info, actionID);

	p4param_t *params = NULL;
	uint32_t status;
	if ((status = createParams(info, actionID, actionData, &params)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	if ((status = table->modifyRule(entry_handle, actionName, params)) != P4DEV_OK)  {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	return PI_STATUS_SUCCESS;
}

//! Modify an existing entry using the match key. Should return an error if
//! entry does not exist.
pi_status_t _pi_table_entry_modify_wkey(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, const pi_match_key_t *match_key, const pi_table_entry_t *table_entry) {
	(void)session_handle;

	const pi_p4info_t *info = infos[dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_id].getTable(tableName);
	if (table == NULL) {
		std::cerr << "Cannot get table with name: " << tableName << "\n";
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	uint32_t status, index;
	p4key_elem_t *key;
	if ((status = createKeys(info, table_id, match_key, &key)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	if ((status = table->findRule(key, index)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	pi_p4_id_t actionID = table_entry->entry.action_data->action_id;
	const char *actionData = table_entry->entry.action_data->data;
	const char *actionName = pi_p4info_action_name_from_id(info, actionID);

	p4param_t *params = NULL;
	if ((status = createParams(info, actionID, actionData, &params)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	if ((status = table->modifyRule(index, actionName, params)) != P4DEV_OK) {
		p4dev_err_stderr(status);
		return pi_status_t(PI_STATUS_TARGET_ERROR + status);
	}

	return PI_STATUS_SUCCESS;
}

//! Retrieve all entries in table as one big blob.
pi_status_t _pi_table_entries_fetch(pi_session_handle_t session_handle, pi_dev_id_t dev_id, pi_p4_id_t table_id, pi_table_fetch_res_t *res) {
	(void) session_handle;

	const pi_p4info_t *info = infos[dev_id];
	assert(info != NULL);

	// Retrieve table handle
	const char *tableName = pi_p4info_table_name_from_id(info, table_id);
	p4::TablePtr table = devices[dev_id].getTable(tableName);
	if (table == NULL) {
		return PI_STATUS_NETV_INVALID_OBJ_ID;
	}

	res->p4info = info;
	res->num_direct_resources = res->num_entries;
	res->num_entries = table->getTableSize();
	size_t dataSize = 0U;

	dataSize += res->num_entries * sizeof(s_pi_entry_handle_t);
	dataSize += res->num_entries * sizeof(s_pi_action_entry_type_t);
	dataSize += res->num_entries * sizeof(uint32_t);  // for priority
	dataSize += res->num_entries * sizeof(uint32_t);  // for properties
	dataSize += res->num_entries * sizeof(uint32_t);  // for direct resources

	res->mkey_nbytes = pi_p4info_table_match_key_size(info, table_id);
	dataSize += res->num_entries * res->mkey_nbytes;

	size_t num_actions;
	auto actionIds = pi_p4info_table_get_actions(info, table_id, &num_actions);
	auto actionMap = computeActionSizes(info, actionIds, num_actions);

	for (uint32_t i = 0; i < res->num_entries; i++) {
		auto *rule = table->getRule(i);
		
		dataSize += actionMap.at(rule->action).size;
		dataSize += sizeof(s_pi_p4_id_t); // Action ID
		dataSize += sizeof(uint32_t); // Action params bytewidth
	}

	char *data = new char[dataSize];
	if (data == NULL) return PI_STATUS_ALLOC_ERROR;

	// in some cases, we do not use the whole buffer
	std::fill(data, data + dataSize, 0);
	res->entries_size = dataSize;
	res->entries = data;

	for (uint32_t i = 0; i < res->num_entries; i++) {
		auto *rule = table->getRule(i);

		data += emit_entry_handle(data, i);
		// We don't have priority yet
		data += emit_uint32(data, 0); // priority

		p4key_elem_t *key = rule->key;
		while (key != NULL) {

			switch (rule->engine) {
			case P4ENGINE_TCAM: // ternary
				std::memcpy(data, key->value, key->val_size);
				data += key->val_size;
				std::memcpy(data, key->opt.mask, key->val_size);
				data += key->val_size;
				break;

			case P4ENGINE_LPM:
				std::memcpy(data, key->value, key->val_size);
				data += key->val_size;
				data += emit_uint32(data, key->opt.prefix_len);
				break;

			case P4ENGINE_CUCKOO: //Exact
				std::memcpy(data, key->value, key->val_size);
				data += key->val_size;
				break;
			}

			key = key->next;
		}

		// Our actions are always direct
		data += emit_action_entry_type(data, PI_ACTION_ENTRY_TYPE_DATA);
		auto actionProperties = actionMap.at(rule->action);

		std::cerr << actionProperties.id << " " << actionProperties.size << "\n";

		data += emit_p4_id(data, actionProperties.id);
		data += emit_uint32(data, actionProperties.size);
		data = dumpActionData(info, data, actionProperties.id, rule->param);

		data += emit_uint32(data, 0);  // properties
		data += emit_uint32(data, 0);  // TODO(antonin): direct resources
	}

	return PI_STATUS_SUCCESS;
}

//! Need to be called after a pi_table_entries_fetch, once you wish the memory
//! to be released.
pi_status_t _pi_table_entries_fetch_done(pi_session_handle_t session_handle, pi_table_fetch_res_t *res) {
	(void) session_handle;
	
	delete[] res->entries;

	return PI_STATUS_SUCCESS;
}

}
