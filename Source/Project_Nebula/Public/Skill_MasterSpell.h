#pragma once

#include "CoreMinimal.h"
#include "../NebulaSkillBase.h"
#include "../NebulaProjectile.h"
#include "Skill_MasterSpell.generated.h"

UCLASS()
class PROJECT_NEBULA_API USkill_MasterSpell : public UNebulaSkillBase
{
    GENERATED_BODY()

public:
    USkill_MasterSpell();

    UPROPERTY(EditDefaultsOnly, Category = "Spell Setup")
    TSubclassOf<ANebulaProjectile> ProjectileClass;

    virtual void ExecuteSkill(AProject_NebulaCharacter* Caster) override;
    virtual void ExecuteHoldSkill(AProject_NebulaCharacter* Caster) override;

protected:
    bool MeetsClassRequirements(AProject_NebulaCharacter* Caster) const;
};