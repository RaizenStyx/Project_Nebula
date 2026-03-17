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
};