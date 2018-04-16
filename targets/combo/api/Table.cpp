#include "P4Dev.hpp"
#include <iostream>
#include <cassert>
#include <functional>

using p4::Table;

const uint32_t DEFAULT_RULE_INDEX = 0;
const uint32_t RULES_BEGIN = 1;

bool Table::keysMatch(const p4key_elem_t* first, const p4key_elem_t* second) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::keysMatch(...)\n";
	#endif
	
	assert(first != NULL);
	assert(second != NULL);
	assert(type != P4ENGINE_UNKNOWN); // NOTE: Verify and remove
	
	std::function<bool(const p4key_elem_t*, const p4key_elem_t*)> comparators[] = {
		tcam_p4key_cmp, bstlpm_p4key_cmp, cuckoo_p4key_cmp
	};
	
	return comparators[type](first, second);
}

uint32_t Table::deleteRuleRaw(uint32_t index) {
	p4rule_free(rules[index]);
	rules[index] = NULL;
	
	if (not (index == DEFAULT_RULE_INDEX)) {
		if (type == P4ENGINE_TCAM) {
			// Maintain the order of items (O(n) deletion)
			for (uint32_t i = index; i < getSize() + RULES_BEGIN - 1; i++) {
				rules[index] = rules[index + 1];
			}
			rules[RULES_BEGIN + getSize() - 1] = NULL;
		}
		else {
			// Swap with last item (O(1) deletion)
			uint32_t lastRule = RULES_BEGIN + getSize() - 1;
			rules[index] = rules[lastRule];
			rules[lastRule] = NULL;
		}
		
		size--;
	}
	
	return writeRules();
}

uint32_t Table::writeRules() {
	uint32_t status;
	status = p4dev_disable(deviceInfo);
	if (status != P4DEV_OK) {
		return status;
	}
	
	p4rule_t **data = rules.data();
	uint32_t rulesRealSize = getSize() + 1;
	if (not hasDefaultRule()) {
		data += RULES_BEGIN; // Skip empty default rule
		rulesRealSize--;
	}
	
	status = p4dev_initialize_table(deviceInfo, name.c_str());
	if (status != P4DEV_OK) {
		return status;
	}
	
	if (rulesRealSize != 0) {
		status = p4dev_insert_rules(deviceInfo, (const p4rule_t**)(data), rulesRealSize);
		if (status != P4DEV_OK) {
			return status;
		}
	}
	
	return p4dev_enable(deviceInfo);
}

uint32_t Table::insertRule(p4rule_t *rule, uint32_t &index, bool overwrite) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::insertRule(...)\n";
	#endif
	
	assert(rule != NULL);
	assert(strcmp(rule->table_name, name.c_str()) == 0);
	assert(rule->engine == type);

	// Check table capacity
	if (getSize() == getCapacity()) {
		std::cerr << "ERROR:Table - Table full\n";
		return P4DEV_ERROR; // TODO: P4DEV_TABLE_FULL;
	}
	
	// Verify rule is not already here
	uint32_t status, newIndex;
	if ((status = findRule(rule->key, newIndex)) == P4DEV_OK) {
		// If yes, we might overwrite it
		if (overwrite) {
			p4rule_free(rules[newIndex]);
		}
		else {
			std::cerr << "ERROR:Table - Rule already in the table\n";
			return P4DEV_ERROR;
		}
	}
	else {
		// If no, compute where to put the rule
		newIndex = RULES_BEGIN + getSize();
	}
	
	rules[newIndex] = rule;
	index = newIndex - RULES_BEGIN;
	if (index >= size) size++;
	
	return writeRules();
}

uint32_t Table::insertDefaultRule(p4rule_t *rule) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::insertDefaultRule(...)\n";
	#endif
	
	assert(rule != NULL);
	assert(strcmp(rule->table_name, name.c_str()) == 0);
	
	// Verify table does not have a default rule
	if (hasDefaultRule()) {
		return P4DEV_ERROR;
	}
	
	// Insert to ruleset
	rules[DEFAULT_RULE_INDEX] = rule;
	
	return writeRules();
}

