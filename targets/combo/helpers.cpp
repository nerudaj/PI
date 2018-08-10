#include "helpers.hpp"
#include <iostream>

p4engine_type_t translateEngine(pi_p4info_match_type_t matchEngine) {
	switch (matchEngine) {
	case PI_P4INFO_MATCH_TYPE_EXACT:
		return P4ENGINE_EXACT;
	case PI_P4INFO_MATCH_TYPE_LPM:
		return P4ENGINE_LPM;
	case PI_P4INFO_MATCH_TYPE_TERNARY:
		return P4ENGINE_TERNARY;
	}

	return P4ENGINE_UNKNOWN;
}

char *dumpActionData(const pi_p4info_t *info, char *data, pi_p4_id_t actionId, const p4param_t *actionParams) {
	size_t paramCount;
	const pi_p4_id_t *paramIds = pi_p4info_action_get_params(info, actionId, &paramCount);

	size_t i = 0;
	const p4param_t *param = actionParams;
	while (param != NULL) {
		size_t bitwidth = pi_p4info_action_param_bitwidth(info, actionId, paramIds[i]);
		size_t bytewidth = (bitwidth + 7) / 8;

		assert(bytewidth >= param->value_size);

		flipEndianness(param->value, param->value_size); // Flip everything that is uint8_t array on I/O communication with libp4dev
		
		size_t offset = bytewidth - param->value_size;
		std::memset(data, 0, offset);
		std::memcpy(data + offset, param->value, param->value_size);
		data += bytewidth;
		i++;
		param = param->next;
	}

	return data;
}

pi_status_t retrieveEntry(const pi_p4info_t *info, const char *actionName, const p4param_t *actionParams, pi_table_entry_t *table_entry) {
	const pi_p4_id_t actionId = pi_p4info_action_id_from_name(info, actionName);
	const size_t actionDataSize = pi_p4info_action_data_size(info, actionId);

	table_entry->entry_type = PI_ACTION_ENTRY_TYPE_DATA;

	char *data_ = new char[sizeof(pi_action_data_t) + actionDataSize];
	if (data_ == NULL) return PI_STATUS_ALLOC_ERROR;
	pi_action_data_t *actionData = (pi_action_data_t *)(data_);
	data_ += sizeof(pi_action_data_t);

	actionData->p4info = info;
	actionData->action_id = actionId;
	actionData->data_size = actionDataSize;
	actionData->data = data_;

	table_entry->entry.action_data = actionData;

	data_ = dumpActionData(info, data_, actionId, actionParams);

	return PI_STATUS_SUCCESS;
}

std::unordered_map<std::string, ActionProperties> computeActionSizes(const pi_p4info_t *info, const pi_p4_id_t * actionIds, size_t actionCount) {
	std::unordered_map<std::string, ActionProperties> result;
	result.reserve(actionCount);

	for (size_t i = 0; i < actionCount; i++) {
		result.emplace(
			std::string(pi_p4info_action_name_from_id(info, actionIds[i])),
			ActionProperties(pi_p4info_action_data_size(info, actionIds[i]), actionIds[i])
		);
	}

	return result;
}

void flipEndianness(uint8_t *data, size_t dataSize) {
	for (unsigned i = 0; i < dataSize / 2; i++) {
		std::swap(data[i], data[dataSize - 1 - i]);
	}
}
