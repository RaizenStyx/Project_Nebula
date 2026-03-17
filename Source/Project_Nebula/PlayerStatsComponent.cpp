// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStatsComponent.h"
#include "Math/UnrealMathUtility.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize Base RPG Attributes
    PhysicalProwess = 10.0f;
    Synchronization = 10.0f;
    Agility = 10.0f;
    Fortitude = 10.0f;
    Vigor = 10.0f;

    // Initialize Resources (overwritten by CalculateDerivedStats in BeginPlay, but good practice)
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;

    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;

    MaxAwakeTimerMinutes = 960.0f; // 16 Hours * 60 Mins
    CurrentAwakeTimerMinutes = MaxAwakeTimerMinutes;
}

void UPlayerStatsComponent::BeginPlay()
{
    Super::BeginPlay();

    // Calculate actual pools based on starting attributes
    CalculateDerivedStats();

    // Fill pools to max on spawn
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
    CurrentAwakeTimerMinutes = MaxAwakeTimerMinutes;
}

// -------------------------------------------------------------------
// MATH & LOGIC
// -------------------------------------------------------------------

float UPlayerStatsComponent::GetEffectiveStatValue(float BaseStatValue) const
{
    // The Scaling Engine: Diminishing Returns Model
    float EffectiveStat = 0.0f;

    if (BaseStatValue <= 100.0f)
    {
        // Points 0-100: 100% Efficiency
        EffectiveStat = BaseStatValue;
    }
    else if (BaseStatValue <= 250.0f)
    {
        // Points 101-250: 50% Efficiency
        EffectiveStat = 100.0f + ((BaseStatValue - 100.0f) * 0.5f);
    }
    else
    {
        // Points 251+: 25% Efficiency
        // 100 (from first tier) + 75 (from 150 points at 50%) = 175 base before adding the 25% tier
        EffectiveStat = 175.0f + ((BaseStatValue - 250.0f) * 0.25f);
    }

    return EffectiveStat;
}

void UPlayerStatsComponent::CalculateDerivedStats()
{
    // 1. Calculate Health from Fortitude (+15 HP per point)
    float EffectiveFortitude = GetEffectiveStatValue(Fortitude);
    float BaseHP = 100.0f;
    MaxHealth = BaseHP + (EffectiveFortitude * 15.0f);

    // 2. Calculate Stamina from Vigor (+10 Stamina per point)
    float EffectiveVigor = GetEffectiveStatValue(Vigor);
    float BaseStamina = 100.0f;
    MaxStamina = BaseStamina + (EffectiveVigor * 10.0f);

    // 3. Calculate Awake Timer from Vigor (+5 mins per point)
    // Base is 16 In-Game Hours (960 minutes)
    float BaseAwakeMinutes = 960.0f;
    MaxAwakeTimerMinutes = BaseAwakeMinutes + (EffectiveVigor * 5.0f);    // FIXED: Removed stray [cite:163] artifact

    // Safety clamp to ensure current values don't exceed new maximums if stats are recalculated mid-game
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
    CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
    CurrentAwakeTimerMinutes = FMath::Clamp(CurrentAwakeTimerMinutes, 0.0f, MaxAwakeTimerMinutes);
}

// -------------------------------------------------------------------
// RESOURCE MODIFIERS
// -------------------------------------------------------------------

void UPlayerStatsComponent::ModifyHealth(float Amount)
{
    // Amount can be negative (damage) or positive (healing)
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);

    // Broadcast so the HUD knows to update the health bar
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UPlayerStatsComponent::ModifyStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);

    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

    // Note: This is where you will eventually check if CurrentStamina dropped too fast
    // to trigger the "Mana Burn" state.
}

void UPlayerStatsComponent::ModifyAwakeTimer(float Amount)
{
    // Amount will typically be negative as the timer drains over time
    CurrentAwakeTimerMinutes = FMath::Clamp(CurrentAwakeTimerMinutes + Amount, 0.0f, MaxAwakeTimerMinutes);

    OnAwakeTimerChanged.Broadcast(CurrentAwakeTimerMinutes);
}

float UPlayerStatsComponent::CalculateOutgoingPhysicalDamage(float BaseWeaponDamage, ETechniqueStyle TechStyle, EEnemyArchetype TargetArchetype) const
{

    // 0.1. Determine the Technique Multiplier based on matchups
    float TechniqueMultiplier = 1.0f; // Default if no condition is met

    switch (TechStyle)
    {

    case ETechniqueStyle::RiverStyx:
        // All-arounder style
         TechniqueMultiplier = 1.4f; // This number will be fun to play with
         break;


    case ETechniqueStyle::StoneBreaker:
        // Counters Turtle/Survivability types
        if (TargetArchetype == EEnemyArchetype::Survivability)
        {
            TechniqueMultiplier = 1.5f;
        }
        break;


    case ETechniqueStyle::GaleWind:
        // Counters Hit-and-Run/Evasive types
        if (TargetArchetype == EEnemyArchetype::Fighting)
        {
            TechniqueMultiplier = 1.5f;
        }
        break;

    case ETechniqueStyle::IronForm:
        // Counters Turtle/Survivability types
        if (TargetArchetype == EEnemyArchetype::Evasive)
        {
            TechniqueMultiplier = 1.5f;
        }
        break;

    case ETechniqueStyle::None:
    default:
        TechniqueMultiplier = 0.0f;   // If enemy is set to none, then it should be unbeatable
        break;
    }

    // 1. Get the Diminishing Returns applied value for Prowess
    float EffectiveProwess = GetEffectiveStatValue(PhysicalProwess);

    // 2. Convert Prowess into your GDD's percentage multiplier (+1.5% per point)
    float ProwessBonusPercentage = (EffectiveProwess * 1.5f) / 100.0f;

    // Note: We are keeping Passive% at 0.0f for now until you build the Unified Passive Pool
    float PassiveBonusPercentage = 0.0f;

    // 3. Execute the core formula: (Weapon Damage * (1 + Prowess% + Passive%)) * Technique Multiplier
    float TotalOutgoingDamage = (BaseWeaponDamage * (1.0f + ProwessBonusPercentage + PassiveBonusPercentage)) * TechniqueMultiplier;

    return TotalOutgoingDamage;
}