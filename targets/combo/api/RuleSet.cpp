#include "P4Dev.hpp"
#include <cassert>
#include <iostream>

using namespace p4;

void RuleSet::cleanup() {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::cleanup(...)\n";
	#endif
	
	for (uint32_t i = 0; i < rules.size(); i++) {
		if (isInvalidRule(rules[i])) {
			rules.erase(rules.begin() + i);
		}
	}
	
	for (uint32_t i = 0; i < tables.size(); i++) {
		tables[i]->recomputeIndices();
	}
}

uint32_t RuleSet::writeRules() {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::writeRules(...)\n";
	#endif
	
	uint32_t status;
	status = p4dev_disable(deviceInfo);
	if (status != P4DEV_OK) {
		return status;
	}
	
    p4rule_t **data = rules.data();
	status = p4dev_insert_rules(deviceInfo, (const p4rule_t**)(data), rules.size());
	if (status != P4DEV_OK) {
		return status;
	}
	
	status = p4dev_enable(deviceInfo);
	if (status != P4DEV_OK) {
		return status;
	}
	
	return P4DEV_OK;
}

uint32_t RuleSet::invalidateRule(const uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::invalidateRule(...)\n";
	#endif
	
	// TODO: rewrite
	p4rule_free(rules[index]);
	rules[index]->table_name = NULL;
	return P4DEV_OK;
}

bool RuleSet::isInvalidRule(p4rule_t *rule) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::isInvalidRule(...)\n";
	#endif
	
	// TODO: rewrite
	return rule->table_name == NULL;
}


uint32_t RuleSet::insertRule(p4rule_t *rule, uint32_t &index) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::insertRule(...)\n";
	#endif
	
	assert(rule != NULL);
	
	try {
		rules.push_back(rule);
	}
	catch (std::bad_alloc& ba) {
		return P4DEV_ALLOCATE_ERROR;
	}
	
	uint32_t status = writeRules();
	if (status != P4DEV_OK) {
		rules.pop_back();
		return status;
	}
	
	index = rules.size() - 1;
	return P4DEV_OK;
}

uint32_t RuleSet::modifyRule(p4rule *rule, uint32_t &index) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::modifyRule(...)\n";
	#endif
	
	p4rule_free(rules[index]);
	rules[index] = rule;
	
	uint32_t status = writeRules();
	if (status != P4DEV_OK) {
		rules.pop_back();
		return status;
	}
	
	return P4DEV_OK;
}

uint32_t RuleSet::deleteRule(const uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::deleteRule(...)\n";
	#endif
	
	uint32_t status;;
	if ((status = invalidateRule(index)) != P4DEV_OK) {
		return status;
	}
	
	deletedRulesCnt++;
	if ((rules.size() * 100 / deletedRulesCnt) >= DELETE_THRESHOLD_PERCENTAGE) {
		cleanup();
	}
	
	if ((status = writeRules()) != P4DEV_OK) {
		return status;
	}
	
	return status;
}

uint32_t RuleSet::addTablePointer(TablePtr tablePtr) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::addTablePointer(...)\n";
	#endif
	
	assert(tablePtr != NULL);
	
	try {
		tables.push_back(tablePtr);
	}
	catch (std::bad_alloc& ba) {
		return P4DEV_ALLOCATE_ERROR;
	}
	
	return P4DEV_OK;
}

p4rule_t *RuleSet::getRule(const uint32_t index) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::getRule(...)\n";
	#endif
	
	if (index >= rules.size()) {
		return NULL;
	}
	
	return rules[index];
}

void RuleSet::initialize(p4dev_t *deviceInfoPtr) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::initialize(...)\n";
	#endif
	
	assert(deviceInfoPtr != NULL);
	
	deviceInfo = deviceInfoPtr;
	deletedRulesCnt = 0;
}

void RuleSet::deinitialize() {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::deinitialize(...)\n";
	#endif
	
	clear();
	tables.clear();
}

void RuleSet::clear() {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::clear(...)\n";
	#endif
	
	for (uint32_t i = 0; i < rules.size(); i++) {
		p4rule_free(rules[i]);
	}
	rules.clear();
	deletedRulesCnt = 0;
}

void RuleSet::clearTable(const std::vector<uint32_t> &indices) {
	#ifdef DEBUG_LOGS
	std::cout << "RuleSet::clearTable(...)\n";
	#endif
	
	for (uint32_t i = 0; i < indices.size(); i++) {
		invalidateRule(indices[i]);
	}
	
	cleanup();
}

RuleSet::RuleSet() {
}

RuleSet::~RuleSet() {
	deinitialize();
}