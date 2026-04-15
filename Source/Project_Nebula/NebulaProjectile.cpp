// Fill out your copyright notice in the Description page of Project Settings.


#include "NebulaProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

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
	
}

void ANebulaProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Ensure we don't damage ourselves
    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetInstigator()))
    {
        // Apply damage using UE's built-in pipeline, which triggers TakeDamage() on the victim
        UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, GetInstigatorController(), this, UDamageType::StaticClass());

        // Destroy projectile on impact
        Destroy();
    }
}

// Called every frame
void ANebulaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

