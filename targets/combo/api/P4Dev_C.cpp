#include "P4Dev_C.h"
#include "P4Dev.hpp"
#include <vector>
#include <iostream>
#include <cassert>

// *** DEVICE ***

uint32_t Device_initialize(const p4dev_name_t name, DevicePtr *device) {
	*device = new p4::Device;
	if (*device == NULL) {
		return P4DEV_ERROR;
	}
	
	p4::Device *dev = dynamic_cast<p4::Device*>((p4::Device*)(*device));
	return dev->initialize(name);
}

void Device_deinitialize(DevicePtr *device) {
	assert(*device != NULL);
	
	if (p4::Device *dev = dynamic_cast<p4::Device*>((p4::Device*)(*device))) {
		delete dev;
	}

	device = NULL;
}

uint32_t Device_reset(const DevicePtr device) {
	assert(device != NULL);
	
	if (p4::Device *dev = dynamic_cast<p4::Device*>((p4::Device*)(device))) {
		return dev->reset();
	}
	
	return P4DEV_ERROR;
}

TablePtr Device_getTable(const DevicePtr device, const char *name) {
	assert(device != NULL);
	
	if (p4::Device *dev = dynamic_cast<p4::Device*>((p4::Device*)(device))) {
		return dev->getTable(name);
	}
	
	return P4DEV_ERROR;
}

// *** TABLE ***

uint32_t Table_insertRule(const TablePtr table, p4rule_t *rule) {
	assert(table != NULL);
	
	if (p4::Table* t = dynamic_cast<p4::Table*>((p4::Table*)(table))) {
		return t->insertRule(rule);
	}
	
	return P4DEV_ERROR;
}

uint32_t Table_modifyRule(const TablePtr table, uint32_t index/*, /* data */) {
	assert(table != NULL);
	
	p4::Table* t = dynamic_cast<p4::Table*>((p4::Table*)(table));
	// TODO: check corectness
	
	return P4DEV_NOT_IMPLEMENTED;//t->modifyRule(index);
}

uint32_t Table_deleteRule(const TablePtr table, uint32_t index) {
	assert(table != NULL);
	
	if (p4::Table* t = dynamic_cast<p4::Table*>((p4::Table*)(table))) {
		return t->deleteRule(index);
	}
	
	return P4DEV_ERROR;
}

uint32_t Table_findRule(const TablePtr table, p4key_elem_t* key, uint32_t *index) {
	assert(table != NULL);
	
	if (p4::Table* t = dynamic_cast<p4::Table*>((p4::Table*)(table))) {
		return t->findRule(key, *index);
	}
	
	return P4DEV_ERROR;
}

void Table_recomputeIndices(const TablePtr table) {
	assert(table != NULL);
	
	if (p4::Table* t = dynamic_cast<p4::Table*>((p4::Table*)(table))) {
		t->recomputeIndices();
	}
}

uint32_t Table_clear(const TablePtr table) {
	assert(table != NULL);
	
	if (p4::Table* t = dynamic_cast<p4::Table*>((p4::Table*)(table))) {
		return t->clear();
	}
	
	return P4DEV_ERROR;
}
