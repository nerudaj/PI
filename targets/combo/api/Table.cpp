#include "P4Dev.hpp"
#include <iostream>
#include <cassert>
#include <functional>

using p4::Table;

#define UINT32_MAX 0xffffffff

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

uint32_t Table::insertRule(p4rule_t *rule, uint32_t &index, bool overwrite) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::insertRule(...)\n";
	#endif
	
	assert(rule != NULL);
	assert(strcmp(rule->table_name, name.c_str()) == 0);

	// Check table capacity
	if (capacity == indices.size()) {
		std::cerr << "ERROR:Table - Table full\n";
		return P4DEV_ERROR; // TODO: P4DEV_TABLE_FULL;
	}
	
	// Verify rule is not already here
	uint32_t status, auxIndex;
	if ((status = findRule(rule->key, auxIndex)) == P4DEV_OK) {
		// If yes, we might overwrite it
		if (overwrite) {
			if ((status = ruleset->overwriteRule(rule, auxIndex)) != P4DEV_OK) {
				return status;
			}
			
			return P4DEV_OK;
		}
		else {
			std::cerr << "ERROR:Table - Rule already in the table\n";
			return P4DEV_ERROR;
		}
	}
	
	// Insert to ruleset
	if ((status = ruleset->insertRule(rule, index)) != P4DEV_OK) {
		return status;
	}
	
	// Build index
	try {
		indices.push_back(index);
	}
	catch (std::bad_alloc& ba) {
		ruleset->deleteRule(index);
		return P4DEV_ALLOCATE_ERROR;
	}
	
	return P4DEV_OK;
}

uint32_t Table::insertDefaultRule(p4rule_t *rule) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::insertDefaultRule(...)\n";
	#endif
	
	assert(rule != NULL);
	assert(strcmp(rule->table_name, name.c_str()) == 0);
	
	// Check table capacity
	if (capacity == indices.size()) {
		std::cerr << "ERROR:Table - Table full\n";
		return P4DEV_ERROR; // TODO: P4DEV_TABLE_FULL;
	}
	
	// Verify table does not have a default rule
	if (hasDefaultRule()) {
		return P4DEV_ERROR;
	}
	
	// Insert to ruleset
	uint32_t status;
	if ((status = ruleset->insertRule(rule, defaultRuleIndex)) != P4DEV_OK) {
		return status;
	}
	
	return P4DEV_OK;
}

uint32_t Table::modifyRule(uint32_t index, const char *actionName, p4param_t *params) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::modifyRule(...)\n";
	#endif
	
	assert(actionName != NULL);
	assert(params != NULL);

	if (index >= indices.size()) {
		return P4DEV_ERROR;
	}

	uint32_t status;
	if ((status = ruleset->modifyRule(indices[index], actionName, params)) != P4DEV_OK) {
		return P4DEV_OK;
	}

	return P4DEV_OK;
}

uint32_t Table::deleteRule(uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::deleteRule(...)\n";
	#endif
	
	if (index >= indices.size()) {
		return P4DEV_ERROR;
	}
	
	uint32_t status, rulesetIndex = indices[index];
	indices.erase(indices.begin() + index);
	if ((status = ruleset->deleteRule(rulesetIndex)) != P4DEV_OK) {
		indices.erase(indices.begin() + index);
		return status;
	}
	
	return P4DEV_OK;
}

uint32_t Table::resetDefaultRule() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::resetDefaultRule(...)\n";
	#endif
	
	if (not hasDefaultRule()) {
		return P4DEV_ERROR;
	}
	
	uint32_t status;
	if ((status = ruleset->deleteRule(defaultRuleIndex)) != P4DEV_OK) {
		return status;
	}
	
	defaultRuleIndex = UINT32_MAX;
	
	return P4DEV_OK;
}

uint32_t Table::findRule(p4key_elem_t* key, uint32_t &index) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::findRule(...)\n";
	#endif
	
	assert(key != NULL);
	
	for (uint32_t i = 0; i < indices.size(); i++) {
		if (keysMatch(ruleset->getRule(indices[i])->key, key)) {
			index = indices[i];
			return P4DEV_OK;
		}
	}
	
	return P4DEV_ERROR;
}

p4rule_t *p4::Table::getRule(uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::getRule(...)\n";
	#endif

	if (index >= indices.size()) {
		return NULL;
	}

	return ruleset->getRule(indices[index]);
}

p4rule_t * p4::Table::getDefaultRule() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::getDefaultRule(...)\n";
	#endif

	if (not hasDefaultRule()) {
		return NULL;
	}

	return ruleset->getRule(defaultRuleIndex);
}

uint32_t Table::initialize(const char *name, RuleSet *rulesetPtr, p4dev_t *deviceInfoPtr) {
	#ifdef DEBUG_LOGS
	std::cout << "Table::initialize(...)\n";
	#endif
	
	assert(name != NULL);
	assert(rulesetPtr != NULL);
	assert(deviceInfoPtr != NULL);
	
	Table::name = name;
	deviceInfo = deviceInfoPtr;
	indices.clear();
	
	type = p4dev_get_table_type(deviceInfo, name);
	uint32_t status = p4dev_get_table_capacity(deviceInfo, name, &capacity);
	if (status != P4DEV_OK) {
		return status;
	}
	defaultRuleIndex = UINT32_MAX;
	
	/*
	NOTE: Preallocate memory?
	try {
		indices.reserve(capacity);
	}
	catch (std::bad_alloc &e) {
		return P4DEV_ALLOCATE_ERROR;
	}
	catch (std::length_error &e) {
		return P4DEV_ALLOCATE_ERROR;
	}*/
	
	ruleset = rulesetPtr;
	
	return P4DEV_OK;
}

void Table::deinitialize() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::deinitialize(...)\n";
	#endif
	
	indices.clear();
	name.clear();
	ruleset = NULL;
	deviceInfo = NULL;
}

void Table::recomputeIndices() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::recomputeIndices(...)\n";
	#endif
	
	indices.clear();
	uint32_t size = ruleset->getSize();
	
	p4rule *rule;
	for (uint32_t i = 0; i < size; i++) {
		rule = ruleset->getRule(i);
		
		if (strcmp(rule->table_name, name.c_str()) == 0) {
			if (rule->def) {
				defaultRuleIndex = i;
			}
			else {
				// NOTE: Realloc might have happened
				indices.push_back(i);
			}
		}
	}
}

uint32_t Table::clear() {
	#ifdef DEBUG_LOGS
	std::cout << "Table::clear(...)\n";
	#endif
	
	uint32_t status = p4dev_initialize_table(deviceInfo, name.c_str());
	if (status != P4DEV_OK) {
		return status;
	}
	
	defaultRuleIndex = UINT32_MAX;
	ruleset->clearTable(indices);
	indices.clear();
	
	return P4DEV_OK;
}

Table::Table() {
	deviceInfo = NULL;
	ruleset = NULL;
}

Table::Table(const char *name, RuleSet *rulesetPtr, p4dev_t *deviceInfoPtr) {
	initialize(name, rulesetPtr, deviceInfoPtr);
}

Table::~Table() {
	deinitialize();
}
