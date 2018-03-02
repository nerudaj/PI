#include "P4Dev.hpp"
#include <cassert>
#include <iostream>

using namespace p4;

void RuleSet::cleanup() {
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
	uint32_t status;
	status = p4dev_enable(deviceInfo);	
	if (status != P4DEV_OK) {
		return status;
	}
	
    p4rule_t **data = rules.data();
	status = p4dev_insert_rules(deviceInfo, (const p4rule_t**)(data), rules.size());
	if (status != P4DEV_OK) {
		return status;
	}
	
	status = p4dev_disable(deviceInfo);
	if (status != P4DEV_OK) {
		return status;
	}
	
	return status;
}

uint32_t RuleSet::invalidateRule(const uint32_t index) {
	// TODO: rewrite
	p4rule_free(rules[index]);
	rules[index]->table_name = NULL;
	return P4DEV_OK;
}

bool RuleSet::isInvalidRule(p4rule_t *rule) {
	assert(rule != NULL);
	// TODO: rewrite
	return rule->table_name == NULL;
}


uint32_t RuleSet::insertRule(p4rule_t *rule, uint32_t &index) {
	assert(rule != NULL);
	
	try {
		rules.push_back(rule);
	}
	catch (std::bad_alloc& ba) {
		return P4DEV_ALLOCATE_ERROR;
	}
	
	uint32_t status = writeRules();
	if (status != P4DEV_OK) {
		return status;
	}
	
	index = rules.size() - 1;
	return P4DEV_OK;
}

uint32_t RuleSet::modifyRule() {
	// TODO: this
	return P4DEV_ERROR;
}

uint32_t RuleSet::deleteRule(const uint32_t index) {
	uint32_t status = invalidateRule(index);
	if (status != P4DEV_OK) {
		return status;
	}
	
	deletedRulesCnt++;
	if ((rules.size() * 100 / deletedRulesCnt) >= DELETE_THRESHOLD_PERCENTAGE) {
		cleanup();
	}
	
	return status;
}

uint32_t RuleSet::addTablePointer(TablePtr tablePtr) {
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
	if (index >= rules.size()) {
		return NULL;
	}
	
	return rules[index];
}

void RuleSet::initialize(p4dev_t *deviceInfoPtr) {
	assert(deviceInfoPtr != NULL);
	
	deviceInfo = deviceInfoPtr;
	deletedRulesCnt = 0;
}

void RuleSet::deinitialize() {
	clear();
	tables.clear();
}

void RuleSet::clear() {
	for (uint32_t i = 0; i < rules.size(); i++) {
		p4rule_free(rules[i]);
	}
	rules.clear();
	deletedRulesCnt = 0;
}

void RuleSet::clearTable(const std::vector<uint32_t> &indices) {
	for (uint32_t i = 0; i < indices.size(); i++) {
		invalidateRule(indices[i]);
	}
	
	cleanup();
}

RuleSet::RuleSet() {
	
}

RuleSet::~RuleSet() {
	std::cerr << "ERR: RuleSet destructor\n";
	deinitialize();
}