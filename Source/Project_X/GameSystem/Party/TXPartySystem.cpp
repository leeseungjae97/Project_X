// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystem/Party/TXPartySystem.h"
#include "CombatCharacter.h"

void UTXPartySystem::CreateParty()
{
}

void UTXPartySystem::ChangePartyLeader(int64 ObjectId)
{
	PartyInfo.LeaderObjectId = ObjectId;
	OnPartyLeaderChanged.Broadcast(ObjectId);
}

void UTXPartySystem::PartyKick(int64 ObjectId)
{
	MemberRemove(ObjectId);

	// Protocol::C_PARTY_KICK pkt;
	// pkt.set_target_object_id(ObjectId);
	//
	// SendBufferRef sendBuffer{ ServerPacketHandler::MakeSendBuffer(pkt) };
	// GetGameInstance()->GetSubsystem<UGameNetworkSubsystem>()->SendPacket(sendBuffer);
}

void UTXPartySystem::PartyInvite(const FString& PlayerName)
{
	// Protocol::C_PARTY_INVITE pkt;
	// pkt.set_target_name(TCHAR_TO_UTF8(*PlayerName));
	//
	// SendBufferRef sendBuffer{ ServerPacketHandler::MakeSendBuffer(pkt) };
	// GetGameInstance()->GetSubsystem<UGameNetworkSubsystem>()->SendPacket(sendBuffer);
}

void UTXPartySystem::PartyLeave()
{
	// Protocol::C_PARTY_LEAVE pkt;
	//
	// SendBufferRef sendBuffer{ ServerPacketHandler::MakeSendBuffer(pkt) };
	// GetGameInstance()->GetSubsystem<UGameNetworkSubsystem>()->SendPacket(sendBuffer);
}

void UTXPartySystem::PlayerReady(bool bReady)
{
	// Protocol::C_PARTY_READY pkt;
	// pkt.set_is_ready(bReady);
	//
	// SendBufferRef sendBuffer{ ServerPacketHandler::MakeSendBuffer(pkt) };
	// GetGameInstance()->GetSubsystem<UGameNetworkSubsystem>()->SendPacket(sendBuffer);
}

void UTXPartySystem::SetReady(int64 ObjectId, bool bReady)
{
	if (!PartyInfo.MemberMap.Contains(ObjectId))
		return;

	PartyInfo.MemberMap[ObjectId].IsReady = bReady;
}

void UTXPartySystem::AcceptParty(int64 PartyId, bool bAccept)
{
	// Protocol::C_PARTY_ACCEPT pkt;
	// pkt.set_party_id(PartyId);
	// // pkt.set_accept(bAccept);
	//
	// SendBufferRef sendBuffer{ ServerPacketHandler::MakeSendBuffer(pkt) };
	// GetGameInstance()->GetSubsystem<UGameNetworkSubsystem>()->SendPacket(sendBuffer);
}

bool UTXPartySystem::IsAllReady()
{
	for ( auto Member : PartyInfo.MemberMap)
	{
		FPartyMemberInfo MemberInfo = Member.Value;
		if (!MemberInfo.IsReady)
		{
			return false;
		}
	}

	return true;
}

void UTXPartySystem::MemberAdd(const FPartyMemberInfo& Member)
{
	PartyInfo.MemberMap.Add(Member.ObjectId, Member);

	TArray<FPartyMemberInfo> PartyArray;
	PartyInfo.MemberMap.GenerateValueArray(PartyArray);

	OnPartyDataChanged.Broadcast(PartyArray);
}
void UTXPartySystem::MemberRemove(const FPartyMemberInfo& Member)
{
	if (!PartyInfo.MemberMap.Contains(Member.ObjectId))
		return;

	PartyInfo.MemberMap.Remove(Member.ObjectId);

	TArray<FPartyMemberInfo> PartyArray;
	PartyInfo.MemberMap.GenerateValueArray(PartyArray);

	OnPartyDataChanged.Broadcast(PartyArray);
}
void UTXPartySystem::MemberRemove(int64 ObjectId)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%lld"), ObjectId));
	
	if (!PartyInfo.MemberMap.Contains(ObjectId))
		return;

	PartyInfo.MemberMap.Remove(ObjectId);

	TArray<FPartyMemberInfo> PartyArray;
	PartyInfo.MemberMap.GenerateValueArray(PartyArray);

	OnPartyDataChanged.Broadcast(PartyArray);
}


