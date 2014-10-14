/*  This file is part of texpp library.
    Copyright (C) 2009 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __TEXPP_BASE_BASE_H
#define __TEXPP_BASE_BASE_H

#include <texpp/common.h>

namespace texpp {

class Parser;

namespace base {

/**
 * @brief initSymbols - initialising of m_symbols by control comands and control
 *  variables. Filling m_catCodeTable lookup table for all 256 possible char
 *  values (char <-> category code)
 * @param parser - Parser object which actually contain m_symbols object and
 *  lexer (m_catCodeTable) object
 */
void initSymbols(Parser& parser);

} // namespace base
} // namespace texpp


#endif

