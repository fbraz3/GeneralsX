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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DProjectedShadow.h ///////////////////////////////////////////////////////////
//
// Real time shadow projection through textures.
//
// Author: Mark Wilczynski, February 2002
//
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __W3D_PROJECTED_SHADOW_H_
#define __W3D_PROJECTED_SHADOW_H_

#include "GameClient/Shadow.h"

class W3DShadowTexture;	//forward reference
class W3DShadowTextureManager;	//forward reference
class W3DProjectedShadow; //forward reference
class Drawable;	//forward reference

class W3DProjectedShadowManager	: public ProjectedShadowManager
{
/*	enum ShadowTextureType {
		STT_STATIC	=	0x0001,
		STT_DYNAMIC	= 0x0002,
		STT_SHARED
			};
*/
	public:
		W3DProjectedShadowManager( void );
		~W3DProjectedShadowManager( void );
		Bool init(void);					///<allocate one-time shadow assets for length of entire game.
		void reset(void);					///<free all existing shadows - ready for next map.
		void shutdown(void);			///<free all assets prior to shutdown of entire game.
		Int	 renderShadows(RenderInfoClass & rinfo);	///<iterate over each object and render its shadow onto affected objects.
		void ReleaseResources(void);	///<release device dependent D3D resources.
		Bool ReAcquireResources(void);	///<allocate device dependent D3D resources.
		void invalidateCachedLightPositions(void);	///<forces shadows to update regardless of last lightposition

		virtual Shadow	*addDecal(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadowInfo);	///<add a non-shadow decal
		virtual Shadow	*addDecal(Shadow::ShadowTypeInfo *shadowInfo);	///<add a non-shadow decal which does not follow an object.
		W3DProjectedShadow	*addShadow( RenderObjClass *robj, Shadow::ShadowTypeInfo *shadowInfo, Drawable *draw);	///<add a new shadow with texture of given name or that of robj.
		void removeShadow (W3DProjectedShadow *shadow);
		void removeAllShadows(void); ///< Remove all shadows.
		TextureClass *getRenderTarget(void)	{ return m_dynamicRenderTarget;}
		SpecialRenderInfoClass *getRenderContext(void)	{ return m_shadowContext;}
		void updateRenderTargetTextures(void);	///<render into any textures that need updating.
		void queueDecal(W3DProjectedShadow *shadow);	///<add shadow decal to render list - decal conforms to terrain.
		void queueSimpleDecal(W3DProjectedShadow *shadow);	///< add shadow decal to render list - decal floats on terrain.
		void flushDecals(W3DShadowTexture *texture, ShadowType type);	///<empty queue by rendering all decals with given texture

	private:
		W3DProjectedShadow *m_shadowList;
		W3DProjectedShadow *m_decalList;
		TextureClass	*m_dynamicRenderTarget;	///<offscreen video memory texture used to render all shadow textures.
		Bool m_renderTargetHasAlpha;					///<does render target have destination alpha support?
		CameraClass		*m_shadowCamera;					///<camera used to render all shadow textures - configured by projector
		LightEnvironmentClass m_shadowLightEnv;
		SpecialRenderInfoClass *m_shadowContext;
		W3DShadowTextureManager *m_W3DShadowTextureManager;
		Int m_numDecalShadows;							///< number of decal shadows in the system.
		Int m_numProjectionShadows;						///< number of projected shadows in the system.
		Int renderProjectedTerrainShadow(W3DProjectedShadow *shadow, AABoxClass &box);	///<render shadow on map terrain.
};

extern W3DProjectedShadowManager *TheW3DProjectedShadowManager;

#endif	//__W3D_PROJECTED_SHADOW_H_
