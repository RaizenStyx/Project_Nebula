// Fill out your copyright notice in the Description page of Project Settings.


#include "NebulaProjectile.h"

// Sets default values
ANebulaProjectile::ANebulaProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANebulaProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANebulaProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Spell Triggered!"));
}

// Called every frame
void ANebulaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

