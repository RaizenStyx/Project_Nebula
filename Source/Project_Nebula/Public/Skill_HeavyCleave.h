// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NebulaSkillBase.h"
#include "Skill_HeavyCleave.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_NEBULA_API USkill_HeavyCleave : public UNebulaSkillBase
{
	GENERATED_BODY()

public:
	USkill_HeavyCleave();

	virtual void ExecuteSkill(AProject_NebulaCharacter* Caster) override;
	
};
