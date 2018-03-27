#ifndef TABLE_HPP_
#define TABLE_HPP_

#include <vector>
#include <string>

namespace p4 {
	/**
	 *  \brief Class representing abstraction over P4 table
	 *  
	 *  \details Objects of this class are not supposed to be
	 *  instantiated directly. They always should be instantiated
	 *  by Device::initialize() and pointers to them should be
	 *  retrieved by Device::getTable. Only then the proper
	 *  behaviour can be guaranteed.
	 */
	class Table {
	protected:
		std::string name; ///< Name of the table in P4 program
		p4dev_t *deviceInfo; ///< Information about the device running P4 program
		p4engine_type_t type; ///< Match engine used in the table
		std::vector<p4rule_t*> rules; /// Rule storage
		uint32_t size; ///< How many rules are stored here
		
		bool keysMatch(const p4key_elem_t* first, const p4key_elem_t* second);
		bool hasDefaultRule() const { return rules[0] != NULL; }
		uint32_t deleteRuleRaw(uint32_t index);
		uint32_t writeRules();
		void deinitialize();
		
	public:
		/**
		 *  \brief Insert rule to the table
		 *  
		 *  \param [in] rule Initialized rule object
		 *  \param [out] index Index of rule within table
		 *  \param [in] overwrite Force overwrite default rule
		 *  \return API status code:
		 *      - P4DEV_OK - rule is inserted
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
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
		 *  \return API status code:
		 *      - P4DEV_OK - default rule is inserted
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
		 */
		uint32_t insertDefaultRule(p4rule_t *rule);
		
		/**
		 *  \brief Change action of an existing rule
		 *  
		 *  \param [in] index Index of rule to modify
		 *  \param [in] actionName Name of the new action
		 *  \param [in] params Parameters of the new action
		 *  \return API status code:
		 *      - P4DEV_OK - rule was modified
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
		 */
		uint32_t modifyRule(uint32_t index, const char *actionName, p4param_t *params);
		
		/**
		 *  \brief Deletes a rule on a given index
		 *  
		 *  \param [in] index Index of the rule to delete
		 *  \return API status code:
		 *      - P4DEV_OK - rule was deleted
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
		 *  
		 *  \details Rule will be deleted from the memory, as well as from the 
		 *  card. Please note that deletion of the rule will change indices of 
		 *  rules with greater index by decrementing by one.
		 */
		uint32_t deleteRule(uint32_t index);
		
		/**
		 *  \brief Restores default rule from p4 program for the table
		 *  
		 *  \return API status code:
		 *      - P4DEV_OK - default rule was reset
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
		 */
		uint32_t resetDefaultRule();
		
		/**
		 *  \brief Get an index of a certain rule based on its key
		 *  
		 *  \param [in] key A key to search by
		 *  \param [in] index Reference to variable where resulting index will
		 *  be stored
		 *  \return API status code:
		 *      - P4DEV_OK - rule was found
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
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
		 *  \brief Returns current number of rules stored in the table
		 *  
		 *  \details This number does not include default rule.
		 */
		uint32_t getTableSize() const { return size; }
		
		/**
		 *  \brief Returns maximum number of rules that can be stored in the table
		 *  
		 *  \details This number does not include default rule.
		 */
		uint32_t getTableCapacity() const { return rules.size() - 1; }
		
		/**
		 *  \brief Prepares instantion to be used
		 *  
		 *  \param [in] name String identifier of the table in P4 program
		 *  \param [in] rulesetPtr Pointer to instance of RuleSet
		 *  \param [in] deviceInfoPtr Pointer to instance of device info
		 *  \return API status code:
		 *      - P4DEV_OK - table is prepared
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
		 *  
		 *  \details This method should not be called directly, it should
		 *  always be called by Device::initialize
		 */
		uint32_t initialize(const char *name, p4dev_t *deviceInfoPtr);
		
		/**
		 *  \brief Clears all rules from the table
		 *  
		 *  \return API status code:
		 *      - P4DEV_OK - table is cleared
		 *      - P4DEV_* - an error occured. Use p4dev_err_stderr() to print that error as string on stderr
		 *  
		 *  \details Sets the table to its initial state - clears all rules from 
		 *  card and ram.
		 */
		uint32_t clear();
		
		Table();
		Table(const char *name, p4dev_t *deviceInfoPtr);
		~Table();
	};
    
    typedef Table* TablePtr;
}

#endif
