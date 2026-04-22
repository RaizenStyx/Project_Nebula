// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../NebulaSkillBase.h"
#include "NebulaEssenceTemplate.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_NEBULA_API UNebulaEssenceTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Essence Identity")
    FText EssenceName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Essence Identity")
    FText EssenceLore;

    // The unique, stronger passive granted by this essence
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Essence Skills")
    TSubclassOf<UNebulaSkillBase> GrantedStrongerPassive;

    // Optional: Does this essence force a Star Rank evolution?
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Essence Evolution")
    bool bEvolvesClassRank = false;
	
};
