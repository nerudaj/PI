#ifndef TABLE_HPP_
#define TABLE_HPP_

#include <vector>
#include <string>
#include "RuleSet.hpp"

namespace p4 {
	class RuleSet;
	
	class Table {
	protected:
		std::string name; ///< Name of the table in P4 program
		p4dev_t *deviceInfo; ///< Information about the device running P4 program
		p4engine_type_t type; ///< Match engine used in the table
		uint32_t capacity; ///< Maximum records a table can hold
		std::vector<uint32_t> indices; ///< Indices of rules inside ruleset
		uint32_t defaultRuleIndex; ///< Has value of capacity if there's no default rule
		RuleSet *ruleset; ///< Pointer to class managing the rules
		
		bool keysMatch(const p4key_elem_t* first, const p4key_elem_t* second);
		bool hasDefaultRule() const { return defaultRuleIndex != 0xffffffff; }
		
	public:
		/**
		 *  \brief Insert rule to the table
		 *  
		 *  \param [in] rule Initialized rule object
		 *  \param [out] index Index of rule within table
		 *  \param [in] overwrite Force overwrite default rule
		 *  \return P4DEV_TABLE_FULL if table is already at maximum capacity. 
		 *  P4DEV_ALLOCATE_ERROR if the rule could not be stored in memory.
		 *  P4DEV_ERROR if the rule already exists.
		 *  P4DEV_OK on success.
		 *  
		 *  \details Inserted rule has to have matching table name and search 
		 *  engine as the table you're inserting it into. It also should not
		 *  already exist, unless the overwrite flag is set.
		 */
		uint32_t insertRule(p4rule_t *rule, uint32_t &index, bool overwrite = false);
		
		/**
		 *  \brief Inserts default rule to the table
		 *  
		 *  \param [in] rule Initialized rule object
		 *  \return P4DEV_TABLE_FULL if table is already at maximum capacity.
		 *  P4DEV_ALLOCATE_ERROR if the rule could not be stored in memory.
		 *  P4DEV_ERROR if table already has default rule.
		 *  P4DEV_OK on success.
		 */
		uint32_t insertDefaultRule(p4rule_t *rule);
		
		/**
		 *  \brief Brief description
		 *  
		 *  \param [in] rule Initialized rule object
		 *  \param [in] index Index of rule to modify
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		uint32_t modifyRule(p4rule_t *rule, uint32_t index);
		
		/**
		 *  \brief Deletes a rule on a given index
		 *  
		 *  \param [in] index Index of the rule to delete
		 *  \return P4DEV_ERROR if index is out of bounds. P4DEV_OK on success.
		 *  
		 *  \details Rule will be deleted from the memory, as well as from the 
		 *  card. Please note that deletion of the rule will change indices of 
		 *  rules with greater index by decrementing by one.
		 */
		uint32_t deleteRule(uint32_t index);
		
		/**
		 *  \brief Restores default rule from p4 program for the table
		 */
		uint32_t resetDefaultRule();
		
		/**
		 *  \brief Get an index of a certain rule based on its key
		 *  
		 *  \param [in] key A key to search by
		 *  \param [in] index Reference to variable where resulting index will
		 *  be stored
		 *  \return P4DEV_ERROR if the record was not found. P4DEV_OK 
		 *  on success.
		 *  
		 *  \details Only when rule is found (P4DEV_OK was returned) the value
		 *  of index is modified. Otherwise its value is undefined.
		 */
		uint32_t findRule(p4key_elem_t* key, uint32_t &index);
		
		/**
		 *  \brief Get pointer to rule object
		 *  
		 *  \param [in] index Index of rule within table
		 *  \return NULL if index is out of bounds, pointer to rule otherwise
		 *  
		 *  \note Calling deleteRule and deleteDefaultRule can affect validity
		 *  of your pointers.
		 */
		p4rule_t *getRule(uint32_t index);
		
		/**
		 *  \brief Get pointer to default rule object
		 *  
		 *  \return NULL if default rule was not set, pointer to rule otherwise
		 *  
		 *  \details You can only retrieve default rule you've set by yourself,
		 *  rule defined in p4 program cannot be retrieved this way.
		 */
		p4rule_t *getDefaultRule();

		/**
		 *  \brief Brief description
		 *  
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		uint32_t getTableSize() const { return indices.size(); }
		
		/**
		 *  \brief Prepares instantion to be used
		 *  
		 *  \param [in] name String identifier of the table in P4 program
		 *  \param [in] rulesetPtr Pointer to instance of RuleSet
		 *  \param [in] deviceInfoPtr Pointer to instance of device info
		 *  \return API status (TODO list status)
		 */
		uint32_t initialize(const char *name, RuleSet *rulesetPtr, p4dev_t *deviceInfoPtr);
		
		/**
		 *  \brief Sets the class instance to initial state
		 *  
		 *  \details This method does not affect the state of the card.
		 */
		void deinitialize();
		
		/**
		 *  \brief Clears all rules from the table
		 *  
		 *  \return API status for call to p4dev_initialize_table
		 *  
		 *  \details Sets the table to its initial state - clears all rules from 
		 *  card and ram.
		 */
		uint32_t clear();
		
		/**
		 *  \brief Clears the indices list and rebuilds it based on current 
		 *  state of RuleSet
		 */
		void recomputeIndices();
		
		Table();
		Table(const char *name, RuleSet *rulesetPtr, p4dev_t *deviceInfoPtr);
		~Table();
	};
    
    typedef Table* TablePtr;
}

#endif
