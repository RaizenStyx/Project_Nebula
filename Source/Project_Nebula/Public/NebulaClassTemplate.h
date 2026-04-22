// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NebulaItemTypes.h"
#include "../NebulaSkillBase.h"
#include "NebulaClassTemplate.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECT_NEBULA_API UNebulaClassTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    // --- Identification ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Identity")
    FText ClassName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Identity")
    FText ClassDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Identity")
    ENebulaStarRank StarRank = ENebulaStarRank::OneStar;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Identity")
    ENebulaResourceType ResourceType = ENebulaResourceType::PurePhysical;

    // Determines if this class locks permanently upon equipping (e.g., Legendary Essence classes)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Identity")
    bool bIsPermanentlyLocked = false;

    // --- Slot Limits ---
    // E.g., 4 for 1-Star, up to 8 for 3-Star
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Capacities")
    int32 MaxActiveSkillSlots = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Capacities")
    int32 MaxPassiveSkillSlots = 4;

    // --- Unlock Schedules ---
    // List of active skills and the levels they unlock at
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Progression")
    TArray<FSkillUnlockNode> ActiveSkillUnlocks;

    // List of passive skills and the levels they unlock at
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Progression")
    TArray<FSkillUnlockNode> PassiveSkillUnlocks;

    // The ultimate EX Skill (unlocked at max level or special condition)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Progression")
    TSubclassOf<UNebulaSkillBase> EXSkill;
	
};
