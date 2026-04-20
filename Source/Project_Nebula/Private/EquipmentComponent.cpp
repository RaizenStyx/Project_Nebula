// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"
#include "../Project_NebulaCharacter.h"

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize our Map with empty slots so the UI can safely read them immediately
	EquippedItems.Add(EEquipmentSlot::Head, NAME_None);
	EquippedItems.Add(EEquipmentSlot::Chest, NAME_None);
	EquippedItems.Add(EEquipmentSlot::ArmR, NAME_None);
	EquippedItems.Add(EEquipmentSlot::ArmL, NAME_None);
	EquippedItems.Add(EEquipmentSlot::Legs, NAME_None);
	EquippedItems.Add(EEquipmentSlot::Feet, NAME_None);
	EquippedItems.Add(EEquipmentSlot::WeaponR, NAME_None);
	EquippedItems.Add(EEquipmentSlot::WeaponL, NAME_None);
	
}

FName UEquipmentComponent::EquipItem(EEquipmentSlot TargetSlot, FName NewItemID)
{
	if (TargetSlot == EEquipmentSlot::None) return NAME_None;

	// 1. Remember what we are currently wearing
	FName OldItem = EquippedItems[TargetSlot];

	// 2. Put on the new item
	EquippedItems[TargetSlot] = NewItemID;

	// 3. Tell the Player Character to update 3D Meshes and Stats based on the TargetSlot!
	// --- NEW: Tell the Character to update its 3D model! ---
	if (AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner()))
	{
		PlayerChar->UpdateEquipmentVisuals(TargetSlot, NewItemID);

		// Note: You can also update stats here!
		// Example: PlayerChar->RecalculateStats();
	}

	// 4. Return the old item so the inventory knows what to put back in our pockets
	return OldItem;
}

FName UEquipmentComponent::UnequipItem(EEquipmentSlot TargetSlot)
{
	if (TargetSlot == EEquipmentSlot::None) return NAME_None;

	FName OldItem = EquippedItems[TargetSlot];
	EquippedItems[TargetSlot] = NAME_None;

	// --- NEW: Tell the Character to clear its 3D model! ---
	if (AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner()))
	{
		// Passing NAME_None clears the mesh
		PlayerChar->UpdateEquipmentVisuals(TargetSlot, NAME_None);
	}

	return OldItem;
}
