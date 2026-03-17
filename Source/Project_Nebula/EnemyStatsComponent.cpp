// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyStatsComponent.h"
#include "Math/UnrealMathUtility.h"

UEnemyStatsComponent::UEnemyStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Default Initialization
    BaseAttack = 10.0f;
    BaseDefense = 5.0f;

    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;

    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;
}

void UEnemyStatsComponent::BeginPlay()
{
    Super::BeginPlay();

    // Ensure pools are full on spawn
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
}

// -------------------------------------------------------------------
// DAMAGE & RESOURCE LOGIC
// -------------------------------------------------------------------

void UEnemyStatsComponent::ReceivePhysicalDamage(float IncomingCalculatedDamage)
{
    // Final step of the damage formula: IncomingCalculatedDamage - Target Defense
    float ActualDamage = IncomingCalculatedDamage - BaseDefense;

    // Clamp to 0 so high defense can't accidentally heal the enemy
    ActualDamage = FMath::Max(ActualDamage, 0.0f);

    // Apply as negative to ModifyHealth
    ModifyHealth(-ActualDamage);
}

void UEnemyStatsComponent::ModifyHealth(float Amount)
{
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    // TODO: Check if CurrentHealth <= 0 here to trigger death/despawn logic and xp
}