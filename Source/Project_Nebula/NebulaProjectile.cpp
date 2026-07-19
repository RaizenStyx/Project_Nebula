// Fill out your copyright notice in the Description page of Project Settings.


#include "NebulaProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Project_NebulaCharacter.h"
#include "EnemyBase.h"
#include "MagicalDamageType.h"

// Sets default values
ANebulaProjectile::ANebulaProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 1. Setup Collision
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
    CollisionComp->OnComponentHit.AddDynamic(this, &ANebulaProjectile::OnHit);
    RootComponent = CollisionComp;

    // 2. Setup Niagara VFX
    VisualEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VisualEffect"));
    VisualEffect->SetupAttachment(RootComponent);

    // 3. Setup Movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 2000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.f; // Straight line magic missile

}

// Called when the game starts or when spawned
void ANebulaProjectile::BeginPlay()
{
	Super::BeginPlay();
    // Check the instigator to inherit their Active Element
    if (AProject_NebulaCharacter* Caster = Cast<AProject_NebulaCharacter>(GetInstigator()))
    {
        CurrentElement = Caster->ActiveElement;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Projectile Element: %s"), *UEnum::GetValueAsString(CurrentElement)));

        // Apply the correct Niagara System dynamically
        if (VisualEffect)
        {
            switch (CurrentElement)
            {
            case EElement::Fire:
                if (FireNiagaraSystem) VisualEffect->SetAsset(FireNiagaraSystem);
                break;
            case EElement::Water:
                if (WaterNiagaraSystem) VisualEffect->SetAsset(WaterNiagaraSystem);
                break;
            case EElement::Earth:
                if (EarthNiagaraSystem) VisualEffect->SetAsset(EarthNiagaraSystem);
                break;
            case EElement::Air:
                if (AirNiagaraSystem)
                {
                    VisualEffect->SetAsset(AirNiagaraSystem);
                    // Set Color A to White
                    VisualEffect->SetVariableLinearColor(FName("ColorA"), FLinearColor::White);
                    // Set Color B to Light Blue (RGB values: R=0.2, G=0.6, B=1.0, Alpha=1.0)
                    VisualEffect->SetVariableLinearColor(FName("ColorB"), FLinearColor(0.2f, 0.6f, 1.0f, 1.0f));
                }
                break;
            case EElement::None:
            default:
                if (DefaultNiagaraSystem) VisualEffect->SetAsset(DefaultNiagaraSystem);
                break;
            }
        }
    }
}

void ANebulaProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Ensure we don't damage ourselves or the player who cast it
    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetInstigator()))
    {
        // 1. Try to cast the actor we hit to base enemy class
        AEnemyBase* HitEnemy = Cast<AEnemyBase>(OtherActor);

        if (HitEnemy)
        {
            // 2. Apply Magical Damage using your specific damage type tag
            UGameplayStatics::ApplyDamage(
                HitEnemy,
                BaseDamage, 
                GetInstigatorController(),
                this,
                UMagicalDamageType::StaticClass() // <--- Magical damage tag
            );

            // 3. Apply the elemental status effect
            HitEnemy->ApplyStatusEffect(CurrentElement);
        }

        // 4. Destroy projectile on impact, whether it hit an enemy, a wall, or the floor
        Destroy();
    }
}

// Called every frame
void ANebulaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

