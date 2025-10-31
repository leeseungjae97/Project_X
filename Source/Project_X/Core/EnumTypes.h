#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.generated.h"

UENUM(BlueprintType)
enum class EAugmentEffectType : uint8
{
	Heal,
	HealOverTime,
	DamageBoost,
	LifeSteal,
	StaminaRegen,
	ShieldGain,
};

UENUM(BlueprintType)
enum class EAugmentTriggerCondition : uint8
{
	PassiveAlways,       // 항상 적용
	OutOfCombat,         // 비전투 중
	OnHit,               // 공격 시
	OnKill,              // 적 처치 시
	OnDodge,             // 회피 성공 시
};

UENUM(BlueprintType)
enum class EAugmentTarget : uint8
{
	Self,				// 자기 자신에게 효과 적용
	Enemy,				// 공격 대상에게 효과가 적용됨
	Ally,				// 주변 아군에게 효과가 적용됨
	Area,				// 지속 영역 효과나 범위 내 모두에게 적용
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Projectile,
	HitScan,
	Melee
};

UENUM(BlueprintType)
enum class EPXCollisionShapeType : uint8
{
	Box,
	Sphere,
	Capsule
};
