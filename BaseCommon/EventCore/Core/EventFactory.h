//-------------------------------------------------------------------
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : tEventFactory.h
//  @ Date : 2011-5-18
//  @ Author : ���ĸ�
// �¼�����ϵͳ
// �������ݣ���ͷ���ù�����ʽ�������һ�����޸ģ������ؽ�����������Ӱ��
// �޸�֮ǰ�ļ�¼����ʹ��
//-------------------------------------------------------------------


#if !defined(_TEVENTFACTORY_H)
#define _TEVENTFACTORY_H

#include "EventCore.h"
//#include "NiceTable.h"
#include "Event.h"
#include "EasyList.h"

#include "EasyStack.h"
#include "NiceDataProtocol.h"
//-------------------------------------------------------------------------
enum EEventPacketState
{
	ePacketStateEncrypt	= 1,				// ��Ҫ����
	ePacketStateZip			= 1<<1,		// �Ƿ�ѹ��
	eEventDataPacket		= 1<<2,		// �¼����ݰ������ڴӽ�������ֱ�Ӷ�ȡ�ָ��¼�����
    eFactoryNoProtocolSave = 1<<3,  // ���������¼�ʱ��ʹ��Э�鷽ʽ
};
//-------------------------------------------------------------------------*/
namespace Logic
{
	class tEventCenter;
	typedef AutoTable FactoryTable;
	class tEventFactory;
	class tServerEvent;
	//----------------------------------------------------------------
	class EventCoreDll_Export tEventFactory : public Base<tEventFactory>
	{
	public:
		typedef EasyMap<int, EasyString>	SelectDataMap;

	public:
		friend class tEvent;
		friend class CEvent;
		friend class tEventCenter;
		friend class EventCenter;
		friend class BaseEvent;		

		void Release();
		virtual void ReaydyListNodePool( AutoPtr<NodePool>  hPool) = 0;

	public:
		tEventCenter* GetEventCenter(void)const{ return mEventCenter; }
		void OnRemove();

	public:
		tEventFactory();
		virtual ~tEventFactory();

	public:
		virtual void Initialize(void){}
        virtual int GetID() const { return mID; }
        virtual void SetID(int id){ mID = id; } 
		virtual int GetNameIndex(void){ return mNameIndex; }
		virtual const char* GetEventName(void) const { return mEventName.c_str(); }
        virtual void SetEventName(const char* strEventName);

		virtual AString GetNextEventName(void){ return AString(); }
		virtual NiceData& GetData(void) = 0;
		virtual void SetConfigTable(AutoTable hTable) = 0;
		virtual AutoTable GetConfigTable(void) const = 0;
		virtual const AutoEvent& GetTemplate(void)const = 0;

		virtual void _SetHelpTable(AutoTable helpTable) = 0;
		virtual AutoTable _GetHelpTable() const = 0;

		virtual SelectDataMap& GetSelectData(void) = 0;
		virtual void AppendSelectData(const char* indexName) = 0;
		virtual bool RemoveSelectData(const char* indexName) = 0;

		virtual void SetState(int nState, bool bOpen) = 0;
		virtual bool HasState(int state) = 0;

		//virtual void SetDataProtocol(ANiceDataProtocol protocol){}
		//virtual ANiceDataProtocol& GetDataProtocol(){ static ANiceDataProtocol p; return p; }

	protected:
		virtual AutoEvent NewEvent() = 0; 
		virtual void FreeEvent(tEvent *event);
		virtual bool _HasRelation(AutoEvent &hEvent) { return false; }
		virtual bool SaveEventName(DataStream *destData);
		virtual bool SaveEventNameIndex(DataStream *destData);


	public:		
		virtual AutoEvent StartEvent(void);

		virtual void _OnEvent(AutoEvent &hEvent) = 0;
		virtual bool _NeedWaitTrigger(void) = 0;
		virtual void _OnWaitEvent(AutoEvent hEvent) = 0; 
		virtual void _RemoveWaitEvent(EVENT_ID) = 0;

		virtual void _NodifyEventFinish(AutoEvent hEvent) = 0;

		virtual void AppendTriggerEventName(const char* strEventName){}
		virtual void RemoveTriggerEventName(const char* strEventName){}

		virtual void AllotEventID(tServerEvent *serverEvent){}
		virtual void FreeServerEvent( tServerEvent *serverEvent ){}

        virtual int GetNiceDataProtocolKey(){ return mNiceProtocol ? mNiceProtocol->GetField()->GetCheckCode() : 0; }

	public:
		virtual void Process(void *param) = 0;

#if SHOW_EVENT_TICK
		int				mCount;				// ��������
		UInt64			mStartTick;			// ��ʼ����ʱ��
		int				mAverageTick;		// ƽ������ʱ��
#endif

	protected:
		tEventCenter	*mEventCenter;
		EasyString		mEventName;
		int				mNameIndex;
		int				mID;
        AutoTable   mNiceProtocol;
        DataBuffer   mNiceProtocolData;
		EasyStack<tEvent*>	mFreeEventPool;
	};

	//----------------------------------------------------------------

	//----------------------------------------------------------------
	typedef Hand<tEventFactory>				AutoEventFactory;
	typedef Array<AutoEventFactory>			EventFactoryArray;

	// ȷ���¼�����, ��Ҫȷ�����¼��ڻ�ȡʱ, �ͱ����뵽�ȴ�ȷ�����б���, ֱ���¼�ִ����Finish
	class EventCoreDll_Export CEventFactory : public tEventFactory
	{
	public:
		CEventFactory();
		virtual ~CEventFactory();

	public:
		virtual void ReaydyListNodePool( AutoPtr<NodePool> hPool);

		virtual void SetConfigTable(AutoTable hTable);
		virtual AutoTable GetConfigTable(void) const;
		virtual NiceData& GetData(void){ return mConfigData;}
		const AutoEvent& GetTemplate(void) const { return mTemplateEvent; }
		virtual AString GetNextEventName(void){ return AString(); }

		virtual void _SetHelpTable(AutoTable helpTable);
		virtual AutoTable _GetHelpTable() const;

		virtual SelectDataMap& GetSelectData(void) { return mSelectData; }
		virtual void AppendSelectData(const char* indexName){ mSelectData.insert(MAKE_INDEX_ID(indexName), EasyString(indexName)); }
		virtual bool RemoveSelectData(const char* indexName) { return mSelectData.erase(MAKE_INDEX_ID(indexName))>0; };
		
	public:
		virtual void Initialize( void );

		virtual bool _NeedWaitTrigger(void){ return false; }
		virtual void _OnWaitEvent(AutoEvent hEvent); 
		virtual void _RemoveWaitEvent(EVENT_ID) {}

		virtual void _NodifyEventFinish(AutoEvent hEvent);

		virtual void _OnEvent(AutoEvent &hEvent);

		void RemoveTriggerEvent(AutoEvent &event);

		virtual void AppendTriggerEventName(const char* strEventName);
		virtual void RemoveTriggerEventName(const char* strEventName);
		//ʹ�ù�ϵ�¼��б����ж��Ƿ���ָ���¼��й�ϵ
		virtual bool _HasRelation(AutoEvent &hEvent);

		bool _ExistTriggerEventList(const char* eventName);
		// NOTE: ��ǰ�¼�������ֹͣ�˹��ܵĵ���
		virtual void Process(void *param);

	public:
		virtual void SetState(int nState, bool bOpen) { mState.set(nState, bOpen); }
		virtual bool HasState(int state) { return mState.isOpen(state); }

		//virtual void SetDataProtocol(ANiceDataProtocol protocol){mDataProtocol = protocol; }
		//virtual ANiceDataProtocol& GetDataProtocol(){ return mDataProtocol; }

	protected:
		StateData				mState;
		mutable bool			mNeedRemove;
		mutable NiceData		mConfigData;
		EventList				mTriggerEventList;
		EasyMap<int, AString>	mTriggerEventNameMap;
		SelectDataMap			mSelectData;
		AutoTable				mHelpTable;

		//ANiceDataProtocol		mDataProtocol;

		AutoEvent				mTemplateEvent; // ֧�ֶ���

	public:
		virtual void SetOnBeginFunName(const char* funName){  }
		virtual const EasyString& GetOnBeginFunName(void) const { static EasyString tempName; return tempName; }
		virtual void SetDoEventFunName(const char* funName){  }
		virtual const EasyString& GetDoEventFunName(void) const { static EasyString tempName; return tempName; }
		virtual void SetOnEventScriptFunName(const char* strFunName){ }
		virtual const EasyString& GetOnEventFunName(void)const { static EasyString tempName; return tempName; }
		virtual void SetOnFinishFunName(const char* funName){  }
		virtual const EasyString& GetOnFinishFunName(void) const {static EasyString tempName; return tempName; }

		// for Logic node
		virtual void SetNextLogicFunName(const char* funName){ }
		virtual const EasyString& GetNextLogicFunName(void) const { static EasyString tempName; return tempName; }
		// for Logic event
		virtual void SetBeginLogicName(const char* logicName){  }
		virtual const EasyString& GetBeginLogicName()const{ static EasyString tempName; return tempName; }

	};
	//----------------------------------------------------------------

	template<typename E, bool bNeedLog = true>
	class EventFactory : public Logic::CEventFactory
	{
		virtual AutoEvent NewEvent()
		{
			AutoEvent hEvt = MEM_NEW E();
			hEvt->setLog(bNeedLog);
			return hEvt;
		}

	public:
		EventFactory(bool bEncrypt = true)
		{
			SetState(ePacketStateEncrypt, bEncrypt);
		}
	};
	//-------------------------------------------------------------------------
}

#define REGISTER_EVENT(center, eventClass)	 center->RegisterEvent(#eventClass, MEM_NEW Logic::EventFactory<eventClass, false>());

#ifndef STATE_EVENTCORE_LIB

template<>
void Hand<Logic::tEventFactory>::FreeClass(Logic::tEventFactory *p)
{
	p->Release();
}
template<>
void Hand<Logic::tEventFactory>::FreeUse()
{
	mUse->Release();
}
#endif

#endif  //_TEVENTFACTORY_H