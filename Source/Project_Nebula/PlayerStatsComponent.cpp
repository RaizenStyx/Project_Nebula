// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStatsComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Public/SkillManagerComponent.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize Base RPG Attributes
    BasePhysicalProwess = 0.0f;
    BaseSynchronization = 0.0f;
    BaseAgility = 0.0f;
    BaseFortitude = 0.0f;
    BaseVigor = 0.0f;

    // Set current to base
    PhysicalProwess = BasePhysicalProwess;
    Synchronization = BaseSynchronization;
    Agility = BaseAgility;
    Fortitude = BaseFortitude;
    Vigor = BaseVigor;

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

    ClassLevel = 0; // Starts at 0 until unlocked at Main Level 10 [cite: 100]
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
    // Cache percentages
    float StaminaPct = (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 1.0f;
    float ManaPct = (MaxMana > 0.0f) ? (CurrentMana / MaxMana) : 1.0f;

    // Health & Resource Totals
    float EffectiveFortitude = GetEffectiveStatValue(Fortitude);
    MaxHealth = 100.0f + (EffectiveFortitude * 15.0f);

    float EffectiveVigor = GetEffectiveStatValue(Vigor);
    MaxTotalResource = 100.0f + (EffectiveVigor * 10.0f);

    // Apply the dynamically stored split ratios
    MaxStamina = MaxTotalResource * StaminaSplitRatio;
    MaxMana = MaxTotalResource * ManaSplitRatio;

    // Reapply cached percentages
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
    CurrentStamina = FMath::Clamp(MaxStamina * StaminaPct, 0.0f, MaxStamina);
    CurrentMana = FMath::Clamp(MaxMana * ManaPct, 0.0f, MaxMana);

    // Time logic
    float BaseAwakeMinutes = 960.0f;
    MaxAwakeTimerMinutes = BaseAwakeMinutes + (EffectiveVigor * 5.0f);
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

void UPlayerStatsComponent::ModifyMana(float Amount)
{
    // Don't allow modification if the system isn't unlocked
    if (!bIsManaUnlocked) return;

    CurrentMana = FMath::Clamp(CurrentMana + Amount, 0.0f, MaxMana);

    OnManaChanged.Broadcast(CurrentMana, MaxMana);

    // FUTURE: Add your "Mana Burn" 5-second freeze logic here 
    // if (CurrentMana <= 0.0f) { ... }
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

float UPlayerStatsComponent::CalculateIncomingPhysicalDamage(float BaseDamage) const
{
    float EffectiveFortitude = GetEffectiveStatValue(Fortitude);

    // Combine base stat and equipment armor
    float TotalPhysicalDefense = EffectiveFortitude + PhysicalArmorDefense;

    // Armor formula: 100 / (100 + TotalArmor)
    float DamageMultiplier = 100.0f / (100.0f + TotalPhysicalDefense);

    return BaseDamage * DamageMultiplier;
}

float UPlayerStatsComponent::CalculateOutgoingMagicalDamage(float BaseSpellDamage)
{
    // 1. Get the Diminishing Returns applied value for Sync
    float EffectiveSync = GetEffectiveStatValue(Synchronization);

    // 2. Convert Sync into a percentage multiplier (+1.5% per point, same scaling as Prowess)
    float SyncBonusPercentage = (EffectiveSync * 1.5f) / 100.0f;

    // Note: Kept for when the Unified Passive Pool is connected
    float PassiveBonusPercentage = 0.0f;

    // 3. The flat magical advantage multiplier instead of physical Stances
    float MagicMultiplier = 1.25f;

    // 4. Execute the core formula
    float TotalOutgoingDamage = (BaseSpellDamage * (1.0f + SyncBonusPercentage + PassiveBonusPercentage)) * MagicMultiplier;

    return TotalOutgoingDamage;
}

float UPlayerStatsComponent::CalculateIncomingMagicalDamage(float BaseDamage) const
{
    // Vigor acts as Intellect/Magic Defense
    float EffectiveVigor = GetEffectiveStatValue(Vigor);

    // Combine base stat and equipment armor
    float TotalMagicalDefense = EffectiveVigor + MagicalArmorDefense;

    // Armor formula: 100 / (100 + TotalArmor)
    float DamageMultiplier = 100.0f / (100.0f + TotalMagicalDefense);

    return BaseDamage * DamageMultiplier;
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
        BasePhysicalProwess += 2.0f; // Primary [cite: 140]
        BaseAgility += 1.0f;         // Secondary [cite: 140]
        break;

    case EEssenceType::Evasive:
        BaseAgility += 2.0f;         // Primary [cite: 140]
        BaseSynchronization += 1.0f; // Secondary [cite: 141]
        break;

    case EEssenceType::Survivability:
        BaseVigor += 2.0f;           // Primary [cite: 141]
        BaseFortitude += 1.0f;       // Secondary [cite: 141]
        break;

    case EEssenceType::None:
    default:
        // If no essence is chosen, maybe they just get raw unspent points, or nothing happens
        break;
    }

    // Recalculate derived stats (Health, Stamina, Awake Timer) because Fortitude or Vigor might have increased!
    CalculateDerivedStats();
}

bool UPlayerStatsComponent::SpendStatPoint(ENebulaStatType StatToUpgrade)
{
    if (UnspentStatPoints <= 0) return false;

    // IMPORTANT: Add to the BASE stat, not the temporary one
    switch (StatToUpgrade)
    {
    case ENebulaStatType::Prowess: BasePhysicalProwess += 1.0f; break;
    case ENebulaStatType::Synchronization: BaseSynchronization += 1.0f; break;
    case ENebulaStatType::Agility: BaseAgility += 1.0f; break;
    case ENebulaStatType::Fortitude: BaseFortitude += 1.0f; break;
    case ENebulaStatType::Vigor: BaseVigor += 1.0f; break;
    }

    UnspentStatPoints -= 1;

    // Reapply class stats to instantly reflect the new base + class bonuses
    ApplyClassStats(CurrentClassTemplate, ClassLevel);

    return true;
}

void UPlayerStatsComponent::StudyMagicBook(float AwakeTimeCost, float ProgressAmount)
{
    // Prevent redundant studying if already unlocked
    if (bIsManaUnlocked) return;

    // 1. Deduct from the Awake Timer to simulate time passing while reading [cite: 44]
    ModifyAwakeTimer(-AwakeTimeCost);

    // 2. Advance the study progress
    MagicStudyProgress += ProgressAmount;

    // 3. Check for threshold. Passing 100.0f for ProgressAmount will make this instant for now.
    if (MagicStudyProgress >= 100.0f)
    {
        UnlockManaSystem(0.25f, 0.75f);
    }
}

void UPlayerStatsComponent::UnlockManaSystem(float InManaRatio, float InStaminaRatio)
{
    bIsManaUnlocked = true;

    ManaSplitRatio = InManaRatio;
    StaminaSplitRatio = InStaminaRatio;

    // Process the math with the new ratios
    CalculateDerivedStats();

    // Broadcast the changes so the UI catches the split immediately
    OnManaChanged.Broadcast(CurrentMana, MaxMana);
    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}


// New Add XP function. Old one may be removed after removing it from blueprints.
void UPlayerStatsComponent::AddXP(float RawXP)
{
    // Hard cap constraint: Class Level cannot exceed Main Level
    if (ClassLevel >= MainLevel)
    {
        CurrentMainXP += RawXP;
    }
    else
    {
        float ClassXP = RawXP * ClassXPSplitPercentage;
        float MainXP = RawXP - ClassXP;

        CurrentClassXP += ClassXP;
        CurrentMainXP += MainXP;

        CheckClassLevelUp();
    }

    CheckMainLevelUp();
}


void UPlayerStatsComponent::ApplyClassStats(UNebulaClassTemplate* NewClass, int32 InClassLevel)
{
    // 1. Reset to permanent baseline
    PhysicalProwess = BasePhysicalProwess;
    Synchronization = BaseSynchronization;
    Agility = BaseAgility;
    Fortitude = BaseFortitude;
    Vigor = BaseVigor;

    if (NewClass)
    {
        // 2. Determine Star Rank Multiplier
        int32 StatMultiplier = (NewClass->StarRank == ENebulaStarRank::ThreeStar) ? 2 : 1;
        int32 LevelBonus = StatMultiplier * InClassLevel;

        // 3. Apply class base additions and level scaling
        // (You can filter LevelBonus into a specific stat depending on Essence, or spread it)
        PhysicalProwess += NewClass->BaseStatAdditions.ProwessBonus;
        Synchronization += NewClass->BaseStatAdditions.SynchronizationBonus;
        Agility += NewClass->BaseStatAdditions.AgilityBonus;
        Fortitude += NewClass->BaseStatAdditions.FortitudeBonus;
        Vigor += NewClass->BaseStatAdditions.VigorBonus;

        if (NewClass->ResourceType == ENebulaResourceType::PureMana)
        {
            Synchronization += LevelBonus;
        }
        else
        {
            PhysicalProwess += LevelBonus;
        }
    }

    // 4. Recalculate derived pools with the newly calculated Vigor and Fortitude
    CalculateDerivedStats();
}


void UPlayerStatsComponent::CheckMainLevelUp()
{
    bool bLeveledUp = false;

    while (CurrentMainXP >= NextLevelMainXP)
    {
        CurrentMainXP -= NextLevelMainXP;
        MainLevel++;
        bLeveledUp = true;

        // Staged Exponential Curve Scaling
        if (MainLevel <= 25) NextLevelMainXP *= 2.0f;
        else if (MainLevel <= 50) NextLevelMainXP *= 3.0f;
        else if (MainLevel <= 75) NextLevelMainXP *= 4.0f;
        else NextLevelMainXP *= 5.0f;

        UnspentStatPoints += 2;
        AutoAllocateEssenceStats();
    }

    if (bLeveledUp)
    {
        OnMainLevelUp.Broadcast(MainLevel);
        NextLevelClassXP = NextLevelMainXP * 0.5f; // Class XP strictly maps to 50% of Main
        CalculateDerivedStats();
    }
}

void UPlayerStatsComponent::CheckClassLevelUp()
{
	if (!CurrentClassTemplate) return; // Safety check
    while (CurrentClassXP >= NextLevelClassXP && ClassLevel < MainLevel)
    {
        CurrentClassXP -= NextLevelClassXP;
        ClassLevel++;

        // Ensure synchronization of requirements
        NextLevelClassXP = NextLevelMainXP * 0.5f;

        // Apply the new stats internally
        if (CurrentClassTemplate)
        {
            ApplyClassStats(CurrentClassTemplate, ClassLevel);
        }

        OnClassLevelUp.Broadcast(ClassLevel);
    }

    // If overflow forces the class level to hit the cap, redirect remaining Class XP back into Main XP
    if (ClassLevel >= MainLevel && CurrentClassXP > 0.0f)
    {
        CurrentMainXP += CurrentClassXP;
        CurrentClassXP = 0.0f;
        CheckMainLevelUp(); // Trigger cascade check just in case the overflow forced a Main Level up
    }
}