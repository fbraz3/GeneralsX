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

#pragma once

#ifndef _AI_INFLUENCE_MAP_H_
#define _AI_INFLUENCE_MAP_H_

#include <vector>

class Xfer;

class AIInfluenceMap
{
public:
	AIInfluenceMap();

	void reset( Real originX, Real originY, Real width, Real height, Real cellSize );
	Bool isSized() const { return m_cols > 0; }
	void clear();

	Int getCols() const { return m_cols; }
	Int getRows() const { return m_rows; }
	void cellCenter( Int col, Int row, Real *x, Real *y ) const;
	void setCellHeight( Int col, Int row, Real z );

	void stampEnemy( Real x, Real y, Real z, Real range, Real power );
	void stampFriend( Real x, Real y, Real z, Real range, Real power );

	Real enemyAt( Real x, Real y ) const;
	Real friendAt( Real x, Real y ) const;

	void xfer( Xfer *xfer );

	void markBuilt( UnsignedInt frame ) { m_builtFrame = frame; m_built = TRUE; }
	Bool isBuilt() const { return m_built; }
	UnsignedInt getBuiltFrame() const { return m_builtFrame; }

private:
	void stamp( std::vector<Real> &layer, Real x, Real y, Real z, Real range, Real power );
	Int cellIndexAt( Real x, Real y ) const;

	Real m_originX, m_originY, m_cellSize;
	Int m_cols, m_rows;
	std::vector<Real> m_enemy;
	std::vector<Real> m_friend;
	std::vector<Real> m_height;
	Bool m_built;
	UnsignedInt m_builtFrame;
};

#endif
