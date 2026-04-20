// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NebulaItemTypes.h"
#include "EquipmentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_NEBULA_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipmentComponent();

	// The core memory bank: Maps a Body Slot to the ItemID currently worn there
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TMap<EEquipmentSlot, FName> EquippedItems;

	// Equips an item and returns the ID of the item that was previously in that slot (if any)
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FName EquipItem(EEquipmentSlot TargetSlot, FName NewItemID);

	// Unequips whatever is in the slot and returns its ID
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FName UnequipItem(EEquipmentSlot TargetSlot);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//public:	
//	// Called every frame
//	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
