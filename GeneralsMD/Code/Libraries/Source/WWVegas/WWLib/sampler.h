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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Sampler                                                      *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwlib/sampler.h                                   $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// 'function selected for automatic inline expansion'.  Cool, but since we're treating
// warnings as errors, don't warn me about this!
#pragma warning(disable : 4711)

// 'not inlined' -- the evil twin of the above warning.
#pragma warning(disable : 4710)

#ifndef SAMPLER_H
#define SAMPLER_H

class Random4Class;

// This module contains sampling techniques that sample over multidimensional space
// Dimension is the length of the vector to sample
// Divisions is used to determine the number of strata to sample over
// In some samplers, e.g. regular grid, it will repeat after dimension*division calls
// to sample. A call to sample increments the internal state of the sampler.
// Hector Yee 6/11/01

// Abstract base class
// all these sampling algoriths modify a vector of length Dimensions
// they all return a value between 0..1
// Hector Yee 6/11/01
class SamplingClass
{
public:
	SamplingClass(unsigned int dimensions, unsigned char divisions):
		Dimensions(dimensions),
		Divisions(divisions)
		{};	
	virtual void Reset() {};
	virtual void Sample(float *target)=0;
	virtual ~SamplingClass() {};
protected:
	unsigned int Dimensions;
	unsigned char Divisions;
};

// Samples randomly in the dimensions using Mesenne Twister
// divisions ignored
// Hector Yee 6/11/01
class RandomSamplingClass : public SamplingClass
{
public:
	RandomSamplingClass(unsigned int dimensions, unsigned char divisions=0);
	virtual void Reset() {};
	virtual void Sample(float *target);	
};

// samples over a regular hypergrid
// Hector Yee 6/11/01
class RegularSamplingClass : public SamplingClass
{
public:
	RegularSamplingClass(unsigned int dimensions, unsigned char divisions=3);
	virtual void Reset();
	virtual void Sample(float *target);
	virtual ~RegularSamplingClass();
protected:
	unsigned char *index;
};

// samples over a regular hypergrid with random perturbations
// Hector Yee 6/11/01
class StratifiedSamplingClass : public SamplingClass
{
public:
	StratifiedSamplingClass(unsigned int dimensions, unsigned char divisions=3);
	virtual void Reset();
	virtual void Sample(float *target);
	virtual ~StratifiedSamplingClass();
protected:
	unsigned char *index;
};

// samples using QuasiMonteCarlo
// divisions ignored
// based on the Halton-Hammersly sequence which is in turn based on the inverse radical function
// Hector Yee 6/11/01
class QMCSamplingClass : public SamplingClass
{
public:
	QMCSamplingClass(unsigned int dimensions, unsigned char divisions=0);
	virtual void Reset() {index=0;};
	virtual void Sample(float *target);
	void Set_Offset(unsigned int offset) { index=offset; }
protected:
	unsigned int index;
};


#endif