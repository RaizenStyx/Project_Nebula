// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "NebulaItemTypes.h"
#include "../Project_NebulaCharacter.h"
#include "../PlayerStatsComponent.h"
#include "SkillManagerComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Default capacity
	MaxInventorySlots = 20;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize our array with empty slots based on our Max capacity.
	// This is crucial so Slot 0 is always the first slot, Slot 19 is the last.
	InventorySlots.SetNum(MaxInventorySlots);
	
}

// -------------------------------------------------------------------
// HELPER: Get Data from the Master Table
// -------------------------------------------------------------------
FNebulaItemData* UInventoryComponent::GetItemData(FName ItemID) const
{
	if (!MasterItemDataTable || ItemID.IsNone())
	{
		return nullptr;
	}

	// "Context" is just a string Unreal uses for error logging if it fails
	FString ContextString = TEXT("Inventory Item Context");
	return MasterItemDataTable->FindRow<FNebulaItemData>(ItemID, ContextString);
}

// -------------------------------------------------------------------
// ACTION: Add an item, handling stack limits
// -------------------------------------------------------------------
bool UInventoryComponent::AddItem(FName ItemID, int32 Quantity, int32& OutRemaining)
{
    OutRemaining = Quantity;

    // 1. Get the rules for this item
    FNebulaItemData* ItemData = GetItemData(ItemID);
    if (!ItemData || Quantity <= 0)
    {
        return false;
    }

    int32 MaxStack = ItemData->MaxStackSize;

    // 2. Pass One: Try to add to existing, non-full stacks
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID == ItemID && InventorySlots[i].Quantity < MaxStack)
        {
            int32 SpaceLeftInSlot = MaxStack - InventorySlots[i].Quantity;
            int32 AmountToAdd = FMath::Min(SpaceLeftInSlot, OutRemaining);

            InventorySlots[i].Quantity += AmountToAdd;
            OutRemaining -= AmountToAdd;

            if (OutRemaining <= 0)
            {
                return true; // We successfully added everything
            }
        }
    }

    // 3. Pass Two: We still have items to add, look for totally empty slots
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].IsEmpty())
        {
            InventorySlots[i].ItemID = ItemID;

            int32 AmountToAdd = FMath::Min(MaxStack, OutRemaining);
            InventorySlots[i].Quantity = AmountToAdd;
            OutRemaining -= AmountToAdd;

            if (OutRemaining <= 0)
            {
                return true; // Successfully added the rest
            }
        }
    }

    // If we reach here, OutRemaining > 0, meaning the inventory is full!
    // The player's pockets are stuffed.
    return false;
}

// -------------------------------------------------------------------
// ACTION: Remove items
// -------------------------------------------------------------------
bool UInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 QuantityToRemove)
{
    // Make sure they aren't asking for a slot that doesn't exist
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
    {
        return false;
    }

    if (QuantityToRemove <= 0)
    {
        return false;
    }

    // Deduct the quantity
    InventorySlots[SlotIndex].Quantity -= QuantityToRemove;

    // If we used the last one, fully clear the slot
    if (InventorySlots[SlotIndex].Quantity <= 0)
    {
        InventorySlots[SlotIndex].Clear();
    }

    return true;
}

void UInventoryComponent::UseItem(int32 SlotIndex)
{
    // 1. Safety check: Is this a valid slot and does it actually have an item?
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
    {
        return;
    }

    // 2. Fetch the item rules from the Master Database
    FName ID = InventorySlots[SlotIndex].ItemID;
    FNebulaItemData* ItemData = GetItemData(ID);
    if (!ItemData) return;

    // 3. Get the player character
    AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner());
    if (!PlayerChar) return;

    bool bItemWasUsed = false;

    // 4. Route the logic based on what kind of item it is
    switch (ItemData->ItemType)
    {
    case ENebulaItemType::Consumable:

        // Is it a Skill Orb?
        if (ItemData->GrantedSkill != nullptr)
        {
            if (USkillManagerComponent* SkillManager = PlayerChar->FindComponentByClass<USkillManagerComponent>())
            {
                // Create the skill object and add it to the Unlocked Passives pool you made!
                UNebulaSkillBase* NewSkill = NewObject<UNebulaSkillBase>(PlayerChar, ItemData->GrantedSkill);
                SkillManager->UnlockedPassives.Add(NewSkill);

                // TODO: Fire off a cool particle effect/sound here later
                bItemWasUsed = true;
            }
        }
        // Otherwise, it's a normal potion/food
        else
        {
            if (UPlayerStatsComponent* StatsComp = PlayerChar->FindComponentByClass<UPlayerStatsComponent>())
            {
                // Uncomment these once your Heal/Restore stamina functions exist in UPlayerStatsComponent!
                // StatsComp->RestoreHealth(ItemData->HealthRestoreAmount);
                // StatsComp->RestoreStamina(ItemData->StaminaRestoreAmount);
                bItemWasUsed = true;
            }
        }
        break;

    case ENebulaItemType::Equipment:
        // TODO: Tell your EquipmentComponent to put this on
        break;

    case ENebulaItemType::Resource:
    case ENebulaItemType::Deployable:
    case ENebulaItemType::Quest:
        // As we discussed, these likely shouldn't be "Used" directly from the pocket UI.
        // Resources/Deployables are used at workbenches.
        // Quests are handled by NPC dialogue.
        // So we do nothing, and bItemWasUsed remains false.
        break;
    }

    // 5. If we successfully used it, consume 1 from the stack!
    if (bItemWasUsed)
    {
        RemoveItemFromSlot(SlotIndex, 1);
    }
}
