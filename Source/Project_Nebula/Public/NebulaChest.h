// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NebulaItemTypes.h"
#include "NebulaChest.generated.h"

UCLASS()
class PROJECT_NEBULA_API ANebulaChest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANebulaChest();

	// The designer (you) will add items and quantities to this array in the UE Editor!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Data")
	TArray<FNebulaInventorySlot> ChestContents;

	// We will call this from your existing Blueprint interaction event
	UFUNCTION(BlueprintCallable, Category = "Chest Interaction")
	void LootChest(class AProject_NebulaCharacter* PlayerCharacter);

protected:
	// Blueprint Events you can use to play sounds, particles, or open the lid animation!
	UFUNCTION(BlueprintImplementableEvent, Category = "Chest Events")
	void OnChestLooted();

	// Fires when every single item is removed from the chest
	UFUNCTION(BlueprintImplementableEvent, Category = "Chest Events")
	void OnChestEmptied();

};
