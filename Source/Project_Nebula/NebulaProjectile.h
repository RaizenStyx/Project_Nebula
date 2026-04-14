// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NebulaProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
//class UNiagaraComponent; 

UCLASS()
class PROJECT_NEBULA_API ANebulaProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANebulaProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    // The collision sphere
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComp;

    // Handles the flight math
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    // The visual effect of the spell
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    //UNiagaraComponent* VisualEffect;

    // Function that runs when the collision sphere hits something
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // Damage amount passed in when the spell is spawned
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Stats")
    float BaseDamage;
};
