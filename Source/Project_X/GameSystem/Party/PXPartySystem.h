// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PXPartyData.h"
#include "PXPartySystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberChanged, const TArray<FPartyMemberInfo>&, Members);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyLeaderChanged, int64, PartyLeaderId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyInvite, int64, PartyId, FString, InviterName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKickFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAddFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartyCreate);

UCLASS()
class PROJECT_X_API UPXPartySystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void CreateParty();

	UFUNCTION(BlueprintCallable)
	void ChangePartyLeader(int64 ObjectId);

	UFUNCTION(BlueprintCallable)
	void PartyKick(int64 ObjectId);

	UFUNCTION(BlueprintCallable)
	void PartyInvite(const FString& PlayerName);

	UFUNCTION(BlueprintCallable)
	void PartyLeave();

	UFUNCTION(BlueprintCallable)
	void PlayerReady(bool bReady);

	UFUNCTION(BlueprintCallable)
	void SetReady(int64 ObjectId, bool bReady);
		
	UFUNCTION(BlueprintCallable)
	void AcceptParty(int64 PartyId, bool bAccept);

	UFUNCTION(BlueprintCallable)
	bool IsAllReady();

	void MemberAdd(const FPartyMemberInfo& Member);
	
	void MemberRemove(const FPartyMemberInfo& Member);

	UFUNCTION()
	void MemberRemove(int64 ObjectId);

	// UFUNCTION(BlueprintPure)
	// FORCEINLINE bool IsGetInvitation() { return bGetInvitation; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int64 GetPartyId() { return PartyInfo.PartyId; }
	FORCEINLINE void SetPartyId(int64 PartyId) { PartyInfo.PartyId = PartyId; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FString GetInviterName() { return InviterName; }

	FORCEINLINE void SetInviterName(FString InInviterName) { InviterName = InInviterName; }

public:
	UPROPERTY(BlueprintAssignable)
	FOnPartyMemberChanged OnPartyDataChanged;

	FOnPartyLeaderChanged OnPartyLeaderChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPartyInvite OnInvite;
	
	UPROPERTY(BlueprintAssignable)
	FOnPartyCreate OnPartyCreate;
	
	UPROPERTY()
	FString InviterName;

	// bool bGetInvitation = false;


private:
	UPROPERTY()
	FPartyInfo PartyInfo;

public:
	FORCEINLINE int32 GetPartyMemberNum() const { return PartyInfo.MemberMap.Num(); }
	FORCEINLINE void SetLeaderObjectId(int64 LeaderId) { PartyInfo.LeaderObjectId = LeaderId; }
};
