/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once
#include "Luna.h"
#include "LuaObject.h"

namespace ivrworx
{
	class rtpproxy: 
		public luaobject
	{
	public:
		rtpproxy(lua_State *L);
		rtpproxy(RtpProxySessionPtr rtpProxySession);

		virtual ~rtpproxy();
		
		// Lua interface
		int allocate(lua_State *L);
		int teardown(lua_State *L);
		int modify(lua_State *L);
		int bridge(lua_State *L);
		int localoffer(lua_State *L);
		int remoteoffer(lua_State *L);

		int waitfordtmf(lua_State *L);
		int cleandtmfbuffer(lua_State *L);
		int dtmfbuffer(lua_State *L);

		ActiveObjectPtr get_active_object();
		

		static const char className[];
		static Luna<rtpproxy>::RegType methods[];

		private:

		RtpProxySessionPtr _rtpProxySession;

	

	};


}

