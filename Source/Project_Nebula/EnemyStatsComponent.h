#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyStatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

 //Defines the Rock-Paper-Scissors enemy types from the Bestiary
UENUM(BlueprintType)
    enum class EEnemyArchetype : uint8
{
    None            UMETA(DisplayName = "None"),
    Fighting        UMETA(DisplayName = "Fighting (Bruisers)"),
    Evasive         UMETA(DisplayName = "Evasive (Skirmishers)"),
    Survivability   UMETA(DisplayName = "Survivability (Tanks)"),
    NullType        UMETA(DisplayName = "Null (Gimmicks)") // Placeholder for boss/neutral types
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_NEBULA_API UEnemyStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyStatsComponent();

protected:
    virtual void BeginPlay() override;

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Attributes")
    EEnemyArchetype Archetype;
 
    // -------------------------------------------------------------------
    // BASE STATS
    // -------------------------------------------------------------------

    // Base Attack scales damage output
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Attributes")
    float BaseAttack;

    // Directly reduces incoming physical damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Enemy Stats|Attributes")
    float BaseDefense;

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