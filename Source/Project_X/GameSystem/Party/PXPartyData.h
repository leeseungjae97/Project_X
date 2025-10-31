#pragma once

#include "CoreMinimal.h"
#include "PXPartyData.generated.h"

USTRUCT(BlueprintType)
struct FPartyMemberInfo
{
	GENERATED_BODY()

public:
	int64 ObjectId;
	FString Name;
	int32 Level;
	bool IsReady = false;
};

USTRUCT(BlueprintType)
struct FPartyInfo
{
	GENERATED_BODY()

public:
	int64 PartyId;
	int64 LeaderObjectId;
	TMap<int64, FPartyMemberInfo> MemberMap;
};