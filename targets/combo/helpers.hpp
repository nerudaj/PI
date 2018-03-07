#ifndef HELPERS_HPP_
#define HELPERS_HPP_

#include "devices.hpp"
#include <PI/int/pi_int.h>
#include <PI/int/serialize.h>
#include <PI/p4info.h>
#include <PI/pi.h>
#include <unordered_map>

p4engine_type_t translateEngine(pi_p4_id_t matchEngine);

char *dumpActionData(const pi_p4info_t *info, char *data, pi_p4_id_t actionId, const p4param_t *actionParams);

/*
	Returns PI_STATUS_SUCCESS or PI_ALLOC_ERROR
*/
pi_status_t retrieveEntry(const pi_p4info_t *info, const char *actionName, const p4param_t *actionParams, pi_table_entry_t *table_entry);

struct ActionProperties {
	uint32_t size;
	pi_p4_id_t id;

	ActionProperties(uint32_t s, pi_p4_id_t i) : size(s), id(i) {}
};

std::unordered_map<std::string, ActionProperties> computeActionSizes(const pi_p4info_t *info, const pi_p4_id_t *action_ids, size_t num_actions);

#endif
