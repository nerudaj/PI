#include "p4rule.h"

API p4rule_t * p4rule_create(const char * table, const p4engine_type_t engine) {
	assert(table != NULL);
	
	p4rule_t *result = (p4rule_t*)(malloc(sizeof(p4rule_t)));
	if (result == NULL) return NULL;
	
	result->table_name = strndup(table, strlen(table));
	
	result->engine = engine;
	result->key = NULL;
	result->action = NULL;
	result->param = NULL;
	result->def = false;
	result->private_param = NULL;
	
	return result;
}

API void p4rule_mark_default(p4rule_t* rule) {
	assert(rule != NULL);
	
	rule->def = true;
}

API uint32_t p4rule_add_key_element(p4rule_t * rule, p4key_elem_t * key) {
	assert(rule != NULL);
	assert(key != NULL);
	fprintf(stderr, "P4RULE_ADD_KEY_ELEM: %p %p\n", rule, key);
	
	if (rule->key == NULL) {
		rule->key = key;
		return P4DEV_OK;
	}
	
	p4key_elem_t *lastValid = rule->key;
	while (lastValid->next != NULL) lastValid = lastValid->next;
	lastValid->next = key;
	lastValid->next->next = NULL;
	
	return P4DEV_OK;
}

API uint32_t p4rule_add_action(p4rule_t * rule, const char * action) {
	assert(rule != NULL);
	assert(action != NULL);
	
	rule->action = strndup(action, strlen(action));
	if (rule->action == NULL) return P4DEV_ALLOCATE_ERROR;
	
	strcpy(rule->action, action);
	
	return P4DEV_OK;
}

API uint32_t p4rule_add_param(p4rule_t * rule, p4param_t * param) {
	assert(rule != NULL);
	assert(param != NULL);
	
	rule->param = param;
	
	return P4DEV_OK;
}

API void p4rule_free(p4rule_t * rule) {
	#ifdef LOGGING_ENABLED
	printf("LOG: Freeing rule\n");
	#endif
	if (rule->table_name) free(rule->table_name);
	if (rule->action != NULL) free(rule->action);
	if (rule->param != NULL) p4param_free(rule->param);
	
	if (rule->key != NULL) {
		switch(rule->engine) {
		case P4ENGINE_TCAM:
			tcam_p4key_free(rule->key);
			break;
		case P4ENGINE_LPM:
			bstlpm_p4key_free(rule->key);
			break;
		case P4ENGINE_CUCKOO:
			cuckoo_p4key_free(rule->key);
			break;
		}
	}
	
	free(rule);
}

API p4param_t * p4param_create(const char * name, uint32_t size, uint8_t * value) {
	p4param_t *result = (p4param_t*)(malloc(sizeof(p4param_t)));
	if (result == NULL) return NULL;
	
	result->param_name = strndup(name, strlen(name));
	result->val_size = size;
	result->value = NULL;
	result->next = NULL;
	
	return result;
}

API void p4param_free(p4param_t * param) {
	if (param->next != NULL) {
		p4param_free(param->next);
	}
	free(param->param_name);
	
	free(param);
}

API p4key_elem_t * tcam_p4key_create(const char * name, uint32_t size, uint8_t * value, uint8_t * mask) {
	p4key_elem_t *result = (p4key_elem_t*)(malloc(sizeof(p4key_elem_t)));
	if (result == NULL) {
		return NULL;
	}
	
	result->name = NULL;
	result->value = NULL;
	result->val_size = size;
	result->next = NULL;
	
	return result;
}

API void tcam_p4key_free(p4key_elem_t * key) {
	if (key->next != NULL) tcam_p4key_free(key->next);
	free(key);
}

API bool tcam_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2) {
	return false;
}

API p4key_elem_t * cuckoo_p4key_create(const char * name, uint32_t size, uint8_t * value) {
	p4key_elem_t *result = (p4key_elem_t*)(malloc(sizeof(p4key_elem_t)));
	if (result == NULL) {
		return NULL;
	}
	
	result->name = NULL;
	result->value = NULL;
	result->val_size = size;
	result->next = NULL;
	
	return result;
}

API void cuckoo_p4key_free(p4key_elem_t * key) {
	if (key->next != NULL) cuckoo_p4key_free(key->next);
	free(key);
}

API bool cuckoo_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2) {
	return false;
}

API p4key_elem_t * bstlpm_p4key_create(const char * name, uint32_t size, uint8_t * value, uint32_t prefix_len) {
	p4key_elem_t *result = (p4key_elem_t*)(malloc(sizeof(p4key_elem_t)));
	if (result == NULL) {
		return NULL;
	}
	
	result->name = name;
	result->value = NULL;
	result->val_size = size;
	result->next = NULL;
	
	return result;
}

API void bstlpm_p4key_free(p4key_elem_t * key) {
	if (key->next != NULL) bstlpm_p4key_free(key->next);
	free(key);
}

API bool bstlpm_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2) {
	return false;
}