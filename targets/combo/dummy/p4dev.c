#include "p4dev.h"

const uint32_t TABLE_COUNT = 3;

char TABLE_NAMES[][16] = {
	"ipv4_lpm",
	"send_frame",
	"forward"
};

uint32_t TABLE_CAPACITIES[] = {
	2, 16, 128
};

p4engine_type_t TABLE_TYPES[] = {
	P4ENGINE_LPM, P4ENGINE_CUCKOO, P4ENGINE_TCAM
};

API uint32_t p4dev_direct_init(const void* dt, p4dev_t* dev, const p4dev_name_t name) {
	return P4DEV_OK;
}

API uint32_t p4dev_init(p4dev_t* dev, const p4dev_name_t name) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Init device\n");
	#endif
	
	int *ptr = (int*)malloc(sizeof(int) * 1000);
	dev->dt = (void*)ptr;
	dev->dt_p4offset = 0;
	
	return P4DEV_OK;
}

API void p4dev_free(p4dev_t* dev) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Free device\n");
	#endif
	
	free(dev->dt);
	dev->dt = NULL;
	dev->dt_p4offset = NULL;
}

API uint32_t p4dev_insert_rules(const p4dev_t* dev, const p4rule_t** p4rules, uint32_t rule_count) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Insert rules. Rule count: %d\n", rule_count);
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_initialize_table(const p4dev_t* dev, const char* name) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Initialize table with name %s\n", name);
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_reset_device(const p4dev_t* dev) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Reset device\n");
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_enable(const p4dev_t* dev) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Enable device\n");
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_disable(const p4dev_t* dev) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Disable device\n");
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_get_table_capacity(const p4dev_t* dev, const char* name, uint32_t* capacity) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Get table capacity\n");
	#endif
	
	for (int i = 0; i < TABLE_COUNT; i++) {
		if (strcmp(TABLE_NAMES[i], name) == 0) {
			*capacity = TABLE_CAPACITIES[i];
		}
	}
	
	return P4DEV_OK;
}

API uint32_t p4dev_get_table_names(const p4dev_t* dev, char*** names, uint32_t* count) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Get table names\n");
	#endif
	
	*count = TABLE_COUNT;
	*names = (char**)(malloc(sizeof(char*) * 3));
	for (int i = 0; i < *count; i++) {
		(*names)[i] = (char*)(malloc(sizeof(char) * 16));
		strcpy((*names)[i], TABLE_NAMES[i]);
	}
	
	return P4DEV_OK;
}

API void p4dev_free_table_names(char*** names, uint32_t* count) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Free table names\n");
	#endif
	
	for (int i = 0; i < *count; i++) {
		free((*names)[i]);
	}
	free(*names);
	*names = NULL;
	*count = 0;
}

API p4engine_type_t  p4dev_get_table_type(const p4dev_t* dev, const char* name) {
	#ifdef DEBUG_LOGS
	printf("p4dev: Get type of table %s\n", name);
	#endif
	
	for (int i = 0; i < TABLE_COUNT; i++) {
		if (strcmp(TABLE_NAMES[i], name) == 0) {
			return TABLE_TYPES[i];
		}
	}
	
	return P4ENGINE_UNKNOWN;
}