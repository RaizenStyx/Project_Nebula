// Fill out your copyright notice in the Description page of Project Settings.


#include "NebulaChest.h"
#include "../Project_NebulaCharacter.h"
#include "InventoryComponent.h"

// Sets default values
ANebulaChest::ANebulaChest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void ANebulaChest::LootChest(AProject_NebulaCharacter* PlayerCharacter)
{
    if (!PlayerCharacter) return;

    // 1. Get the player's inventory
    UInventoryComponent* Inventory = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    bool bTookAnything = false;
    bool bStillHasItems = false;

    // 2. Loop through the chest contents BACKWARDS. 
    // (We loop backwards because we are removing items from the array as we empty them)
    for (int32 i = ChestContents.Num() - 1; i >= 0; --i)
    {
        FNebulaInventorySlot& Slot = ChestContents[i];

        // Skip empty or invalid slots
        if (Slot.IsEmpty())
        {
            ChestContents.RemoveAt(i);
            continue;
        }

        // 3. Try to give the item to the player
        int32 Remaining = 0;
        bool bSuccess = Inventory->AddItem(Slot.ItemID, Slot.Quantity, Remaining);

        if (bSuccess || Remaining < Slot.Quantity)
        {
            bTookAnything = true; // We successfully moved at least 1 item
        }

        // 4. Handle what is left over
        if (Remaining <= 0)
        {
            // The player took the whole stack! Remove it from the chest.
            ChestContents.RemoveAt(i);
        }
        else
        {
            // The player's inventory got full! Leave the remainder in the chest.
            Slot.Quantity = Remaining;
            bStillHasItems = true;
        }
    }

    // 5. Fire off our cosmetic Blueprint events
    if (bTookAnything)
    {
        OnChestLooted();
    }

    if (!bStillHasItems)
    {
        OnChestEmptied();
    }
}