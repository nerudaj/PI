#ifndef DEVICE_HPP_
#define DEVICE_HPP_

#include <cstring>
#include <unordered_map>
#include "Table.hpp"

namespace p4 {
	/**
	 *  \brief Class representing abstraction over P4 device
	 *  
	 *  \details Instance of this class *must* be initialized by the proper method
	 *  prior to any other operation. Initialization will also allocate memory for
	 *  all table abstractions described in the p4 program of the device.
	 */
	class Device {
	protected:
		p4dev_t info; ///< Device info
		std::unordered_map<std::string, Table> tables; ///< Tables, indexed by their identifier in P4 program
	
	public:
		/**
		 *  \brief Initialize P4 device
		 *  
		 *  \param [in] name Name identifier of the device (P4DEV_ID0, P4DEV_ID1)
		 *  \return API status code:
		 *      - P4DEV_OK - everything is OK and device is properly allocated
		 *      - P4DEV_UNABLE_TO_ATTACH - the device cannot be attached
		 *      - P4DEV_UNABLE_TO_MAP_DEVICE_SPACE - unable to map the address space 
		 *      - P4DEV_DEVICE_TREE_NOT_VALID - passed tree is not valid
		 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
		 *      - P4DEV_NO_DEV - no \ref p4dev_t structure was passed
		 *      - P4DEV_DEVICE_TREE_READING_ERROR - reading of the device tree has failed.
		 *      - P4DEV_NO_CALLBACK - appropriate search engine functions are not implemented.
		 *  
		 *  \note This method *must* be called prior to anything else. If does not succeeed,
		 *  card is probably not available or properly configured and cannot be used.		 
		 */
		uint32_t initialize(char *name);
		
		/**
		 *  \brief Closes connection to device
		 *  
		 *  \details This method is called automatically by destructor. All memory
		 *  that has been somehow allocated (rules, tables, etc) is safely freed.
		 */
		void deinitialize();
		
		/**
		 *  \brief Reset device to initial state
		 *  
		 *  \return API status code:
		 *      - P4DEV_OK - rule is inserted
		 *      - P4DEV_NO_DEV - no device structure was passed in \p dev parameter
		 *      - P4DEV_NO_DEVICE_TREE - device tree pointer on \p dev structure is invalid
		 *      - P4DEV_DEVICE_TREE_ERROR - error during the parsing of the device tree
		 *  
		 *  \details Device is reset to its initial state, including clearing
		 *  all tables and memory held by all rules.
		 */
		uint32_t reset();
		
		/**
		 *  \brief Retrieve P4 table abstraction
		 *  
		 *  \param [in] name Table identifier in P4 program
		 *  \return If the name exists, pointer to table object is returned, otherwise
		 *  NULL is returned.
		 */
		TablePtr getTable(const char *name);
		
		/**
		 *  \brief Get array of names of tables available in device
		 *  
		 *  \param [out] names Vector array to store names to
		 *  \return API status code:
		 *      - P4DEV_OK - everything went ok
		 *      - P4DEV_ALLOCATE_ERROR - Memory for names could not be allocated
		 *  
		 *  \details If P4DEV_ALLOCATE_ERROR was returned, names will be empty.
		 */
		uint32_t getTableList(std::vector<std::string> &names);
		
		Device();
		~Device();
	};
}

#endif
