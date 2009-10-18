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

#include "StdAfx.h"
#include "BridgeMacros.h"
#include "CallBridge.h"

namespace ivrworx
{
	const char CallBridge::className[] = "CallBridge";
	Luna<CallBridge>::RegType CallBridge::methods[] = {
		method(CallBridge, answer),
		method(CallBridge, speak),
		method(CallBridge, cleandtmfbuffer),
		method(CallBridge, waitfordtmf),
		{0,0}
	};

	CallBridge::CallBridge(lua_State *L):
	_call(NULL)
	{
		// should never be called directly needed
		// only to compile with luna
		throw;
	}

	CallBridge::CallBridge(CallWithRtpManagement *call):
	_call(call)
	{
		
	}

	CallBridge::~CallBridge(void)
	{
	}

	int
	CallBridge::cleandtmfbuffer(lua_State *L)
	{
		FUNCTRACKER;
		
		LogDebug("CallBridge::cleandtmfbuffer iwh:" << _call->StackCallHandle());

		_call->CleanDtmfBuffer();

		lua_pushnumber (L, API_SUCCESS);
		return 1;

	}

	int
	CallBridge::waitfordtmf(lua_State *L)
	{
		FUNCTRACKER;

		
		LUA_INT_PARAM(L,timeout,-1);
		
		LogDebug("CallBridge::cleandtmfbuffer iwh:" << _call->StackCallHandle() << ", timeout:" << timeout);

		string signal;
		ApiErrorCode res = 
			_call->WaitForDtmf(signal, MilliSeconds(timeout));

		lua_pushnumber (L, res);
		if (IW_SUCCESS(res))
		{
			lua_pushstring(L, signal.c_str());
		} 
		else
		{
			lua_pushnil(L);
		}

		return 1;

	}

	int
	CallBridge::speak(lua_State *L)
	{
		FUNCTRACKER;

		LUA_BOOL_PARAM(L,sync,-1);
		LUA_STRING_PARAM(L,mrcp_string,-2);

		LogDebug("CallBridge::speak  iwh:" << _call->StackCallHandle() << ", speak:" << mrcp_string << ", sync:" << sync);

		ApiErrorCode res =_call->Speak(mrcp_string,sync);

		lua_pushnumber (L, res);
		return 1;

	}

	int
	CallBridge::answer(lua_State *state)
	{
		FUNCTRACKER;

		LogDebug("CallBridge::answer  iwh:" << _call->StackCallHandle());

		ApiErrorCode res = _call->AcceptInitialOffer();
		lua_pushnumber (state, res);

		return 1;
	}


}


