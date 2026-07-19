// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill_MagicBlast.h"
#include "../Project_NebulaCharacter.h"
#include "../NebulaProjectile.h"
#include "Engine/World.h"

USkill_MagicBlast::USkill_MagicBlast()
{
    // Set our defaults so you don't have to remember to do it in the Editor
    SkillName = "Arcane Blast";
    SkillCategory = ENebulaSkillCategory::Class;
    bIsManaType = true;
    bIsPassive = false;
    ResourceCost = 25.0f; // A bit more expensive than the normal attack
}

void USkill_MagicBlast::ExecuteSkill(AProject_NebulaCharacter* Caster)
{
    // 1. Check if we have enough mana
    if (!CanCast(Caster)) return;

    // 2. Eat the mana
    ConsumeResources(Caster);

    if (Caster && ProjectileClass)
    {
        // 3. Play the casting animation
        if (CastMontage)
        {
            Caster->PlayAnimMontage(CastMontage);
        }

        // 4. Spawn the projectile
        UWorld* World = Caster->GetWorld();
        if (World)
        {
            // Calculate where it should spawn (e.g., 100 units in front of the player)
            // You can replace this with a socket location from the weapon if you prefer
            FVector SpawnLocation = Caster->GetActorLocation() + (Caster->GetActorForwardVector() * 100.0f);
            FRotator SpawnRotation = Caster->GetActorRotation();

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = Caster;
            SpawnParams.Instigator = Caster;

            // Spawn it into the world
            ANebulaProjectile* SpawnedProjectile = World->SpawnActor<ANebulaProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

            if (SpawnedProjectile)
            {
                // We can double the size of the projectile directly in code!
                SpawnedProjectile->SetActorScale3D(FVector(2.0f, 2.0f, 2.0f));
            }
        }
    }
}