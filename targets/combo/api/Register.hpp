#ifndef REGISTER_HPP_
#define REGISTER_HPP_

namespace p4 {
	class Register {
	protected:
		p4dev_t *deviceInfo;
		p4_register_t *regInfo;
		
	public:
		/**
		 *  \brief Read data from a register
		 *  
		 *  \param [in] read_data Pointer on array where read data will be stored.
		 *          Data are stored in the little endian order.
		 *  \param [in] length Size of \p read_data array.
		 *  \param [in] reg_ind Index of target memory cell in register array.
		 *          Register cells are numbered from 0.
		 *  \return The function returns following codes
		 *      - P4DEV_OK - everything is OK and data are stored in array
		 *      - P4DEV_DEVICE_TREE_ERROR - throwed if there was any error during parsing of device tree
		 *      - P4DEV_NO_DEV - no device was passed
		 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
		 *      - P4DEV_SMALL_BUFFER - the length of the passed buffer is smaller than is required
		 *      - P4DEV_REG_INDEX_ERROR - given index is out of range
		 *      - P4DEV_NO_REG - no register structure was passed
		 */
		uint32_t read(uint8_t* read_data, const uint32_t length, uint32_t reg_ind);
		
		/**
		 *  \brief Write store value in \p data_to_write array to register \p reg on index \p reg_ind
		 *  
		 *  \param [in] data_to_write Pointer on input array of bytes to be stored in register
		 *  \param [in] length Size of \p data_to_write array in bytes
		 *  \param [in] reg_ind Index of target memory cell in register array.
		 *  \return The function returns following codes
		 *      - P4DEV_OK - everything is OK and data are stored in array
		 *      - P4DEV_DEVICE_TREE_ERROR - throwed if there was any error during parsing of device tree
		 *      - P4DEV_NO_DEV - no device was passed
		 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
		 *      - P4DEV_ERROR - the internal zero vector is too small
		 *      - P4DEV_REG_INDEX_ERROR - given index is out of range
		 *      - P4DEV_NO_REG - no register structure was passed
		 */
		uint32_t write(uint8_t* data_to_write, const uint32_t length, uint32_t reg_ind);
		
		/**
		 *  \brief Prepare instance to be used
		 *  
		 *  \param [in] deviceInfo The P4 device structure
		 *  \param [in] registerInfo Structure with register data
		 *  
		 *  \details This method is called automatically from Device object
		 */
		void initialize(p4dev_t *deviceInfo, p4_register_t *registerInfo);
	};
	
	typedef Register* RegisterPtr;
};

#endif
