#include "IOSync.h"
#include "IOSyncBackends.h"

namespace IOSync
{

Backend::Backend()
{
	for (int classId = 0; classId < Class::NumClasses; classId++)
	{
		g_Classes[classId]->ResetRemote();
	}
}

void Class::SetIndex(int index, int localIndex)
{
	if (localIndex != -1)
	{
		int oldRemote = m_Local[localIndex].m_OtherIndex;
		if (oldRemote != -1)
			m_Remote[oldRemote].m_OtherIndex = -1;
		m_Local[localIndex].m_OtherIndex = index;
	}
	if (index != -1)
	{
		int oldLocal = m_Remote[index].m_OtherIndex;
		if (oldLocal != -1)
			m_Local[oldLocal].m_OtherIndex = -1;
		m_Remote[index].m_OtherIndex = localIndex;
	}
}

void Class::ResetRemote()
{
	for (int i = 0; i < MaxDeviceIndex; i++)
	{
		m_Remote[i] = DeviceInfo();
		m_Local[i].m_OtherIndex = -1;
		m_Local[i].m_IsConnected = false;
	}
}

void Class::OnConnected(int index, int localIndex, PWBuffer&& subtype)
{
	SetIndex(index, localIndex);
	m_Remote[index].m_Subtype = std::move(subtype);
	m_Remote[index].m_IsConnected = true;
}

void Class::OnDisconnected(int index)
{
	SetIndex(index, -1);
	m_Remote[index] = DeviceInfo();
}

void Class::DeviceInfo::DoState(PointerWrap& p)
{
	p.Do(m_OtherIndex);
	p.Do(m_IsConnected);
	p.Do(m_Subtype);
}

void Class::DoState(PointerWrap& p)
{
	for (int i = 0; i < MaxDeviceIndex; i++)
	{
		m_Local[i].DoState(p);
		m_Remote[i].DoState(p);
	}
}

void Init()
{
	if (!g_Backend)
		g_Backend.reset(new BackendLocal());
}

void PostInit()
{
	g_Backend->StartGame();
}

void DoState(PointerWrap& p)
{
	for (int c = 0; c < Class::NumClasses; c++)
	{
		g_Classes[c]->DoState(p);
	}

	g_Backend->DoState(p);
}

void Stop()
{
	g_Backend->StopGame();
}

std::unique_ptr<Backend> g_Backend;
Class* g_Classes[Class::NumClasses];

}

EXISyncClass g_EXISyncClass;
