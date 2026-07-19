// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "Engine/DamageEvents.h"
#include "PhysicalDamageType.h"
#include "MagicalDamageType.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the stats component and attach it to this actor
	EnemyStats = CreateDefaultSubobject<UEnemyStatsComponent>(TEXT("EnemyStats"));

	// Default fallback
	EnemyArchetype = EEnemyArchetype::None;

}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Let the base character process the damage first
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (EnemyStats)
    {
        // Check the flag! Is this physical damage?
        if (DamageEvent.DamageTypeClass == UPhysicalDamageType::StaticClass())
        {
            // Do the math: Subtract defense from incoming damage
            ActualDamage = FMath::Max(ActualDamage - EnemyStats->PhysicalDefense, 0.0f);
        }
        // Is this magical damage?
        else if (DamageEvent.DamageTypeClass == UMagicalDamageType::StaticClass())
        {
            // Do the math: Subtract magical defense from incoming damage
            ActualDamage = FMath::Max(ActualDamage - EnemyStats->MagicalDefense, 0.0f);
        }

        // Apply the final calculated damage to the component's health pool
        EnemyStats->ModifyHealth(-ActualDamage);

        // TODO: Play hit reaction montage here!
    }

    return ActualDamage;
}

void AEnemyBase::ApplyStatusEffect(EElement IncomingElement)
{
    // A quick color/string setup for our debug prints
    FColor DebugColor = FColor::White;
    FString ElementName = TEXT("None");

    switch (IncomingElement)
    {
    case EElement::Fire:
        DebugColor = FColor::Red;
        ElementName = TEXT("Burning");
        // Future logic: Start taking tick damage
        break;
    case EElement::Water:
        DebugColor = FColor::Blue;
        ElementName = TEXT("Soaked");
        // Future logic: Reduce physical defense
        break;
    case EElement::Earth:
        DebugColor = FColor::Green; // Or brown if you use custom hex codes
        ElementName = TEXT("Staggered");
        // Future logic: Reduce movement speed
        break;
    case EElement::Air:
        DebugColor = FColor::Cyan;
        ElementName = TEXT("Lifted");
        // Future logic: Apply knockup/juggle
        break;
    default:
        break;
    }

    if (GEngine && IncomingElement != EElement::None)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, DebugColor, FString::Printf(TEXT("Enemy Status Applied: %s!"), *ElementName));
    }
}