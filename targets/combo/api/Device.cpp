#include "P4Dev.hpp"
#include <iostream>

using namespace p4;

TablePtr Device::getTable(const char *name) {
	#ifdef DEBUG_LOGS
	std::cout << "Device::getTable(...)\n";
	#endif
	
	auto iterator = tables.find(name);
	if (iterator == tables.end()) {
		return NULL;
	}
	
	return &(iterator->second);
}

uint32_t Device::initialize(char *name) {
	#ifdef DEBUG_LOGS
	std::cout << "Device::initialize(...)\n";
	#endif
	
	uint32_t status;
	if ((status = p4dev_init(&info, name)) != P4DEV_OK) {
		return status;
	}
	
	if ((status = registers.initialize(&info)) != P4DEV_OK) {
		deinitialize();
		return status;
	}
	
	char **tableNames;
	uint32_t nameCount;
	if ((status = p4dev_get_table_names(&info, &tableNames, &nameCount)) != P4DEV_OK) {
		deinitialize();
		return status;
	}
	
	try {
		tables.reserve(nameCount);
	}
	catch(...) {
		deinitialize();
		return P4DEV_ALLOCATE_ERROR;
	}
	
	for (uint32_t i = 0; i < nameCount; i++) {
		std::string name(tableNames[i]);
		tables[name].initialize(name.c_str(), &info);
	}
	
	p4dev_free_table_names(&tableNames, &nameCount);
	
	return P4DEV_OK;
}

void Device::deinitialize() {
	#ifdef DEBUG_LOGS
	std::cout << "Device::deinitialize(...)\n";
	#endif
	
	if (info.dt != NULL) {
		tables.clear();
		p4dev_free(&info);
	}
}

uint32_t Device::reset() {
	#ifdef DEBUG_LOGS
	std::cout << "Device::reset(...)\n";
	#endif
	
	uint32_t status = p4dev_reset_device(&info);
	if (status != P4DEV_OK) {
		return status;
	}
	
	for (auto table : tables) {
		table.second.clear();
	}
	/*for (auto iter = tables.begin(); iter != tables.end(); iter++) {
		iter->second.clear();
	}*/
	
	return status;
}

uint32_t Device::getTableList(std::vector<std::string> &names) {
	try {
		names.clear();
		names.reserve(tables.size());
	}
	catch (...) {
		return P4DEV_ALLOCATE_ERROR;
	}
	
	for (auto iter = tables.begin(); iter != tables.end(); iter++) {
		names.push_back(iter->first);
	}
	
	return P4DEV_OK;
}

Device::Device() {
	info.dt = NULL;
}

Device::~Device() {
	deinitialize();
}