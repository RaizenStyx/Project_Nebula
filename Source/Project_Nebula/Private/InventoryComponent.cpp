// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "NebulaItemTypes.h"
#include "EquipmentComponent.h"
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

// -------------------------------------------------------------------
// ACTION: Add items
// -------------------------------------------------------------------

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

    // --- NEW COOLDOWN CHECK ---
    if (!ItemData->CooldownTag.IsNone())
    {
        float RemainingTime = GetRemainingCooldown(ItemData->CooldownTag);
        if (RemainingTime > 0.f)
        {
            // The item is on cooldown! We abort the use.
            // TODO: In the future, we can send a message to the HUD here saying "Item on Cooldown!"
            UE_LOG(LogTemp, Warning, TEXT("%s is on cooldown for %f more seconds!"), *ItemData->CooldownTag.ToString(), RemainingTime);
            return;
        }
    }

    // 3. Get the player character
    AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(GetOwner());
    if (!PlayerChar) return;

    bool bItemWasUsed = false;

    // 4. Route the logic based on what kind of item it is
    switch (ItemData->ItemType)
    {
    case ENebulaItemType::Consumable:

        // 1. --- THE PROGRESSION ITEM INTERCEPT ---
        if (ID == FName("Item_StudyBook"))
        {
            if (UPlayerStatsComponent* StatsComp = PlayerChar->FindComponentByClass<UPlayerStatsComponent>())
            {
                // Optional: Prevent them from studying it again if it's already unlocked
                if (StatsComp->bIsManaUnlocked)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("You have already mastered this knowledge."));
                    return; // Abort entirely
                }

                // Drain 60 mins of Awake Time, grant 100% progress (instant unlock for now)
                StatsComp->StudyMagicBook(60.0f, 100.0f);

                // Notice we in entionally DO NOT set bItemWasUsed = true here.
                // This ensures the inventory system won't delete the book from the slot!

                // We can break out of the switch early since we handled the item.
                break;
            }
        }

        // 2. Is it a Skill Orb?
        else if (ItemData->GrantedSkill != nullptr)
        {
            if (USkillManagerComponent* SkillManager = PlayerChar->FindComponentByClass<USkillManagerComponent>())
            {
                // Attempt to learn and equip it. This function handles Active/Passive routing AND Auto-Slotting!
                bool bSuccess = SkillManager->LearnSkillFromItem(ItemData->GrantedSkill, ItemData->AllowedClasses);

                if (bSuccess)
                {
                    // This tells the end of the UseItem function to consume 1 from the stack!
                    bItemWasUsed = true;
                    UE_LOG(LogTemp, Log, TEXT("Successfully learned and equipped new skill!"));
                }
                else
                {
                    // Fire UI notification: "Cannot learn this skill right now" (or "Hotbar Full")
                    UE_LOG(LogTemp, Warning, TEXT("Failed to learn skill - restricted class or hotbar is full."));
                }
            }
        }

        // Inside UInventoryComponent::UseItem

        // 3. Is it a Class Token?
        else if (ItemData->GrantedClass != nullptr)
        {
            if (USkillManagerComponent* SkillManager = PlayerChar->FindComponentByClass<USkillManagerComponent>())
            {
                SkillManager->EquipNewClass(ItemData->GrantedClass, 1);

                bItemWasUsed = true;
                UE_LOG(LogTemp, Log, TEXT("Player consumed token and changed classes!"));
            }
        }

        // 3. Otherwise, it's a normal potion/food
        else
        {
            if (UPlayerStatsComponent* StatsComp = PlayerChar->FindComponentByClass<UPlayerStatsComponent>())
            {
                // Uncomment these once your Heal/Restore stamina functions exist in UPlayerStatsComponent!
                // StatsComp->ModifyHealth(ItemData->HealthRestoreAmount);
                // StatsComp->ModifyStamina(ItemData->StaminaRestoreAmount);
                bItemWasUsed = true;
            }
        }
        break;

    case ENebulaItemType::Equipment:
    {
        // 1. Get our new Equipment Component
        UEquipmentComponent* EquipComp = PlayerChar->FindComponentByClass<UEquipmentComponent>();
        if (!EquipComp) return;

        // 2. Find out where this item is supposed to go (Head, Chest, WeaponR?)
        EEquipmentSlot TargetSlot = ItemData->EquipSlot;
        if (TargetSlot == EEquipmentSlot::None) return; // Database error: Forgot to set the slot in editor!

        // 3. Equip the new item, and catch whatever fell off the character!
        FName NewEquipID = ItemData->ItemID;
        FName OldEquipID = EquipComp->EquipItem(TargetSlot, NewEquipID);

        // 4. Handle the Inventory Swapping
        if (!OldEquipID.IsNone())
        {
            // Put the old item perfectly into the pocket space we just opened up
            InventorySlots[SlotIndex].ItemID = OldEquipID;
            InventorySlots[SlotIndex].Quantity = 1;

            bItemWasUsed = false; // Prevent the bottom of UseItem from clearing the slot
        }
        else
        {
            // The slot was empty, we just put it on. Clear the pocket slot!
            bItemWasUsed = true;
        }
    }
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
    // --- APPLY COOLDOWN & CONSUME ---
    if (bItemWasUsed)
    {
        // If it had a cooldown tag, calculate the expiration time and save it to the Map
        if (!ItemData->CooldownTag.IsNone() && ItemData->CooldownDuration > 0.f)
        {
            float ExpiryTime = GetWorld()->GetTimeSeconds() + ItemData->CooldownDuration;
            CooldownExpirations.Add(ItemData->CooldownTag, ExpiryTime);
        }

        RemoveItemFromSlot(SlotIndex, 1);
    }
}

