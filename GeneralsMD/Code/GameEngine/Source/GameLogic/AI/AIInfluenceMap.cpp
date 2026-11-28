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

// GeneralsX @feature fbraz3 27/11/2026 Per-AI tactical influence grid (adapted from CnCGeneralsZH-Reforged)

#include "PreRTS.h"

#include <cmath>
#include "Common/Xfer.h"
#include "GameLogic/AIInfluenceMap.h"

AIInfluenceMap::AIInfluenceMap() :
	m_originX( 0.0f ), m_originY( 0.0f ), m_cellSize( 1.0f ), m_cols( 0 ), m_rows( 0 ),
	m_built( FALSE ), m_builtFrame( 0 )
{
}

void AIInfluenceMap::reset( Real originX, Real originY, Real width, Real height, Real cellSize )
{
	m_originX = std::isfinite( originX ) ? originX : 0.0f;
	m_originY = std::isfinite( originY ) ? originY : 0.0f;
	if( cellSize <= 0.0f || !std::isfinite( cellSize ) )
		cellSize = 1.0f;
	m_cellSize = cellSize;
	const Real qCols = (width > 0.0f && std::isfinite( width )) ? WWMath::Div_Safe( width, m_cellSize, 0.0f ) : 0.0f;
	const Real qRows = (height > 0.0f && std::isfinite( height )) ? WWMath::Div_Safe( height, m_cellSize, 0.0f ) : 0.0f;
	m_cols = std::isfinite( qCols ) ? REAL_TO_INT_CEIL( qCols ) : 0;
	m_rows = std::isfinite( qRows ) ? REAL_TO_INT_CEIL( qRows ) : 0;
	m_enemy.assign( m_cols * m_rows, 0.0f );
	m_friend.assign( m_cols * m_rows, 0.0f );
	m_height.assign( m_cols * m_rows, 0.0f );
	m_built = FALSE;
}

void AIInfluenceMap::clear()
{
	m_enemy.assign( m_enemy.size(), 0.0f );
	m_friend.assign( m_friend.size(), 0.0f );
}

void AIInfluenceMap::cellCenter( Int col, Int row, Real *x, Real *y ) const
{
	*x = m_originX + (col + 0.5f) * m_cellSize;
	*y = m_originY + (row + 0.5f) * m_cellSize;
}

void AIInfluenceMap::setCellHeight( Int col, Int row, Real z )
{
	m_height[ row * m_cols + col ] = z;
}

Int AIInfluenceMap::cellIndexAt( Real x, Real y ) const
{
	if( m_cellSize <= 0.0f || !std::isfinite( m_cellSize ) || !std::isfinite( x ) || !std::isfinite( y ) )
		return -1;
	const Real qCol = WWMath::Div_Safe( x - m_originX, m_cellSize, -1.0f );
	const Real qRow = WWMath::Div_Safe( y - m_originY, m_cellSize, -1.0f );
	if( !std::isfinite( qCol ) || !std::isfinite( qRow ) )
		return -1;
	const Int col = REAL_TO_INT_FLOOR( qCol );
	const Int row = REAL_TO_INT_FLOOR( qRow );
	if( col < 0 || row < 0 || col >= m_cols || row >= m_rows )
		return -1;
	return row * m_cols + col;
}

void AIInfluenceMap::stamp( std::vector<Real> &layer, Real x, Real y, Real z, Real range, Real power )
{
	(void)z;
	if( m_cols <= 0 || m_rows <= 0 || m_cellSize <= 0.0f || !std::isfinite( m_cellSize ) || power <= 0.0f || range <= 0.0f )
		return;
	if( !std::isfinite( x ) || !std::isfinite( y ) || !std::isfinite( range ) )
		return;

	const Real qColLo = WWMath::Div_Safe( x - range - m_originX, m_cellSize, 0.0f );
	const Real qColHi = WWMath::Div_Safe( x + range - m_originX, m_cellSize, 0.0f );
	const Real qRowLo = WWMath::Div_Safe( y - range - m_originY, m_cellSize, 0.0f );
	const Real qRowHi = WWMath::Div_Safe( y + range - m_originY, m_cellSize, 0.0f );
	if( !std::isfinite( qColLo ) || !std::isfinite( qColHi ) || !std::isfinite( qRowLo ) || !std::isfinite( qRowHi ) )
		return;

	const Real maxCol = static_cast<Real>( m_cols - 1 );
	const Real maxRow = static_cast<Real>( m_rows - 1 );
	const Int colLo = REAL_TO_INT_FLOOR( WWMath::Clamp( qColLo, 0.0f, maxCol ) );
	const Int colHi = REAL_TO_INT_FLOOR( WWMath::Clamp( qColHi, 0.0f, maxCol ) );
	const Int rowLo = REAL_TO_INT_FLOOR( WWMath::Clamp( qRowLo, 0.0f, maxRow ) );
	const Int rowHi = REAL_TO_INT_FLOOR( WWMath::Clamp( qRowHi, 0.0f, maxRow ) );
	const Real half = m_cellSize * 0.5f;
	const Real rangeSqr = sqr( range );
	for( Int row = rowLo; row <= rowHi; ++row )
	{
		const Real cy = m_originY + (row + 0.5f) * m_cellSize;
		const Real nearestY = WWMath::Clamp( y, cy - half, cy + half );
		const Real dy = y - nearestY;
		for( Int col = colLo; col <= colHi; ++col )
		{
			const Int index = row * m_cols + col;
			const Real cx = m_originX + (col + 0.5f) * m_cellSize;
			const Real nearestX = WWMath::Clamp( x, cx - half, cx + half );
			const Real dx = x - nearestX;
			if( dx * dx + dy * dy <= rangeSqr )
				layer[ index ] += power;
		}
	}
}

void AIInfluenceMap::stampEnemy( Real x, Real y, Real z, Real range, Real power )
{
	stamp( m_enemy, x, y, z, range, power );
}

void AIInfluenceMap::stampFriend( Real x, Real y, Real z, Real range, Real power )
{
	stamp( m_friend, x, y, z, range, power );
}

Real AIInfluenceMap::enemyAt( Real x, Real y ) const
{
	const Int index = cellIndexAt( x, y );
	return (index < 0) ? 0.0f : m_enemy[ index ];
}

Real AIInfluenceMap::friendAt( Real x, Real y ) const
{
	const Int index = cellIndexAt( x, y );
	return (index < 0) ? 0.0f : m_friend[ index ];
}

void AIInfluenceMap::xfer( Xfer *xfer )
{
	xfer->xferReal( &m_originX );
	xfer->xferReal( &m_originY );
	xfer->xferReal( &m_cellSize );
	xfer->xferInt( &m_cols );
	xfer->xferInt( &m_rows );
	xfer->xferBool( &m_built );
	xfer->xferUnsignedInt( &m_builtFrame );
	const size_t cells = (size_t)m_cols * (size_t)m_rows;
	if( xfer->getXferMode() == XFER_LOAD )
	{
		m_enemy.assign( cells, 0.0f );
		m_friend.assign( cells, 0.0f );
		m_height.assign( cells, 0.0f );
	}
	for( size_t i = 0; i < cells; ++i )
	{
		xfer->xferReal( &m_enemy[ i ] );
		xfer->xferReal( &m_friend[ i ] );
		xfer->xferReal( &m_height[ i ] );
	}
}
