#include "Skill_HeavyCleave.h"
#include "../Project_NebulaCharacter.h"

USkill_HeavyCleave::USkill_HeavyCleave()
{
    SkillName = "Heavy Cleave";
    ResourceCost = 25.0f; // Drains 25 Stamina
}

void USkill_HeavyCleave::ExecuteSkill(AProject_NebulaCharacter* Caster)
{
    if (!CanCast(Caster))
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Not enough Stamina!"));
        return;
    }

    ConsumeResources(Caster);

    if (CastMontage)
    {
        Caster->PlayAnimMontage(CastMontage);
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Executed Heavy Cleave!"));
}