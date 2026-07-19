// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../EnemyStatsComponent.h"
#include "NebulaItemTypes.h"
#include "EnemyBase.generated.h"

UENUM(BlueprintType)
enum class EEnemyArchetype : uint8
{
	None UMETA(DisplayName = "None"),
	NullType UMETA(DisplayName = "Null Type"),
	Survivability UMETA(DisplayName = "Survivability"),
	Fighting UMETA(DisplayName = "Fighting"),
	Evasive UMETA(DisplayName = "Evasive")
};

UCLASS()
class PROJECT_NEBULA_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// 2. The Archetype Variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Setup")
	EEnemyArchetype EnemyArchetype;

	// 3. The Stats Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UEnemyStatsComponent* EnemyStats;

	// 4. The Damage Override
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 5. Function to handle incoming status effects
	UFUNCTION(BlueprintCallable, Category = "Combat|Status Effects")
	void ApplyStatusEffect(EElement IncomingElement);
};
