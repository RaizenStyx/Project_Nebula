// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "AResourceNode.generated.h"

UCLASS()
class PROJECT_NEBULA_API AAResourceNode : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAResourceNode();

	UPROPERTY(EditAnywhere, Category = "Resource")
	float NodeHealth = 100.0f;

	// The item that drops when destroyed (You'll make this class later for the walk-over pickup)
	UPROPERTY(EditAnywhere, Category = "Resource")
	TSubclassOf<AActor> DroppedItemClass;

	// 3. Override the Interface function! MUST have _Implementation
	virtual void Gather_Implementation(AActor* Gatherer, float DamageAmount) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
