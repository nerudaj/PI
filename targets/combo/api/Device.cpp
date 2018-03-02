#include "P4Dev.hpp"
#include <iostream>

using namespace p4;

TablePtr Device::getTable(const char *name) {
	auto iterator = tables.find(name);
	if (iterator == tables.end()) {
		return NULL;
	}
	
	return iterator->second;
}

uint32_t Device::initialize(const p4dev_name_t name) {
	uint32_t status;
	if ((status = p4dev_init(&info, name)) != P4DEV_OK) {
		return status;
	}
	
	ruleset.initialize(&info);
	
	char **tableNames;
	uint32_t nameCount;
	if ((status = p4dev_get_table_names(&info, &tableNames, &nameCount)) != P4DEV_OK) {
		deinitialize();
		return status;
	}
	
	for (uint32_t i = 0; i < nameCount; i++) {
		std::string name(tableNames[i]);
		tables[name] = new Table;
		if (tables[name] == NULL) {
			p4dev_free_table_names(&tableNames, &nameCount);
			deinitialize();
			return P4DEV_ALLOCATE_ERROR;
		}
		
		tables[name]->initialize(name.c_str(), &ruleset, &info);
		ruleset.addTablePointer(tables[name]);
	}
	
	p4dev_free_table_names(&tableNames, &nameCount);
	
	return P4DEV_OK;
}

void Device::deinitialize() {
	if (info.dt != NULL) {
		tables.clear();
		ruleset.clear();
		p4dev_free(&info);
	}
}

uint32_t Device::reset() {
	uint32_t status = p4dev_reset_device(&info);
	if (status != P4DEV_OK) {
		return status;
	}
	
	ruleset.clear();
	for (auto iter = tables.begin(); iter != tables.end(); iter++) {
		iter->second->recomputeIndices();
	}
	
	return status;
}

Device::Device() {
	info.dt = NULL;
}

Device::~Device() {
	std::cerr << "ERR: Device destructor\n";
	deinitialize();
}