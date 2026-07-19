// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"
#include "InventoryComponent.h"
#include "../Project_NebulaCharacter.h"
#include "Components/BoxComponent.h"

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

	// 1. Remember what we are currently wearing in the target slot
	FName OldItem = EquippedItems[TargetSlot];

	// 2. Put on the new item
	EquippedItems[TargetSlot] = NewItemID;

	// 3. Tell the Player Character to update 3D Meshes and Stats
	if (AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner()))
	{
		// This updates the mesh AND saves the bIsTwoHanded stat into CurrentWeaponInfo!
		PlayerChar->UpdateEquipmentVisuals(TargetSlot, NewItemID);

		// --- NEW: 2-HANDED CONFLICT LOGIC ---
		UInventoryComponent* InvComp = PlayerChar->FindComponentByClass<UInventoryComponent>();

		// RULE A: Equipping a 2-Handed Weapon to the Right Hand
		if (TargetSlot == EEquipmentSlot::WeaponR && PlayerChar->CurrentWeaponInfo.bIsTwoHanded)
		{
			// Check if there is something in the Left Hand getting in the way
			if (EquippedItems.Contains(EEquipmentSlot::WeaponL) && EquippedItems[EEquipmentSlot::WeaponL] != NAME_None)
			{
				// Unequip it!
				FName BumpedOffhand = UnequipItem(EEquipmentSlot::WeaponL);

				// Force the bumped left-hand item back into the inventory
				if (InvComp)
				{
					int32 Remainder = 0;
					InvComp->AddItem(BumpedOffhand, 1, Remainder);
					GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TEXT("Left-hand item returned to inventory to hold 2H weapon."));
				}
			}
		}
		// RULE B: Equipping an Off-Hand item while holding a 2-Handed Weapon
		else if (TargetSlot == EEquipmentSlot::WeaponL && PlayerChar->CurrentWeaponInfo.bIsTwoHanded)
		{
			// Check if there is a 2-Handed weapon in the Right Hand getting in the way
			if (EquippedItems.Contains(EEquipmentSlot::WeaponR) && EquippedItems[EEquipmentSlot::WeaponR] != NAME_None)
			{
				// Unequip it!
				FName BumpedMainhand = UnequipItem(EEquipmentSlot::WeaponR);

				// Force the bumped 2-Handed weapon back into the inventory
				if (InvComp)
				{
					int32 Remainder = 0;
					InvComp->AddItem(BumpedMainhand, 1, Remainder);
					GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TEXT("2H Weapon returned to inventory to equip off-hand item."));
				}
			}
		}
	}
	
	// --- NEW: Tell the UI this slot has a new item! ---
	// Note: Because EquipItem calls UnequipItem internally during the 2H check, 
	// the left hand will automatically broadcast its empty status during that step!
	OnEquipmentChanged.Broadcast(TargetSlot, NewItemID);

	// --- NEW: Update the Animation Stance! ---
	if (AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner()))
	{
		PlayerChar->DetermineWeaponStance();
	}

	// 4. Return the OldItem (from the primary slot) so the BP UI can swap it as usual
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

	OnEquipmentChanged.Broadcast(TargetSlot, NAME_None);

	// --- NEW: Update the Animation Stance! ---
	if (AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner()))
	{
		PlayerChar->DetermineWeaponStance();
	}

	return OldItem;
}
