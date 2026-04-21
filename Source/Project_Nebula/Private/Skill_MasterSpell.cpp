#include "Skill_MasterSpell.h"
#include "../Project_NebulaCharacter.h"
#include "../PlayerStatsComponent.h"

USkill_MasterSpell::USkill_MasterSpell()
{
    SkillName = "Master Spell";
    ResourceCost = 15.0f; // Pulls from the same stamina pool, but UI will call it Mana
}

bool USkill_MasterSpell::MeetsClassRequirements(AProject_NebulaCharacter* Caster) const
{
    if (Caster)
    {
        // Find the stats component on the caster
        if (UPlayerStatsComponent* StatsComp = Caster->FindComponentByClass<UPlayerStatsComponent>())
        {
            // Return true ONLY if the mana system is unlocked
            return StatsComp->bIsManaUnlocked;
        }
    }
    return false;
}

void USkill_MasterSpell::ExecuteSkill(AProject_NebulaCharacter* Caster)
{
    if (!MeetsClassRequirements(Caster) || !CanCast(Caster)) return;

    ConsumeResources(Caster);

    if (CastMontage) Caster->PlayAnimMontage(CastMontage);

    // Spawn the elemental projectile
    if (ProjectileClass && Caster->GetWorld())
    {
        FVector SpawnLoc = Caster->GetActorLocation() + (Caster->GetActorForwardVector() * 100.f);
        FRotator SpawnRot = Caster->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Instigator = Caster;

        Caster->GetWorld()->SpawnActor<ANebulaProjectile>(ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
    }
}

void USkill_MasterSpell::ExecuteHoldSkill(AProject_NebulaCharacter* Caster)
{
    if (!MeetsClassRequirements(Caster)) return;

    // In the future: Caster->CycleElementalAttunement();
    GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Master Spell HOLD: Element Cycled!"));
}