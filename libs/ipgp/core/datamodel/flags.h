/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 ************************************************************************/

#ifndef ___IPGP_CORE_FLAGS_H___
#define ___IPGP_CORE_FLAGS_H___


#include <ipgp/gui/api.h>
#include <iostream>
#include <numeric>


namespace IPGP {
namespace Core {

/**
 * @class   FlagSet
 * @package IPGP::Core::DataModel
 * @brief   Enumeration flagset
 *
 * This class provides a flag interface which shall be used in combination
 * with enumerations.
 */
template<typename TEnum> class SC_IPGP_GUI_API FlagSet {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit FlagSet(const TEnum flag) :
				_flags(flag) {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Sets a flag
		 * @param flag the flag to e set up
		 * @note  Checks if flag is present before setting it up
		 */
		void set(const TEnum flag) {
			if ( !isSet(flag) )
				_flags |= flag;
		}

		/**
		 * @brief Removes a flag
		 * @param flag the flag to be removed
		 * @note  Checks if flag is set before removing it
		 */
		void remove(const TEnum flag) {
			if ( isSet(flag) )
				_flags &= ~flag;
		}

		/**
		 * @brief Toggles a flag
		 * @param flag the flag to be toggled
		 */
		void toggle(const TEnum flag) {
			_flags ^= flag;
		}

		/**
		 * @brief Unary checks if argument flag is set
		 * @param flag the flag to be tested
		 * @return true on success, false otherwise
		 */
		const bool isSet(const TEnum flag) {
			return ((_flags & flag) == flag);
		}

		inline const bool operator&=(const TEnum flag) {
			return ((_flags &= flag) == flag);
		}

		inline FlagSet& operator|=(FlagSet f) {
			_flags |= f._flags;
			return *this;
		}

		inline FlagSet& operator^=(FlagSet f) {
			_flags ^= f._flags;
			return *this;
		}

		inline FlagSet& operator^=(TEnum f) {
			_flags ^= f;
			return *this;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		int _flags;
};

} // namespace Core
} // namespace IPGP


#endif