uint32_t Table::modifyRule(uint32_t index, const char *actionName, p4param_t *params) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::modifyRule(...)\n";
	#endif
	
	assert(actionName != NULL);

	if (index >= getSize()) {
		return P4DEV_ERROR;
	}

	p4rule *rule = rules[RULES_BEGIN + index];
	assert(rule != NULL);
	
	uint32_t status;
	
	free((char *)(rule->action));
	if ((status = p4rule_add_action(rule, actionName)) != P4DEV_OK) {
		return status;
	}
	
	p4param_free(rule->param);
	rule->param = params;
	
	return writeRules();
}

uint32_t Table::deleteRule(uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::deleteRule(...)\n";
	#endif
	
	if (index >= getSize()) {
		return P4DEV_ERROR;
	}
	
	return deleteRuleRaw(RULES_BEGIN + index);
}

uint32_t Table::resetDefaultRule() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::resetDefaultRule(...)\n";
	#endif
	
	if (not hasDefaultRule()) {
		return P4DEV_ERROR;
	}
	
	return deleteRuleRaw(DEFAULT_RULE_INDEX);
}

uint32_t Table::findRule(p4key_elem_t* key, uint32_t &index) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::findRule(...)\n";
	#endif
	
	assert(key != NULL);
	
	for (uint32_t i = RULES_BEGIN; i < getSize() + RULES_BEGIN; i++) {
		if (keysMatch(rules[i]->key, key)) {
			index = i - RULES_BEGIN;
			return P4DEV_OK;
		}
	}
	
	return P4DEV_ERROR;
}

p4rule_t *p4::Table::getRule(uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::getRule(...)\n";
	#endif

	if (index >= getSize()) {
		return NULL;
	}

	return rules[RULES_BEGIN + index];
}

p4rule_t * p4::Table::getDefaultRule() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::getDefaultRule(...)\n";
	#endif

	if (not hasDefaultRule()) {
		return NULL;
	}

	return rules[DEFAULT_RULE_INDEX];
}

uint32_t Table::initialize(const char *name, p4dev_t *deviceInfoPtr) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::initialize(...)\n";
	#endif
	
	assert(name != NULL);
	assert(deviceInfoPtr != NULL);
	
	Table::name = name;
	deviceInfo = deviceInfoPtr;
	
	type = p4dev_get_table_type(deviceInfo, name);
	uint32_t capacity;
	uint32_t status = p4dev_get_table_capacity(deviceInfo, name, &capacity);
	if (status != P4DEV_OK) {
		return status;
	}
	
	try {
		rules.clear();
		rules.resize(capacity);
		size = 0;
	}
	catch (...) {
		return P4DEV_ALLOCATE_ERROR;
	}
	
	for (uint32_t i = 0; i < capacity; i++) rules[i] = NULL;
	
	return P4DEV_OK;
}

void Table::deinitialize() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::deinitialize(...)\n";
	#endif
	
	rules.clear();
	name.clear();
	deviceInfo = NULL;
}

uint32_t Table::clear() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::clear(...)\n";
	#endif
	
	uint32_t status = p4dev_initialize_table(deviceInfo, name.c_str());
	if (status != P4DEV_OK) {
		return status;
	}
	
	for (uint32_t i = 0; i < rules.size(); i++) {
		if (rules[i] != NULL) {
			p4rule_free(rules[i]);
			rules[i] = NULL;
		}
	}
	
	return P4DEV_OK;
}

Table::Table() {
	deviceInfo = NULL;
}

Table::Table(const char *name, p4dev_t *deviceInfoPtr) {
	initialize(name, deviceInfoPtr);
}

Table::~Table() {
	deinitialize();
}
