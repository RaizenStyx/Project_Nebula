// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NebulaSkillBase.h"
#include "Skill_MagicBlast.generated.h"

/**
 * 
 */

class ANebulaProjectile;

UCLASS()
class PROJECT_NEBULA_API USkill_MagicBlast : public UNebulaSkillBase
{
	GENERATED_BODY()

public:
    USkill_MagicBlast();

    // The projectile we want to fire
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blast Settings")
    TSubclassOf<ANebulaProjectile> ProjectileClass;

    // Override the base execution
    virtual void ExecuteSkill(AProject_NebulaCharacter* Caster) override;
	
};
