#ifndef P4DEV_HPP_
#define P4DEV_HPP_

#include <cstdint>
#include <cstdlib>
#include <p4dev.h>
#include <p4dev_types.h>
#include <p4rule.h>

namespace p4 {
	// Following constant has to be zero until proper invalidate method is imposed
	const uint32_t DELETE_THRESHOLD_PERCENTAGE = 0; ///< In %
}

#include "RuleSet.hpp"
#include "Table.hpp"
#include "Device.hpp"

#endif
