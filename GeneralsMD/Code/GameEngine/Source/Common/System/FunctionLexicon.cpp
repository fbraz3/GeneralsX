/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: FunctionLexicon.cpp //////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/FunctionLexicon.h"
#include <cstring>
#include <typeinfo>

FunctionLexicon *TheFunctionLexicon = NULL;

static const std::type_info *FunctionLexiconTypeInfo = &typeid(FunctionLexicon);

FunctionLexicon::FunctionLexicon( void )
{
	memset( m_tables, 0, sizeof( m_tables ) );
}

FunctionLexicon::~FunctionLexicon( void )
{
	// nothing to clean up for stubbed tables
}

void FunctionLexicon::init( void )
{
	// stub platform does not register UI callbacks
}

void FunctionLexicon::reset( void )
{
	// nothing to reset in stub implementation
}

void FunctionLexicon::update( void )
{
	// no dynamic behavior needed
}

Bool FunctionLexicon::validate( void )
{
	return true;
}

void FunctionLexicon::loadTable( TableEntry *table, TableIndex tableIndex )
{
	m_tables[ tableIndex ] = table;
}

void *FunctionLexicon::keyToFunc( NameKeyType key, TableEntry *table )
{
	if ( key == NAMEKEY_INVALID )
	{
		return NULL;
	}

	for ( TableEntry *entry = table; entry && entry->name; ++entry )
	{
		if ( entry->key == key )
		{
			return entry->func;
		}
	}

	return NULL;
}

void *FunctionLexicon::findFunction( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		for ( int table = 0; table < MAX_FUNCTION_TABLES; ++table )
		{
			void *result = keyToFunc( key, m_tables[ table ] );
			if ( result )
			{
				return result;
			}
		}
		return NULL;
	}

	return keyToFunc( key, m_tables[ index ] );
}
