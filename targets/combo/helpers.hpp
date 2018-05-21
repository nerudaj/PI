#ifndef HELPERS_HPP_
#define HELPERS_HPP_

#include "devices.hpp"
#include <PI/int/pi_int.h>
#include <PI/int/serialize.h>
#include <PI/p4info.h>
#include <PI/pi.h>
#include <unordered_map>

/**
 *  \brief Convert PI match engine enumeration into libp4dev match engine enumeration
 */
p4engine_type_t translateEngine(pi_p4info_match_type_t matchEngine);

/**
 *  \brief Export rule action data into raw byte array
 *  
 *  \param [in] info P4 Info for a device
 *  \param [in] data Array to export to
 *  \param [in] actionId PI id of a action
 *  \param [in] actionParams Action parameters in libp4dev format
 *  \return Pointer to 'data', now updated with dumped informations
 */
char *dumpActionData(const pi_p4info_t *info, char *data, pi_p4_id_t actionId, const p4param_t *actionParams);

/**
 *  \brief Export rule action data into pi_table_entry
 *  
 *  \param [in] info P4 Info for a device
 *  \param [in] actionName Name of the action
 *  \param [in] actionParams Action parameters in libp4dev format
 *  \param [in] table_entry Output entry
 *  \return PI_ACTION_ENTRY_TYPE_DATA or PI_STATUS_ALLOC_ERROR on failure, PI_STATUS_SUCCESS otherwise.
 */
pi_status_t retrieveEntry(const pi_p4info_t *info, const char *actionName, const p4param_t *actionParams, pi_table_entry_t *table_entry);

struct ActionProperties {
	uint32_t size;
	pi_p4_id_t id;

	ActionProperties(uint32_t s, pi_p4_id_t i) : size(s), id(i) {}
};

/**
 *  \brief Compute memory requirements for serializing each action defined in P4 Into
 *  
 *  \param [in] info P4 Info for a device
 *  \param [in] action_ids Array of PI ids of actions
 *  \param [in] num_actions Number of ids in array 'action_ids'
 *  \return Hash table indexed by action name and containing information about how many bytes are needed to serialize that action.
 */
std::unordered_map<std::string, ActionProperties> computeActionSizes(const pi_p4info_t *info, const pi_p4_id_t *action_ids, size_t num_actions);

/**
 *  \brief Swap endianness of a uint8_t array
 *  
 *  \param [in] data Array of bytes
 *  \param [in] dataSize Byte count
 *  
 *  \details Each nth byte is swapped with (dataSize - 1 - n)th byte.
 */
void flipEndianness(uint8_t *data, size_t dataSize);

#endif
