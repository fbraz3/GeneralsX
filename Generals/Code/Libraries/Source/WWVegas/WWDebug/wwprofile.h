/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : WWDebug                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwdebug/wwprofile.h                          $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 6/29/01 3:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef WWPROFILE_H
#define WWPROFILE_H

#ifdef _UNIX
typedef signed long long __int64;
typedef signed long long _int64;
#endif
	
			
/*
** A node in the WWProfile Hierarchy Tree
*/
class	WWProfileHierachyNodeClass {

public:
	WWProfileHierachyNodeClass( const char * name, WWProfileHierachyNodeClass * parent );
	~WWProfileHierachyNodeClass( void );

	WWProfileHierachyNodeClass * Get_Sub_Node( const char * name );

	WWProfileHierachyNodeClass * Get_Parent( void )			{ return Parent; }
	WWProfileHierachyNodeClass * Get_Sibling( void )		{ return Sibling; }
	WWProfileHierachyNodeClass * Get_Child( void )			{ return Child; }

	void								Reset( void );
	void								Call( void );
	bool								Return( void );

	const char *					Get_Name( void )				{ return Name; }
	int								Get_Total_Calls( void )		{ return TotalCalls; }
	float								Get_Total_Time( void )		{ return TotalTime; }

protected:

	const char *					Name;
	int								TotalCalls;
	float								TotalTime;
	__int64							StartTime;
	int								RecursionCounter;

	WWProfileHierachyNodeClass *	Parent;
	WWProfileHierachyNodeClass *	Child;
	WWProfileHierachyNodeClass *	Sibling;
};

/*
** An iterator to navigate through the tree
*/
class WWProfileIterator
{
public:
	// Access all the children of the current parent
	void				First(void);
	void				Next(void);
	bool				Is_Done(void);

	void				Enter_Child( void );			// Make the current child the new parent
	void				Enter_Child( int index );	// Make the given child the new parent
	void				Enter_Parent( void );		// Make the current parent's parent the new parent

	// Access the current child
	const char *	Get_Current_Name( void )			{ return CurrentChild->Get_Name(); }
	int				Get_Current_Total_Calls( void )	{ return CurrentChild->Get_Total_Calls(); }
	float				Get_Current_Total_Time( void )	{ return CurrentChild->Get_Total_Time(); }

	// Access the current parent
	const char *	Get_Current_Parent_Name( void )			{ return CurrentParent->Get_Name(); }
	int				Get_Current_Parent_Total_Calls( void )	{ return CurrentParent->Get_Total_Calls(); }
	float				Get_Current_Parent_Total_Time( void )	{ return CurrentParent->Get_Total_Time(); }
	
protected:
	WWProfileHierachyNodeClass *	CurrentParent;
	WWProfileHierachyNodeClass *	CurrentChild;

	WWProfileIterator( WWProfileHierachyNodeClass * start );
	friend	class		WWProfileManager;
};


/*
** An iterator to walk through the tree in depth first order
*/
class WWProfileInOrderIterator
{
public:
	void				First(void);
	void				Next(void);
	bool				Is_Done(void);

	// Access the current node
	const char *	Get_Current_Name( void )			{ return CurrentNode->Get_Name(); }
	int				Get_Current_Total_Calls( void )	{ return CurrentNode->Get_Total_Calls(); }
	float				Get_Current_Total_Time( void )	{ return CurrentNode->Get_Total_Time(); }

protected:
	WWProfileHierachyNodeClass *	CurrentNode;

	WWProfileInOrderIterator( void );
	friend	class		WWProfileManager;
};


/*
** The Manager for the WWProfile system
*/
class	WWProfileManager {
public:
	static	void								Start_Profile( const char * name );
	static	void								Stop_Profile( void );

	static	void								Reset( void );
	static	void								Increment_Frame_Counter( void );
	static	int								Get_Frame_Count_Since_Reset( void )		{ return FrameCounter; }
	static	float								Get_Time_Since_Reset( void );

	static	WWProfileIterator *			Get_Iterator( void );
	static	void								Release_Iterator( WWProfileIterator * iterator );
	static	WWProfileInOrderIterator *	Get_In_Order_Iterator( void );
	static	void								Release_In_Order_Iterator( WWProfileInOrderIterator * iterator );

private:
	static	WWProfileHierachyNodeClass		Root;
	static	WWProfileHierachyNodeClass *	CurrentNode;
	static	int									FrameCounter;
	static	__int64								ResetTime;

	friend	class		WWProfileInOrderIterator;
};


/*
** WWProfileSampleClass is a simple way to profile a function's scope
** Use the WWPROFILE macro at the start of scope to time
*/
class	WWProfileSampleClass {
public:
	WWProfileSampleClass( const char * name )		
	{ 
		WWProfileManager::Start_Profile( name ); 
	}
	
	~WWProfileSampleClass( void )					
	{ 
		WWProfileManager::Stop_Profile(); 
	}
};

#ifdef WWDEBUG
#define	WWPROFILE( name )						WWProfileSampleClass _wwprofile( name )
#else
#define	WWPROFILE( name )
#endif


/*
** WWTimeIt is like WWProfile, but it doesn't save anything, it just times one routine, regardless of thread
*/
class	WWTimeItClass {
public:
	WWTimeItClass( const char * name );
	~WWTimeItClass( void );
private:
	const char * Name;
	__int64	Time;
};

#ifdef WWDEBUG
#define	WWTIMEIT( name )	WWTimeItClass _wwtimeit( name )
#else
#define	WWTIMEIT( name )
#endif



/*
** TSS 06/27/01
** WWMeasureItClass is like WWTimeItClass, but it pokes the result into the given float, 
** and can be used in the release build.
*/
class	WWMeasureItClass {
public:
	WWMeasureItClass( float * p_result );
	~WWMeasureItClass( void );

private:
	__int64	Time;
	float *  PResult;
};


#endif	// WWPROFILE_H
