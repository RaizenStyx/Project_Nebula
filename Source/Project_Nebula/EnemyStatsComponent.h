#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyStatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHealthChangedSignature, float, CurrentHealth, float, MaxHealth);



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_NEBULA_API UEnemyStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyStatsComponent();

protected:
    virtual void BeginPlay() override;

public:
 
    // -------------------------------------------------------------------
    // BASE STATS
    // -------------------------------------------------------------------

    // Base Attack scales damage output
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Attributes")
    float BaseAttack;

    // Directly reduces incoming physical damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Attributes")
    float PhysicalDefense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Attributes")
    float MagicalDefense;

    // -------------------------------------------------------------------
    // RESOURCE POOLS
    // -------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Resources")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Enemy Stats|Resources")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Resources")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Enemy Stats|Resources")
    float CurrentStamina;

    // -------------------------------------------------------------------
	// POISE SYSTEM
    // -------------------------------------------------------------------
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Resources")
    float MaxPoise;

    UPROPERTY(BlueprintReadOnly, Category = "Nebula Enemy Stats|Resources")
    float CurrentPoise;

    // -------------------------------------------------------------------
    // DELEGATES
    // -------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Nebula Enemy Stats|Events")
    FOnEnemyHealthChangedSignature OnHealthChanged;

    // -------------------------------------------------------------------
    // CORE FUNCTIONS
    // -------------------------------------------------------------------

    // Handles the final step of the damage formula: IncomingCalculatedDamage - BaseDefense
    UFUNCTION(BlueprintCallable, Category = "Nebula Enemy Stats|Methods")
    void ReceivePhysicalDamage(float IncomingCalculatedDamage);

    UFUNCTION(BlueprintCallable, Category = "Nebula Enemy Stats|Methods")
    void ModifyHealth(float Amount);
};