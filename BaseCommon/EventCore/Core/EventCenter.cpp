//-------------------------------------------------------------------------
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : tEventCenter.cpp
//  @ Date : 2011-5-18
//  @ Author : ���ĸ�
//	�¼�����ģ��
//-------------------------------------------------------------------------


#include "EventCenter.h"
#include "Event.h"
#include "EventFactory.h"
#include "TableTool.h"
#include "FixedTimeManager.h"
#include "TableManager.h"

#include <stdarg.h>

#include "CEvent.h"

#include "ResponseEvent.h"

#include "AutoObjectFieldInfo.h"
#include "CEvent.h"
#include "ClientEvent.h"
//-------------------------------------------------------------------------
namespace FieldInfoName
{
	extern const char eventName[]			= TYPE_EVENT_NAME;
}
//-------------------------------------------------------------------------
//�ػ��¼������ֶεĻ�ȡ�ַ�Ϊ�¼������Ƶ�����Ϣ
template<>
bool AutoObjFieldInfo<AutoEvent, FIELD_EVENT>::get(void *data, AString &resultString) const
{
	AutoEvent temp;
	if (get(data, &temp, typeid(AutoEvent)) && temp)
		resultString = temp->GetEventNameInfo();
	else
		resultString = "NULL Event";
	return true;
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
namespace Logic
{
	//-------------------------------------------------------------------------
	class _UpdateEvent : public CEvent
	{
		friend class tEventCenter;

	public:
		virtual bool Update(float onceTime)
		{
			return mUpdateCall(this);
		}

		virtual void InitData() override
		{
			mUpdateCall.cleanup();
		}

	protected:
		EventCallBack		mUpdateCall;
	};
	//-------------------------------------------------------------------------
	tEventCenter::tEventCenter()
		: mEnvironmentID(-1)
		, mbNeedGenerateIndex(false)
		, mbNowProcessing(false)
		, mbCheckOptimizeSpeed(false)
	{
		mMsgIndexData.resize(1024);
		mMsgIndexData.write((ushort)0);
		mArrayIndex.push_back(AutoEventFactory());

		mShareListPool = MEM_NEW NodePool();
		mShareListPool->setBlockSize(sizeof(PoolList<AutoEvent>::ListNode));
		//mTempRunEventList.ReadyNodePool(mShareListPool);
		//mEventList.ReadyNodePool(mShareListPool);

		mFixedTimeManager = new FixedTimeManager();

		mResponseEventFactory = MEM_NEW ResponseEventFactory();
		RegisterEvent("ResponsionC2S", mResponseEventFactory);
		RegisterEvent("_UpdateEvent", MEM_NEW EventFactory<_UpdateEvent>());

		//!!! ����֧�ֱ����¼����ͣ��Ժ���Ҫ����֧�ֶ��� ע�ᱣ���¼���������
		//if (!FieldInfoManager::GetMe().getFieldInfoFactory(FIELD_EVENT))
		//{
		//	FieldInfoManager::GetMe().addFieldInfoFactory(MEM_NEW AutoObjFieldInfoFactory<AutoEvent, FIELD_EVENT, FieldInfoName::eventName>);
		//	NiceDataField::getMe().appendDataFieldOnTypeInfo<AutoEvent>(FIELD_EVENT);
		//}

	}
 
	tEventCenter::~tEventCenter()
	{
		if (!mEventList.empty())
		{
			for (size_t i=0; i<mEventList.size(); ++i)
			{
				AutoEvent &hEvt = mEventList[i];
				if ( hEvt )
				{
					if (hEvt->needFree())
						hEvt._free();
				}
			}
			mEventList.clear(true);
		}

		// �ͷ�˳�����Ҫ����Ȼ�¼��ͷŵ� mFreeEventList �ò��� delete
		mFixedTimeManager->ClearAllEvent();
		mEventList.clear();
		mTempRunEventList.clear();
#if USE_SAFT_RELEASE_EVENT
		for (size_t i=0; i<mFreeEventList.size(); ++i)
		{
			delete mFreeEventList[i];
		}
		mFreeEventList.clear(false);
#endif
		RemoveAllFactory();
		delete mFixedTimeManager;		
		mFixedTimeManager = NULL;
#if USE_SAFT_RELEASE_EVENT
		// ��Ҫִ�����Σ����Ƴ�����ʱ�������ڵ��¼��ͷ��п����ٴ�ת�Ƶ����ĵ��ͷ��б���
		for (size_t i=0; i<mFreeEventList.size(); ++i)
		{
			delete mFreeEventList[i];
		}
		mFreeEventList.clear(true);
#endif
		mShareListPool._free();
	}

	// �յĹ���, ������ʱ����ID
	class EmptyFactroy : public CEventFactory
	{
	public:
		EmptyFactroy()
		{
			mEventName = "Empty";
		}

		virtual AutoEvent NewEvent()
		{
			//ERROR_LOG("ʹ����δע����¼� [%d]", GetNameIndex());
			
			return AutoEvent();
		}

		virtual AutoEvent StartEvent(void) override
		{
			ERROR_LOG("ʹ����δע����¼� [%d] [%s]", GetNameIndex(), GetEventName());
			return AutoEvent();
		}
	};

	void tEventCenter::UpdateNetMsgIndex(AutoData msgIndexData)
	{
		AutoNet net = _GetNetTool(0);
		if (net && !net->NeedUpdateMsgIndex())
		{
			ERROR_LOG("��ǰ����Ӧ��Ϊ��������ʹ�ã������Ա�����");
			return;
		}
		msgIndexData->seek(0);

		ushort num = 0;
		if (!msgIndexData->read(num))
			return;

		ushort id = 0;
		int index = 0;
		for (int i=0; i<num; ++i)
		{
			if (!msgIndexData->read(id))
				return;

			if (!msgIndexData->read(index))
				return;

			//AString name;
			//if (!msgIndexData->readString(name))
			//	return;
			if (id>0)
			{
				auto factory = mFactoryArray.find(index);
				if (factory)
				{
					if (factory->mID!=0 && factory->mID!=id)
					{
						WARN_LOG("[%s] Event factory �ظ������˲�ͬ��ID����", factory->GetEventName());
					}
					//else
					//	GREEN_LOG("[%s] ID = %d", it->second->GetEventName(), id);
					factory->mID = id;

					if (id>=mArrayIndex.size())
						mArrayIndex.resize(id+8);
					mArrayIndex[id] = factory;

				}
				else
				{
					//ERROR_LOG("[%s] Event factory no register", name.c_str());
					AutoEventFactory f = MEM_NEW EmptyFactroy();
					f->mEventCenter = this;
					f->mNameIndex = index;
					f->mID = id;

					mFactoryArray[index] = f;

					if (id>=mArrayIndex.size())
						mArrayIndex.resize(id+8);
					mArrayIndex[id] = f;
				}
			}
		}
		
	}

	bool tEventCenter::GenerateMsgIndex(AutoData &destData, int msgNameIndex /*= 0*/)
	{
		AutoNet net = _GetNetTool(0);
		if (!destData)
			destData = MEM_NEW DataBuffer(1024);
		else
		{
			destData->seek(0);
			destData->setDataSize(0);
		}
		if (msgNameIndex==0)
		{
			if (mbNeedGenerateIndex)
			{				
				mMsgIndexData.clear(false);
				mMsgIndexData.seek(2);
				int i=0;
				for (auto it=mFactoryArray.begin(); it; ++it)
				{
					AutoEventFactory evtFact = it.get();
					if (evtFact->mID>0 && (!net || net->NeedMsgEventIndex(evtFact->GetEventName(), evtFact->GetNameIndex())) )
					{
						++i;
						mMsgIndexData.write((ushort)evtFact->mID);
						mMsgIndexData.write((int)evtFact->GetNameIndex());
#if DEVELOP_MODE
						//destData->writeString(it->second->GetEventName());
						GREEN_LOG("Msg id [%d] = [%d] >[%s]", evtFact->mID, evtFact->GetNameIndex(), evtFact->GetEventName());
#endif
					}
				}
				mMsgIndexData.seek(0);
				mMsgIndexData.write((ushort)i);

				mbNeedGenerateIndex = false;
			}
			//destData->seek(0);
			destData->_write(mMsgIndexData.data(), mMsgIndexData.dataSize());

			return true;
		}
		else
		{
			auto factory=mFactoryArray.find(msgNameIndex);
			if (factory)
			{				
				if (factory->mID>0 && (!net || net->NeedMsgEventIndex(factory->GetEventName(), factory->GetNameIndex())) )
				{				
					destData->write((ushort)1);
					destData->write((ushort)factory->mID);
					destData->write((int)factory->GetNameIndex());
					//destData->writeString(it->second->GetEventName());
					return true;
				}
			}
		}
		destData->write((ushort)0);
		return false;
	}

	AutoEvent tEventCenter::StartEvent( const AString &strEventName )
	{
		return StartEvent(strEventName.c_str());
		
	}

	AutoEvent tEventCenter::StartEvent( const char *szEventName )
	{
		if (szEventName==NULL)
			return AutoEvent();

		AutoEventFactory hF = mFactoryMap.find(szEventName);
		if (hF)
		{
			return hF->StartEvent();
		}

		ERROR_LOG("Error: no exist event factory [%s]", szEventName);
		return AutoEvent();
	}

	AutoEvent tEventCenter::StartEvent(int nEventNameIndex )
	{
		AutoEventFactory hF = mFactoryArray.find(nEventNameIndex);				
		if (hF)
		{
			return hF->StartEvent();
		}

		ERROR_LOG("Error: no exist event factory [%d]", nEventNameIndex);
		return AutoEvent();
	}


	AutoEvent tEventCenter::StartEvent( const char* strEventName, bool bWarn )
	{
		AutoEventFactory hF = mFactoryMap.find(strEventName);
		if (hF)
		{
			AutoEvent evt = hF->StartEvent();
			evt->SetFactory(hF);
			//GREEN_LOG("*** Start event >[%s]", evt->GetEventNameInfo().c_str());
			return evt;
		}

		if (bWarn)
			WARN_LOG("Error: no exist event factory [%s]", strEventName);

		return AutoEvent();
	}

	void tEventCenter::ProcessEvent()
	{
		// ʹ�� mbNowProcessing ��֤�����ݹ����
		if (mbNowProcessing)
			return;

		mbNowProcessing = true;

		if (mbCheckOptimizeSpeed)
		{
			// �Ż�: NOTE: ע�������¼���, �Ż������ƽ��в����¼�����
			mFactoryMap.OptimizeSpeed(10.5f);
			mFactoryArray.OptimizeSpeed(10.5f);
			mbCheckOptimizeSpeed = false;

			//mFactoryMap.Dump();
			//mFactoryArray.Dump();
		}

		if (!mEventList.empty())
		{
		//	for (EventList::iterator it=mEventList.begin(); it!=mEventList.end(); ++it)
		//	{
		//		mTempRunEventList.push_front(*it);
		//	}
		//	mEventList.clear();
		//}

		//if (!mTempRunEventList.empty())
		//{
			mTempRunEventList.swap(mEventList);
			// run easylist raversal, then do ProcessEventList::onRaversal
			//for (EventList::iterator it=mTempRunEventList.begin(); it!=mTempRunEventList.end(); ++it)
			//{
			//	AutoEvent hEvt = *it;
			for (size_t i=0; i<mTempRunEventList.size(); ++i)
			{
				AutoEvent &hEvt = mTempRunEventList[i];
				if ( hEvt )
				{
					if (hEvt->needFree())
						hEvt._free();
					else if ( !hEvt->getFinished() )
					{
						try{
							hEvt->DoEvent(true);
							//if ((*it)->_AutoFinish())
							//	(*it)->Finish();
						}
						catch (...)
						{
							ERROR_LOG("[%s] ִ���쳣", hEvt->GetEventName());
						}
					}
					hEvt.setNull();
				}
			}
			mTempRunEventList._setEmpty();
		}
		//���¹�������
		//for (size_t i=0; i<mFactoryMap.size(); ++i)
		//{
		//	mFactoryMap.get(i)->Process(NULL);
		//}

		if (mResponseEventFactory)
			mResponseEventFactory->Process(NULL);

		try{
			mFixedTimeManager->Process();
		}
		catch (std::exception &e)
		{
			ERROR_LOG("Time event process error >%s", e.what());
		}
		catch (...)
		{
			ERROR_LOG("Time event process error");
		}
#if USE_SAFT_RELEASE_EVENT
		mTempFreeEventList.swap(mFreeEventList);
		for (size_t i=0; i<mTempFreeEventList.size(); ++i)
		{
			tEvent* &freeEvt = mTempFreeEventList[i];
			if (!freeEvt->mSelf.is())
			{
#if DEVELOP_MODE
				AssertEx(!freeEvt->mbNowFree, "���ش���[%s]�¼���ǰ��Ӧ��Ϊ�ͷ�״̬", freeEvt->GetEventName());
#endif
				Hand<tEventFactory> f = freeEvt->GetEventFactory();
				if (f)
					f->FreeEvent(freeEvt);
				else
					delete freeEvt;
			}
			else
				WARN_LOG("Happy event release then repeated use >%s", freeEvt->GetEventName());
			freeEvt = NULL;
		}
		mTempFreeEventList._setEmpty();
#endif
		
		mbNowProcessing = false;
	}

	void tEventCenter::Waiting(AutoEvent event)
	{
		mEventList.push_back(event);
	}

	void tEventCenter::RegisterEvent(const AString &strEventName, AutoEventFactory hFactory)
	{
		RegisterEvent(strEventName.c_str(), hFactory);
	}

	void tEventCenter::RegisterEvent( const char* strEventName, AutoEventFactory hFactory )
	{
		mbCheckOptimizeSpeed = true;
		//int key = MAKE_INDEX_ID(strEventName);
		int id = 0;

		KEY indexID = MAKE_INDEX_ID(strEventName);

		auto existFactory = mFactoryArray.find(indexID);		
		if (existFactory)
		{
            if (existFactory==hFactory)
            {
                ERROR_LOG("%s already register", strEventName);
                return;
            }
			id = existFactory->mID;
		}
		if (RemoveFactory(strEventName))
		{
			WARN_LOG("Warning: [%s] repeat Register event, already exist, now replace to new.", strEventName);			
		}		

		hFactory->SetEventName(strEventName);
		hFactory->mEventCenter = this;
		hFactory->ReaydyListNodePool(mShareListPool);

		if (mFactoryMap.size()>0xFFFE*0.5f)
		{
			ERROR_LOG("������С���� [%d]", (int)0xFFFE*0.5f);
			return;
		}
		mFactoryMap.insert(strEventName, hFactory);
		TABLE_LOG("Register event %d >%s", hFactory->GetNameIndex(), strEventName);
		mFactoryArray[indexID] = hFactory;
		hFactory->Initialize();

		AutoNet net = _GetNetTool(0);
		if (net)
		{
			if (net->NeedUpdateMsgIndex()) 
			{
				// ��Ϊ�ͻ���ʱ
				if (id>0)
				{
					if (id>=mArrayIndex.size())
						mArrayIndex.resize(id+1);

					hFactory->mID = id;
					mArrayIndex[id] = hFactory;
				}
			}
			else
			{
				// ��Ϊ������ʱ
				if (net->NeedMsgEventIndex(strEventName, indexID))
				{
					if (id>0 && id<mArrayIndex.size())
					{
						mArrayIndex[id] = hFactory;
						hFactory->mID = id;
					}
					else
					{
						id = mArrayIndex.size();
						mArrayIndex.push_back(hFactory);
						hFactory->mID = id;
					}
					if (id>0)
						net->OnMsgRegistered(indexID);

					mbNeedGenerateIndex = true;
				}
			}
		}
		//Hand<tClientEvent> evt = StartEvent(strEventName);
		//if (evt)
		//{
		//	WARN_LOG("*** Request>[%s]", evt->GetEventName());
		//}
	}

	void tEventCenter::_NodifyEventFinish( AutoEvent hEvent )
	{		
		mFixedTimeManager->RemoveEvent(hEvent);

		//for (size_t i=0; i<mFactoryMap.size(); ++i)
		//{
		//	if (mFactoryMap[i]->_HasRelation(hEvent))
		//	{
		//		mFactoryMap[i]->_OnEvent(hEvent);
		//		//it->second->_NodifyEventFinish(hEvent);
		//	}
		//}

#if SHOW_EVENT_TICK
		auto f = mFactoryArray.find(hEvent->GetNameIndex());
		if (f)
		{
			f->_OnEvent(hEvent);
		}		
#endif
	}

	bool tEventCenter::RemoveFactory(const char* eventName)
	{
		size_t size = 0;
		//int key = MAKE_INDEX_ID(eventName);
		AutoEventFactory f = mFactoryMap.find(eventName);
		if ( f && mFactoryMap.erase(eventName) )
		{
			f->OnRemove();
			mFactoryArray.erase(f->GetNameIndex());
			// NOTE: ���ﲻ��ǿ���ͷŴ˹�����ע��˹������Զ��ͷŻ���
			f._free();
			INFO_LOG("Succeed remove factory [%s]", eventName);

			mbNeedGenerateIndex = true;

			return true;
		}
		else
			mFactoryArray.erase(MAKE_INDEX_ID(eventName));

		return false;
	}
	
	void tEventCenter::RemoveAllFactory(void)
	{
		mFixedTimeManager->ClearAllEvent();
		for (auto it= mFactoryMap.begin(); it.have(); it.next())
		{
			AutoEventFactory hF = it.get();
			AutoEvent hT = hF->GetTemplate();
			hT._free();
			hF._free();
		}
		mFactoryMap.clear(true);
		mFactoryArray.clear();
	}


	Logic::AutoEventFactory tEventCenter::_GetEventFactory( const char* eventName )
	{
		AutoEventFactory hF = mFactoryMap.find(eventName);
		if (!hF)
		{
			ERROR_LOG("Error: no exist event factory [%s]", eventName);
		}
		return hF;
	}

	//Logic::AutoEventFactory tEventCenter::_GetEventFactory( int nameIndexID )
	//{
	//	auto it = mFactoryMap.find(nameIndexID);
	//	if (it!=mFactoryMap.end())
	//		return it->second;
	//	return AutoEventFactory();
	//	//return mFactoryMap.find(nameIndexID);
	//}

	void tEventCenter::SetTrigger( const char* triggerEventName, const char* actionEventName )
	{
		AutoEventFactory hFact = _GetEventFactory(triggerEventName);
		if (hFact)
			hFact->AppendTriggerEventName(actionEventName);
	}

	void tEventCenter::RemoveTrigger( const char* triggerEventName, const char* actionEventName )
	{
		AutoEventFactory hFact = _GetEventFactory(triggerEventName);
		if (hFact)
			hFact->RemoveTriggerEventName(actionEventName);
	}

	bool tEventCenter::SerializeEvent(AutoEvent &hEvent, DataStream *destData)
	{
		//AutoData hData = MEM_NEW DataBuffer(64);
#if SERIALIZE_EVENT_NAME
		hEvent->GetEventFactory()->SaveEventName(destData);
#else
		hEvent->GetEventFactory()->SaveEventNameIndex(destData);
#endif		
		if (hEvent->_SaveDataType(destData) && hEvent->_Serialize(destData))
			return true;

		ERROR_LOG("get event [%s] data stream error", hEvent->GetEventNameInfo().c_str());
		return false;
	}

	AutoEvent tEventCenter::RestoreEvent(DataStream *scrData)
	{
#if SERIALIZE_EVENT_NAME
		AutoEvent hEvent;
		AString eventName;
		if (!scrData->readString(eventName))
			return false;

		hEvent = StartEvent(eventName);

		if (hEvent)
		{
			if (hEvent->_RestoreDataType(scrData)==NULL_NICEDATA)
			{
				ERROR_LOG("[%s]�ָ��¼���������Ϊ�� NULL_NICEDATA", hEvent->GetEventName());
				return AutoEvent();
			}

			if (hEvent->_Restore(scrData))
			{				
				return hEvent;
			}
			ERROR_LOG("Error: Fail restor event form data stream, may be is same event type, example default net event use ClientEvent receive", eventName.c_str());	
		}
		else
			ERROR_LOG("Error: restor event form data stream, [%s] no exist", eventName.c_str());				
#else
		int eventTypeIndex = 0;
		if (scrData->read(eventTypeIndex))
		{			
			AutoEvent hEvent = StartEvent(eventTypeIndex);
			if (hEvent)
			{
				if (hEvent->_RestoreDataType(scrData)==NULL_NICEDATA)
				{
					ERROR_LOG("[%s]�ָ��¼���������Ϊ�� NULL_NICEDATA", hEvent->GetEventName());
					return AutoEvent();
				}

				if (hEvent->_Restore(scrData))
					return hEvent;

				ERROR_LOG("Error: restor event [%s] form data stream", hEvent->GetEventName());
			}
			else
				ERROR_LOG("Error: restor event form data stream, [%d] no exist", eventTypeIndex);
		}
		else
			ERROR_LOG("Error: restor event form data stream");
#endif
		return AutoEvent();
	}

	bool tEventCenter::_ExistEventFactory( const char* eventName )
	{
		return mFactoryMap.exist(eventName);
	}

	void tEventCenter::Dump( void )
	{
		INFO_LOG("Begin show event center infomation ...");
		INFO_LOG("------------------------------------------------------");
		for (auto it= mFactoryMap.begin(); it.have(); it.next())
		{
			AutoEventFactory hF = it.get();
			if (hF)
				INFO_LOG("[%s] ID [%d]", hF->GetEventName(), hF->GetNameIndex());
		}
		//for (size_t i=0; i<mFactoryMap.size(); ++i)
		//{
		//	const EventFactoryMap::Value &v = mFactoryMap.getValue(i);
		//	if (v.mVal)
		//		INFO_LOG("[%s] ID [%d]", v.mVal->GetEventName(), v.mKey);
		//}
		INFO_LOG("------------------------------------------------------"); 

#if SHOW_EVENT_TICK
		NOTE_LOG("------------------------------------------------------");
		for (auto it= mFactoryMap.begin(); it.have(); it.next())
		{
			AutoEventFactory hF = it.get();
			if (hF && hF->mCount > 0)
			{
				NOTE_LOG("[%s] Count=%d, Tick=%d", hF->GetEventName(), hF->mCount, hF->mAverageTick);
			}
		}
		NOTE_LOG("------------------------------------------------------"); 
#endif
	}

	void tEventCenter::Release()
	{
		delete this;
	}


	bool tEventCenter::_StartWaitTime( AutoEvent waitEvent, float waitSecond, EventCallBack callBack )
	{
		// ����ݹ���ã���ʹ�ȴ�ʱ��Ϊ�㣬Ҳ�����б��У�ֱ����һִ֡�У�Э�̵�����
		//if (waitSecond<=0)
		//{
		//	if (callBack.Nothing())
		//		waitEvent->DoTimeOver();
		//	else
		//		callBack(waitEvent.getPtr());
		//}
		//else
			mFixedTimeManager->_OnWaitEvent(waitEvent, waitSecond, callBack); 
		return true;

	}

	bool tEventCenter::_StartUpdateEvent( AutoEvent updateEvent, float spaceSecond )
	{
		mFixedTimeManager->_OnStartUpdateEvent(updateEvent, spaceSecond); 
		return true;
	}

	void tEventCenter::Log( const char *szInfo, ... )
	{
#if DEVELOP_MODE
		va_list va;
		va_start(va,szInfo);
		LOG_TOOL(va,szInfo);
#endif
	}

	AutoEvent tEventCenter::WaitTime( float fSecond, EventCallBack callBack )
	{
		AutoEvent waitTime = StartDefaultEvent("_TM_WAITTIME_DO_CALL_");
		waitTime->WaitTime(fSecond, callBack);
		return waitTime;
	}

	AutoEvent tEventCenter::WaitUpdate( float fSpaceTime, EventCallBack callBack )
	{
		Hand<_UpdateEvent> evt = StartEvent("_UpdateEvent");
		AssertEx(evt, "_UpdateEvent No register");
		evt->mUpdateCall = callBack;
		evt->StartUpdate(fSpaceTime);	
		return evt;
	}

	void tEventCenter::StopWaitTimeEvent(AutoEvent evt)
	{
		if (mFixedTimeManager!=NULL)
			mFixedTimeManager->RemoveWaitEvent(evt);
	}

	void tEventCenter::StopUpdateEvent(AutoEvent evt)
	{
		if (mFixedTimeManager!=NULL)
			mFixedTimeManager->RemoveUpdateEvent(evt);
	}

	void tEventCenter::PauseEvent(AutoEvent evt)
	{
		if (mFixedTimeManager!=NULL)
			mFixedTimeManager->PauseEvent(evt);
	}

	void tEventCenter::ContinueEvent(AutoEvent evt)
	{
		if (!evt->getFinished() && mFixedTimeManager!=NULL)
			mFixedTimeManager->ContinueEvent(evt);
	}

	bool tEventCenter::SerializeMsg( AutoEvent &hEvent, DataStream *destData )
	{
#if SERIALIZE_EVENT_NAME
		hEvent->GetEventFactory()->SaveEventName(destData);
#else
		hEvent->GetEventFactory()->SaveEventNameIndex(destData);
#endif			
		int id = hEvent->GetEventFactory()->mID;
		if (id>0xffff)
		{
			ERROR_LOG("[%s] Event ID over 65535", hEvent->GetEventName());
			id = 0;
		}

		destData->write((ushort)id);		

		if (hEvent->_SaveDataType(destData) && hEvent->_Serialize(destData))
			return true;

		ERROR_LOG("get event [%s] data stream error", hEvent->GetEventNameInfo().c_str());
		return false;
	}

	AutoEvent tEventCenter::RestoreMsg( DataStream *scrData )
	{
#if SERIALIZE_EVENT_NAME
		AutoEvent hEvent;
		AString eventName;
		if (!scrData->readString(eventName))
			return AutoEvent();

		ushort id = 0;
		if (!scrData->read(id))
		{
			ERROR_LOG("Read event [%s] id fail", eventName.c_str());
			return AutoEvent();
		}
		if (id>0)
		{
			hEvent = _Start(id, MAKE_INDEX_ID(eventName.c_str()));
			if (!hEvent)
			{
				ERROR_LOG("����ID�����¼�ʧ��>[%d]", id);			
				hEvent = StartEvent(eventName);
			}
		}
		else
			hEvent = StartEvent(eventName);

		if (hEvent)
		{
			if (hEvent->_RestoreDataType(scrData)==NULL_NICEDATA)
			{
				ERROR_LOG("[%s]�ָ��¼���������Ϊ�� NULL_NICEDATA", hEvent->GetEventName());
				return AutoEvent();
			}

			if (hEvent->_Restore(scrData))
			{				
				return hEvent;
			}
			ERROR_LOG("Error: Fail restor event form data stream, may be is same event type, example default net event use ClientEvent receive", eventName.c_str());	
		}
		else
			ERROR_LOG("Error: restor event form data stream, [%s] no exist", eventName.c_str());				
#else				
		AutoEvent hEvent;
		int eventTypeIndex = 0;
		if (scrData->read(eventTypeIndex))
		{			
			ushort id = 0;
			if (!scrData->read(id))
			{
				ERROR_LOG("Read event [%d] id fail", eventTypeIndex);
				return AutoEvent();
			}
			if (id>0)
			{
				hEvent = _Start(id, eventTypeIndex);
				if (!hEvent)
				{
					//ERROR_LOG("����ID�����¼�ʧ��>[%d]", id);			
					hEvent = StartEvent(eventTypeIndex);

                    auto factory = hEvent->GetEventFactory();
                    if (factory)
                    {
                        if (factory->mID!=0 && factory->mID!=id)
                        {
                            WARN_LOG("[%s] Event factory �ظ������˲�ͬ��ID����", factory->GetEventName());
                        }
                        factory->mID = id;

                        if (id>=mArrayIndex.size())
                            mArrayIndex.resize(id+8);
                    
                        mArrayIndex[id] = factory;
                        TABLE_LOG("Set ID [%d] >%s", id, factory->GetEventName());
                    }
				}
			}
			else
				hEvent = StartEvent(eventTypeIndex);
			
			if (hEvent)
			{
				//NOTE_LOG("Restore msg >%s", hEvent->GetEventName());
				if (hEvent->_RestoreDataType(scrData)==NULL_NICEDATA)
				{
					ERROR_LOG("[%s]�ָ��¼���������Ϊ�� NULL_NICEDATA", hEvent->GetEventName());
					return AutoEvent();
				}
			
				//if (dataType==eArrayNiceData)
				//{
				//	if (dynamic_cast<NiceEventEx*>(hEvent.getPtr())==NULL && typeid(hEvent->GetData())!=typeid(ArrayNiceData))
				//	{
				//		ERROR_LOG("���ջָ��¼� [%s] ����ʹ�� ArrayNiceData, ���Լ̳� ArrayNiceEvent �� NiceEventEx", hEvent->GetEventName());
				//		return AutoEvent();
				//	}
				//}

				if (hEvent->_Restore(scrData))
					return hEvent;
				ERROR_LOG("Error: restor event [%s] form data stream", hEvent->GetEventName());
			}
			else
				ERROR_LOG("Error: restor event form data stream, [%d] no exist", eventTypeIndex);
		}
		else
			ERROR_LOG("Error: restor event form data stream");
#endif
		return AutoEvent();
	}

	AutoEvent tEventCenter::_Start( ushort eventID, int eventNameIndex )
	{
		if (eventID>0 && eventID<mArrayIndex.size())
		{
			AutoEventFactory hF = mArrayIndex[eventID];
			if (hF)
			{
				if (eventNameIndex!=hF->GetNameIndex())
				{
					ERROR_LOG("Event [%s] by ID [%d], ���¼������������ [%d]", hF->GetEventName(), eventID, hF->GetNameIndex());
					return AutoEvent();
				}

				if (dynamic_cast<EmptyFactroy*>(hF.getPtr())!=NULL)
				{
					ERROR_LOG("[%s] Use Empty event, may be event is not register", hF->GetEventName());
					return AutoEvent();
				}

				return hF->StartEvent();
			}
		}
		return AutoEvent();
	}

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------

	AutoEvent EventCenter::StartDefaultEvent( const char* eventName )
	{
		AutoEvent hEvent = StartEvent(eventName, false);
		if (!hEvent)
		{
			RegisterDefaultEvent(eventName);
			hEvent = StartEvent(eventName);
			if (!hEvent)
			{
				ERROR_LOG("Error: logic error, register default event fail.");
			}
		}
		return hEvent;
	}

	class _DefaultEvent : public CEvent
	{
	public:
		virtual void InitData() override {}
	};

	Logic::AutoEventFactory EventCenter::RegisterDefaultEvent( const char* szEventName )
	{
		Logic::tEventFactory  *pFact = MEM_NEW EventFactory<Logic::_DefaultEvent>();
		Logic::AutoEventFactory hFact = pFact;
		RegisterEvent(szEventName, hFact);
		return hFact;
	}

	EventCenter::EventCenter()
	{		

	}

	//bool EventCenter::SendEvent( AutoEvent &event, int nNetType /*= 0*/, int nTarget /*= 0*/ )
	//{
	//	if (mNet)
	//	{
	//		HandConnect conn = mNet->GetConnect(nTarget);
	//		if (conn)
	//			return conn->SendEvent(event.getPtr());
	//	}
	//	return false;
	//}

	void EventCenter::_SetNetTool(int nType, AutoNet netTool)
	{
		mNet = netTool;

		if (mNet && !mNet->NeedUpdateMsgIndex())
		{
			// ���������¼���������
			mArrayIndex.clear(false);
			mArrayIndex.push_back(AutoEventFactory());
			for (auto it=mFactoryArray.begin(); it; ++it)
			{
				auto f = it.get();
				if (f)
				{
					f->mID = mArrayIndex.size();
					mArrayIndex.push_back(f);
				}
			}
		}
	}

	//----------------------------------------------------------------------
}