// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponDataTypes.h" 
#include "EnemyStatsComponent.h"
#include "PlayerStatsComponent.generated.h"


//// Delegates for UI and other systems to listen to
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAwakeTimerChangedSignature, float, CurrentAwakeTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpSignature, int32, NewLevel);


// Defines the player's chosen Essence
UENUM(BlueprintType)
enum class EEssenceType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fighting        UMETA(DisplayName = "Fighting (Æsir's Eye)"),
    Evasive         UMETA(DisplayName = "Evasive (The Void Flicker)"),
    Survivability   UMETA(DisplayName = "Survivability (Stone Wall)")
};

UENUM(BlueprintType)
enum class ENebulaStatType : uint8
{
    Prowess         UMETA(DisplayName = "Physical Prowess"),
    Synchronization UMETA(DisplayName = "Synchronization"),
    Agility         UMETA(DisplayName = "Agility"),
    Fortitude       UMETA(DisplayName = "Fortitude"),
    Vigor           UMETA(DisplayName = "Vigor")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_NEBULA_API UPlayerStatsComponent : public UActorComponent  
{
    GENERATED_BODY()

public:
    UPlayerStatsComponent();    

protected:
    virtual void BeginPlay() override;

public:

    //virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // -------------------------------------------------------------------
    // PRIMARY ATTRIBUTES
    // -------------------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Primary")
    float PhysicalProwess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Primary")
    float Synchronization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Primary")
    float Agility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Primary")
    float Fortitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Primary")
    float Vigor;

    // -------------------------------------------------------------------
    // RESOURCE POOLS (DERIVED STATS)
    // -------------------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Resources")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Resources")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Resources")
    float CurrentStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Resources")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Resources")
    float CurrentAwakeTimerMinutes;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Resources")
    float MaxAwakeTimerMinutes;

    // -------------------------------------------------------------------
    // DELEGATES
    // -------------------------------------------------------------------
    
    UPROPERTY(BlueprintAssignable, Category = "Nebula Stats|Events")
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Nebula Stats|Events")
    FOnStaminaChangedSignature OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Nebula Stats|Events")
    FOnAwakeTimerChangedSignature OnAwakeTimerChanged;

    // -------------------------------------------------------------------
    // CORE FUNCTIONS
    // -------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    void CalculateDerivedStats();

    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    void ModifyHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    void ModifyStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    void ModifyAwakeTimer(float Amount);   

    UFUNCTION(BlueprintPure, Category = "Nebula Stats|Methods")
    float GetEffectiveStatValue(float BaseStatValue) const;

    // Calculates outgoing damage based on Prowess and Weapon Stats
    UFUNCTION(BlueprintPure, Category = "Nebula Stats|Combat")
    float CalculateOutgoingPhysicalDamage(float BaseWeaponDamage, ETechniqueStyle TechniqueStyle, EEnemyArchetype Archetype) const;

    UFUNCTION(BlueprintPure, Category = "Nebula Stats|Combat")
    float CalculateIncomingPhysicalDamage(float BaseDamage) const;

    // -------------------------------------------------------------------
    // PROGRESSION & XP
    // -------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    int32 MainLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    float CurrentMainXP;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    float NextLevelMainXP;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    int32 ClassLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    float CurrentClassXP;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    float NextLevelClassXP;

    // Defines how XP is split. 0.0 = 100% Main / 0% Class. 0.5 = 50/50. 1.0 = 0% Main / 100% Class.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Progression")
    float ClassXPSplitPercentage;

    // Used for your future "System Status" screen UI
    UPROPERTY(BlueprintReadOnly, Category = "Nebula Stats|Progression")
    int32 UnspentStatPoints;

    UPROPERTY(BlueprintAssignable, Category = "Nebula Stats|Events")
    FOnLevelUpSignature OnMainLevelUp;

    // The Essence chosen at the start of the game
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Progression")
    EEssenceType PlayerEssence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Stats|Progression")
    ENebulaStatType StatType;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    void AddExperience(float RawXP);

    UFUNCTION(BlueprintPure, Category = "Nebula Stats|Methods")
    float CalculateRequiredXP(int32 TargetLevel) const;

    // Handles the automatic +2/+1 stat distribution on level up
    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    void AutoAllocateEssenceStats();

    // Spends one unallocated point on the chosen stat. Returns true if successful.
    UFUNCTION(BlueprintCallable, Category = "Nebula Stats|Methods")
    bool SpendStatPoint(ENebulaStatType StatToUpgrade);

};