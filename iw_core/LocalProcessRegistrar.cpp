#include "StdAfx.h"
#include "LocalProcessRegistrar.h"
#include "Logger.h"
#include "IwBase.h"

namespace ivrworx
{



LocalProcessRegistrar *
LocalProcessRegistrar::_instance = NULL;

mutex 
LocalProcessRegistrar::_instanceMutex;


RegistrationGuard::RegistrationGuard(IN LpHandlePtr ptr, 
									 IN int process_alias):
_handleUid(ptr->GetObjectUid()),
_aliasId(process_alias)
{
	LocalProcessRegistrar::Instance().RegisterChannel(_handleUid,ptr);
	LocalProcessRegistrar::Instance().RegisterChannel(_aliasId,ptr);
	
}



RegistrationGuard::~RegistrationGuard()
{
	LocalProcessRegistrar::Instance().UnregisterChannel(_handleUid);
	LocalProcessRegistrar::Instance().UnregisterChannel(_aliasId);
}


LocalProcessRegistrar::LocalProcessRegistrar(void)
{
}

LocalProcessRegistrar::~LocalProcessRegistrar(void)
{
}

LocalProcessRegistrar &
LocalProcessRegistrar::Instance()
{
	mutex::scoped_lock lock(_instanceMutex);

	if (_instance == NULL)
	{
		_instance = new LocalProcessRegistrar();
	}

	return *_instance;

}

void
LocalProcessRegistrar::RegisterChannel(IN int handle_id, IN LpHandlePtr ptr)
{
	
	if (handle_id == IW_UNDEFINED)
	{
		return;
	}
	
	mutex::scoped_lock lock(_mutex);
	if ( _locProcessesMap.find(handle_id) != _locProcessesMap.end())
	{
		throw;
	}

	_locProcessesMap[handle_id] = ptr;
	
	LogDebug("Mapped " << handle_id << " to (" << ptr.get() << ")");
}

void
LocalProcessRegistrar::UnregisterChannel(int handle_id)
{
	if (handle_id == IW_UNDEFINED)
	{
		return;
	}
	
	mutex::scoped_lock lock(_mutex);
	if (_locProcessesMap.find(handle_id) == _locProcessesMap.end())
	{
		return;
	}

	_locProcessesMap.erase(handle_id);


	ListenersMap::iterator iter = _listenersMap.find(handle_id);
	if (iter != _listenersMap.end())
	{
		HandlesList &list = (*iter).second;
		for (HandlesList::iterator set_iter = list.begin(); 
			set_iter != list.end(); 
			set_iter++)
		{
			(*set_iter)->Send(new MsgShutdownEvt(handle_id));
		}

		list.clear();

		_listenersMap.erase(iter);
			
	}

	LogDebug("Unregistered handle " << handle_id);
}


LpHandlePtr 
LocalProcessRegistrar::GetHandle(int procId)
{
	return GetHandle(procId,"");

}

void
LocalProcessRegistrar::AddShutdownListener(IN int procId, IN LpHandlePtr channel)
{
	
	mutex::scoped_lock lock(_mutex);

	//
	// Create new list if needed.
	//
	if (_listenersMap.find(procId) == _listenersMap.end())
	{
		HandlesList list; 
		_listenersMap[procId] = list;
	}

	_listenersMap[procId].push_back(channel);

}

LpHandlePtr
LocalProcessRegistrar::GetHandle(int procId,const string &qpath)
{
	//
	// Scope to refrain taking nested locks
	// logs & collection
	//
	{
		mutex::scoped_lock lock(_mutex);

		//
		// user specified no destination queue 
		// and pid is not well known process
		// => if the process found in registrar send 
		// it locally
		//
		LocalProcessesMap::iterator i = 
			_locProcessesMap.find(procId);

		if (i != _locProcessesMap.end())
		{
			return (*i).second;
		}

		
		return IW_NULL_HANDLE;
	}

}

void 
AddShutdownListener( IN LpHandlePair observable_pair, 
					 IN LpHandlePtr listener_handle)
{
	LocalProcessRegistrar::Instance().AddShutdownListener(
		observable_pair.inbound->GetObjectUid(), 
		listener_handle);

}

LpHandlePtr 
GetHandle(IN int handle_id)
{
	return LocalProcessRegistrar::Instance().GetHandle(handle_id);

}


}