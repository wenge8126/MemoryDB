
#ifndef _INCLUDE_NETINDEXPACKET_H_
#define _INCLUDE_NETINDEXPACKET_H_

#include "EventPacket.h"
#include "PacketFactory.h"
//-------------------------------------------------------------------------*/
// ֪ͨ��Ϣ�������Ż�ʹ��������ϢID��������Ϣ
class NotifyEventIndexPacket : public DataPacket
{
public:
	NotifyEventIndexPacket()
	{
		SetData(MEM_NEW DataBuffer(512), 0);
	}

	virtual	PacketID_t	GetPacketID( ) const { return  eNotifyNetEventID; } 

	virtual UINT		Execute( tNetConnect* pConnect ) 
	{
		pConnect->GetNetHandle()->GetEventCenter()->UpdateNetMsgIndex(mData);
		return 0;
	}

	// ����Ϣ���ݲ����м���
	virtual void SetNeedEncrypt(bool bNeed) const override { mStateData.close(ePacketStateEncrypt); }
};
class IndexPacketFactory : public PacketFactory
{
public:
	virtual HandPacket	CreatePacket () override { return mIndexPacket; }
	virtual PacketID_t	GetPacketID ()const override { return eNotifyNetEventID; } 
	virtual	void		destroyPacket(Packet* pPacket) override {}

public:
	HandPacket	mIndexPacket;

public:
	IndexPacketFactory()
	{
		mIndexPacket = MEM_NEW NotifyEventIndexPacket();
	}
};
//-------------------------------------------------------------------------*/
// ���������ɷ�������10��ͳһ���͸��ͻ��ˣ��ͻ���ֻ���������հ�ʱ��
// NOTE: Ŀǰ�������ܾ����մ���Ϣ
class HeartBeatPacket : public Packet
{
public:
	virtual	PacketID_t	GetPacketID( ) const { return  eNotifyHeartBeat; }
	virtual	UINT		GetPacketSize( ) const { return 0; }

	virtual BOOL		Read( LoopDataStream& iStream, size_t packetSize ) { return TRUE; }
	virtual BOOL		Write( LoopDataStream& oStream ) const { return TRUE; }

	virtual UINT		Execute( tNetConnect* pConnect ) 
    { 
#ifdef __SERVER__
        WARN_LOG("No thing, now server can not this packet"); 
#endif
		NOTE_LOG("���յ�һ��������");
        return 0; 
    }

	virtual UINT		GetState() const { return 0; }
	virtual VOID		SetState(UINT stateData) { }
};
class HeartBeatPacketFactory : public PacketFactory
{
public:
	virtual HandPacket	CreatePacket () override  { return mHeartPacket; }
	virtual PacketID_t	GetPacketID ()const override { return eNotifyHeartBeat; } 
	virtual	void		destroyPacket(Packet* pPacket) override {}

public:
	HandPacket	mHeartPacket;

public:
	HeartBeatPacketFactory()
	{
		mHeartPacket = MEM_NEW HeartBeatPacket();
	}
};
//-------------------------------------------------------------------------*/

#endif //_INCLUDE_NETINDEXPACKET_H_