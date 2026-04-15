// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NebulaSkillBase.generated.h"

class AProject_NebulaCharacter;

UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class PROJECT_NEBULA_API UNebulaSkillBase : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Info")
    FName SkillName;

    // We use a single resource cost, pulling from the unified Stamina pool
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Info")
    float ResourceCost = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Info")
    UAnimMontage* CastMontage;

    // Called on Tap
    virtual void ExecuteSkill(AProject_NebulaCharacter* Caster);

    // Called on Hold
    virtual void ExecuteHoldSkill(AProject_NebulaCharacter* Caster);

protected:
    bool CanCast(AProject_NebulaCharacter* Caster) const;
    void ConsumeResources(AProject_NebulaCharacter* Caster);
};