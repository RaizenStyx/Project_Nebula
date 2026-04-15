#include "NebulaSkillBase.h"
#include "Project_NebulaCharacter.h"
#include "PlayerStatsComponent.h"

void UNebulaSkillBase::ExecuteSkill(AProject_NebulaCharacter* Caster)
{
    // Default empty implementation. Subclasses override this.
}

void UNebulaSkillBase::ExecuteHoldSkill(AProject_NebulaCharacter* Caster)
{
    // Default empty implementation.
}

bool UNebulaSkillBase::CanCast(AProject_NebulaCharacter* Caster) const
{
    if (!Caster) return false;

    // Get the stats component (Assuming you added it to your Character class!)
    UPlayerStatsComponent* Stats = Caster->FindComponentByClass<UPlayerStatsComponent>();
    if (Stats)
    {
        return Stats->CurrentStamina >= ResourceCost;
    }
    return false;
}

void UNebulaSkillBase::ConsumeResources(AProject_NebulaCharacter* Caster)
{
    if (!Caster) return;

    UPlayerStatsComponent* Stats = Caster->FindComponentByClass<UPlayerStatsComponent>();
    if (Stats)
    {
        // Subtract the cost from the unified Stamina/Mana pool
        Stats->ModifyStamina(-ResourceCost);
    }
}