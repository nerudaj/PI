#ifndef RULESET_HPP_
#define RULESET_HPP_

#include <vector>

namespace p4 {
	class Table;
	typedef Table* TablePtr;
	
	class RuleSet {
	protected:
		p4dev_t *deviceInfo;
		std::vector<p4rule_t*> rules;
		uint32_t deletedRulesCnt;
		std::vector<TablePtr> tables;
	
		void cleanup();
		uint32_t writeRules();
		uint32_t invalidateRule(const uint32_t index);
		static bool isInvalidRule(p4rule_t *rule);
	
	public:
		/**
		 *  \brief Brief description
		 *  
		 *  \param [in] rule Description for rule
		 *  \param [out] index Description for index
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		uint32_t insertRule(p4rule_t *rule, uint32_t &index);
		
		/**
		 *  \brief Brief description
		 *  
		 *  \param [in] rule Description for rule
		 *  \param [out] index Description for index
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		uint32_t overwriteRule(p4rule_t *rule, uint32_t index);
		
		/**
		 *  \brief Brief description
		 *  
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		uint32_t modifyRule(uint32_t &index, const char *actionName, p4param_t *params);
		
		/**
		 *  \brief Brief description
		 *  
		 *  \param [in] index Description for index
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		uint32_t deleteRule(const uint32_t index);
		
		/**
		 *  \brief Adds a pointer to a Table object
		 *  
		 *  \param [in] tablePtr Pointer to valid table object
		 *  \return Returns P4DEV_ALLOCATE_ERROR if pointer could not be stored 
		 *  in internal memory, P4DEV_OK otherwise.
		 *  
		 *  \details Table pointers are important when rule array is modified in
		 *  such way that validity of table indices cannot be guaranteed. In that
		 *  case RuleSet object will send a signal to each table to recompute
		 *  its indices.
		 */
		uint32_t addTablePointer(TablePtr tablePtr);
		
		/**
		 *  \brief Retrieve an exact rule
		 *  
		 *  \param [in] index Index of the rule
		 *  \return Pointer to rule or NULL if index is out of bounds
		 *  
		 *  \details Index is checked against size of the rule array. If it is 
		 *  out of bounds, NULL is returned.
		 */
		p4rule_t *getRule(const uint32_t index);
		
		/**
		 *  \brief Get number of stored rules
		 */
		uint32_t getSize() const { return rules.size(); }
		
		/**
		 *  \brief Brief description
		 *  
		 *  \param [in] deviceInfoPtr Description for deviceInfoPtr
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		void initialize(p4dev_t *deviceInfoPtr);
		
		/**
		 *  \brief Brief description
		 *  
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		void deinitialize();
		
		/**
		 *  \brief Brief description
		 *  
		 *  \return Return description
		 *  
		 *  \details More details
		 */
		void clear();
		
		/**
		 *  \brief Clear table to initial state
		 *  
		 *  \param [in] indices Description for indices
		 *  \return Return description
		 *  
		 *  \details Details
		 */
		void clearTable(const std::vector<uint32_t> &indices);
		
		RuleSet();
		~RuleSet();
	};
}

#endif
