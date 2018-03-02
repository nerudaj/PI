#ifndef P4DEV_C_HPP_
#define P4DEV_C_HPP_

#include <p4dev.h>
#include <p4dev_types.h>
#include <p4rule.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

typedef void* DevicePtr;
typedef void* TablePtr;

// *** DEVICE ***

uint32_t Device_initialize(const p4dev_name_t name, DevicePtr *device);

void Device_deinitialize(DevicePtr *device);

uint32_t Device_reset(const DevicePtr device);

TablePtr Device_getTable(const DevicePtr device, const char *name);

// *** TABLE ***

uint32_t Table_insertRule(const TablePtr table, p4rule_t *rule);

uint32_t Table_modifyRule(const TablePtr table, uint32_t index/*,  data */);

uint32_t Table_deleteRule(const TablePtr table, uint32_t index);

uint32_t Table_findRule(const TablePtr table, p4key_elem_t* key, uint32_t *index);

void Table_recomputeIndices(const TablePtr table);

uint32_t Table_clear(const TablePtr table);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif
