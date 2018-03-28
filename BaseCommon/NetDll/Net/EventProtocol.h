/********************************************************************
	created:	2013/05/14
	created:	14:5:2013   17:32
	filename: 	C:\Work\Game\Mushroom\BaseCommon\ServerBase\Base\EventProtocol.h
	file path:	C:\Work\Game\Mushroom\BaseCommon\ServerBase\Base
	file base:	EventProtocol
	file ext:	h
	author:		Yang WenGe
	
	purpose:	
*********************************************************************/
#ifndef _INCLUDE_EVENTPROTOCOL_H_
#define _INCLUDE_EVENTPROTOCOL_H_

#include "NetHandle.h"
#include "NetHead.h"
#include "Packet.h"
//#include "TableRecord.h"
#include "Event.h"
#include "EventPacket.h"
//----------------------------------------------------------------------
enum
{
	ID_INDEX_COL	= 0,
	VALUENAME_COL	= 1,
	TYPE_COL		= 2,
	DEFAULT_COL		= 3,
	CHECK_COL_MAX
};

const char gMsgTableFieldName[4][16] = { "INDEX", "VALUENAME", "TYPE", "DEFAULT" };

#define TABLE_TYPE_STRING_FLAT	"TABLE"
//----------------------------------------------------------------------------------------------
// �¼�������Ϣ�����������¼������л��¼��� ��Ҫ��д������
class EventPacket;
class EventData;
struct PacketData;

class Net_Export EventNetProtocol : public tNetProtocol
{
	friend class BaseNetHandle;

public:
	EventNetProtocol();

public:
	// ��Ϣ���ݶ���
	//AutoRecord GetStructRecord( AutoEvent evt, const AString &structDefineIndex, AutoEvent parentEvent, int nIndex, bool bCreate = true );

public:
	virtual bool RegisterNetPacket(AutoPacketFactory f, bool bRespace = true);
	virtual int AppendNetPacketFrom(tNetProtocol *other, bool bReplace);

	virtual HandPacket	CreatePacket (PacketID packetID);

	virtual HandPacket GenerateEventPacket(tNetConnect *pConnect, Logic::tEvent *pSendEvent, bool bNeedZip = true);

	virtual bool WritePacket( const Packet* pPacket, LoopDataStream *mSocketOutputStream ) override;
	virtual HandPacket ReadPacket( tNetConnect *pConnect, LoopDataStream *mSocketInputStream ) override;		

	virtual void OnWritePacketError(AString strErrorInfo);
	virtual void OnReadPacketError(AString strErrorInfo);
	// Ϊ�����簲ȫ, �������Э��ָ�ʧ��ʱ, ȫ��������Ӵ���
	virtual void OnPacketExecuteError(tNetConnect *pConnect, Packet *pPacket)
	{
		WARN_LOG("Net packet excute error >[%d], then now remove connect >[%s:%d]", pPacket->GetPacketID(), pConnect->GetIp(), pConnect->GetPort());
		pConnect->SetRemove(true); 
	}
	
    virtual void OnBeforeSendEvent(tNetConnect *pConnect, Logic::tEvent *pEvent);

    virtual bool UseNiceDataProtocol() const { return true; }

public:
	virtual bool CheckPacketInfo(tNetConnect *pConnect, PacketID_t packetID, UINT packetSize);

protected:
	ArrayMap<AutoPacketFactory> mNetPacketFactoryList;

public:
	static bool SendEvent(tNetConnect *connect, Logic::tEvent *sendEvent);

public:
	// NOTE: IOCP�����̣߳��������ݺ�ֱ�ӽ����¼����ݣ�Ȼ��ֱ�ӷ��봦������
	static void ReadAnalyzeEventData(tNetConnect *pConnect, LoopDataStream *pReceiveData);

	static void ProcessEventData(tNetConnect *pConnect, EventData *pData);

	static void ProcessPacketData(tNetConnect *pConnect, PacketData *pData);
};

//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
#endif //_INCLUDE_EVENTPROTOCOL_H_