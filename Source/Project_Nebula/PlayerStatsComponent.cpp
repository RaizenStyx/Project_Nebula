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

    // Initialize Progression
    MainLevel = 1;
    CurrentMainXP = 0.0f;
    NextLevelMainXP = 100.0f; // Base requirement for Lv 1 -> 2 [cite: 122]

    ClassLevel = 0; // Starts at 0 until unlocked at Main Level 25 [cite: 100]
    CurrentClassXP = 0.0f;
    NextLevelClassXP = 50.0f; // Class requires exactly 50% of Main Level XP [cite: 123]

    ClassXPSplitPercentage = 0.5f; // Default to 50/50 split 
    UnspentStatPoints = 0;

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
    MaxAwakeTimerMinutes = BaseAwakeMinutes + (EffectiveVigor * 5.0f);   

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
        // Against the Nulltype, this technique should be extra effective. 
        if (TargetArchetype == EEnemyArchetype::NullType)
        {
            TechniqueMultiplier = 2.0f;
        }
        else // All-arounder style
        {
            TechniqueMultiplier = 1.4f; // This number will be fun to play with
        }
         break;


    case ETechniqueStyle::StoneBreaker:
        // Counters Turtle/Survivability types
        if (TargetArchetype == EEnemyArchetype::Survivability)
        {
            TechniqueMultiplier = 1.5f;
        }
        break;


    case ETechniqueStyle::GaleWind:
        // Counters Fighting types
        if (TargetArchetype == EEnemyArchetype::Fighting)
        {
            TechniqueMultiplier = 1.5f;
        }
        break;

    case ETechniqueStyle::IronForm:
        // Counters Hit-and-Run/Evasive types
        if (TargetArchetype == EEnemyArchetype::Evasive)
        {
            TechniqueMultiplier = 1.5f;
        }
        break;

    case ETechniqueStyle::None:
    default:
        TechniqueMultiplier = 1.0f;   // If techique style is None, it is basic and does not do extra damage.
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

float UPlayerStatsComponent::CalculateRequiredXP(int32 TargetLevel) const
{
    // The base requirement for Level 1 -> 2 is 100 XP [cite: 122]
    if (TargetLevel <= 1) return 100.0f;

    float RequiredXP = 100.0f;

    // Iteratively calculate the curve to avoid massive recursion overhead
    for (int32 i = 2; i <= TargetLevel; ++i)
    {
        if (i <= 25)
        {
            RequiredXP *= 2.0f; // Tier 1 [cite: 123]
        }
        else if (i <= 50)
        {
            RequiredXP *= 3.0f; // Tier 2 [cite: 123]
        }
        else if (i <= 75)
        {
            RequiredXP *= 4.0f; // Tier 3 [cite: 123]
        }
        else
        {
            RequiredXP *= 5.0f; // Tier 4 [cite: 123]
        }
    }

    return RequiredXP;
}

void UPlayerStatsComponent::AutoAllocateEssenceStats()
{
    // Apply the specific +2 Primary and +1 Secondary based on the chosen Essence
    switch (PlayerEssence)
    {
    case EEssenceType::Fighting:
        PhysicalProwess += 2.0f; // Primary [cite: 140]
        Agility += 1.0f;         // Secondary [cite: 140]
        break;

    case EEssenceType::Evasive:
        Agility += 2.0f;         // Primary [cite: 140]
        Synchronization += 1.0f; // Secondary [cite: 141]
        break;

    case EEssenceType::Survivability:
        Vigor += 2.0f;           // Primary [cite: 141]
        Fortitude += 1.0f;       // Secondary [cite: 141]
        break;

    case EEssenceType::None:
    default:
        // If no essence is chosen, maybe they just get raw unspent points, or nothing happens
        break;
    }

    // Recalculate derived stats (Health, Stamina, Awake Timer) because Fortitude or Vigor might have increased!
    CalculateDerivedStats();
}

void UPlayerStatsComponent::AddExperience(float RawXP)
{
    // PLACEHOLDER: Future Area Level scaling math will go here!
    float FinalXP = RawXP;

    // Calculate the distribution 
    float XPForClass = FinalXP * ClassXPSplitPercentage;
    float XPForMain = FinalXP - XPForClass;

    // Hard Cap Rule: Class Level cannot exceed Main Level 
    // If we are capped (or haven't unlocked a class yet at Lv 0), redirect all XP to Main [cite: 125]
    if (ClassLevel >= MainLevel || ClassLevel == 0)
    {
        XPForMain = FinalXP;
        XPForClass = 0.0f;
    }

    CurrentMainXP += XPForMain;
    CurrentClassXP += XPForClass;

    // Check for Main Level Up
    while (CurrentMainXP >= NextLevelMainXP)
    {
        CurrentMainXP -= NextLevelMainXP;
        MainLevel++;
        
        // Grant the 2 Free Allocation points for the player to spend manually 
        UnspentStatPoints += 2; 
        
        // Trigger the automatic +2/+1 Essence allocation
        AutoAllocateEssenceStats();

        NextLevelMainXP = CalculateRequiredXP(MainLevel);
        
        // Tell the UI we leveled up!
        OnMainLevelUp.Broadcast(MainLevel);
    }

    // Check for Class Level Up
    if (ClassLevel > 0)
    {
        while (CurrentClassXP >= NextLevelClassXP && ClassLevel < MainLevel)
        {
            CurrentClassXP -= NextLevelClassXP;
            ClassLevel++;
            // Class XP requirement is always exactly 50% of the Main Level requirement [cite: 123]
            NextLevelClassXP = CalculateRequiredXP(ClassLevel) * 0.5f;
        }
    }
}

bool UPlayerStatsComponent::SpendStatPoint(ENebulaStatType StatToUpgrade)
{
    // Fail-safe: Don't do anything if they have no points
    if (UnspentStatPoints <= 0)
    {
        return false;
    }

    // Add +1 to the requested stat
    switch (StatToUpgrade)
    {
    case ENebulaStatType::Prowess:
        PhysicalProwess += 1.0f;
        break;
    case ENebulaStatType::Synchronization:
        Synchronization += 1.0f;
        break;
    case ENebulaStatType::Agility:
        Agility += 1.0f;
        break;
    case ENebulaStatType::Fortitude:
        Fortitude += 1.0f;
        break;
    case ENebulaStatType::Vigor:
        Vigor += 1.0f;
        break;
    }

    // Deduct the point and recalculate health/stamina
    UnspentStatPoints -= 1;
    CalculateDerivedStats();

    return true;
}