float UInventoryComponent::GetRemainingCooldown(FName CooldownTag) const
{
    if (CooldownTag.IsNone() || !CooldownExpirations.Contains(CooldownTag))
    {
        return 0.f; // No cooldown
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ExpirationTime = CooldownExpirations[CooldownTag];

    if (CurrentTime < ExpirationTime)
    {
        return ExpirationTime - CurrentTime; // Return seconds remaining
    }

    return 0.f; // Cooldown has finished
}

// -------------------------------------------------------------------
// CRAFTING HELPER: Check if player has enough materials
// -------------------------------------------------------------------
bool UInventoryComponent::HasEnoughItems(FName ItemID, int32 RequiredAmount) const
{
    if (RequiredAmount <= 0) return true;

    int32 TotalFound = 0;

    // Loop through pockets and count everything up
    for (const FNebulaInventorySlot& Slot : InventorySlots)
    {
        if (Slot.ItemID == ItemID)
        {
            TotalFound += Slot.Quantity;
            if (TotalFound >= RequiredAmount)
            {
                return true; // We found enough, stop searching!
            }
        }
    }

    return false; // Not enough materials
}

// -------------------------------------------------------------------
// CRAFTING HELPER: Consume materials across multiple stacks
// -------------------------------------------------------------------
bool UInventoryComponent::ConsumeItems(FName ItemID, int32 AmountToConsume)
{
    // Safety First: Don't consume anything if they don't have enough total!
    if (!HasEnoughItems(ItemID, AmountToConsume))
    {
        return false;
    }

    int32 AmountLeftToConsume = AmountToConsume;

    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID == ItemID)
        {
            // If this stack has MORE or EXACTLY what we need to finish the cost
            if (InventorySlots[i].Quantity >= AmountLeftToConsume)
            {
                InventorySlots[i].Quantity -= AmountLeftToConsume;

                if (InventorySlots[i].Quantity <= 0)
                {
                    InventorySlots[i].Clear(); // Stack empty, clear it
                }

                return true; // We are done consuming!
            }
            // If this stack has SOME, but we still need more
            else
            {
                AmountLeftToConsume -= InventorySlots[i].Quantity;
                InventorySlots[i].Clear(); // We ate this whole stack, clear it and keep looping
            }
        }
    }

    return true;
}