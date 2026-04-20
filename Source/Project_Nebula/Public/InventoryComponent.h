// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NebulaItemTypes.h"
#include "InventoryComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_NEBULA_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // The hard cap of slots the player has (can be expanded by backpacks later)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Config")
    int32 MaxInventorySlots = 20;

    // Pointer to your Master Item Data Table in the editor
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Config")
    TObjectPtr<UDataTable> MasterItemDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|State")
    TArray<FNebulaInventorySlot> InventorySlots;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Actions")
    bool AddItem(FName ItemID, int32 Quantity, int32& OutRemaining);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Actions")
    bool RemoveItemFromSlot(int32 SlotIndex, int32 QuantityToRemove);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Actions")
    void UseItem(int32 SlotIndex);

    // Scans the whole inventory to see if you have enough of a specific item
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Crafting")
    bool HasEnoughItems(FName ItemID, int32 RequiredAmount) const;

    // Searches the inventory and deletes the specified amount. 
    // Handles pulling from multiple different stacks if necessary!
    UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
    bool ConsumeItems(FName ItemID, int32 AmountToConsume);

protected:
    virtual void BeginPlay() override;

    // Helper to fetch data safely
    FNebulaItemData* GetItemData(FName ItemID) const;

public:
    // Memory bank for cooldowns: Maps the CooldownTag to its Expiration Time
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Cooldowns")
    TMap<FName, float> CooldownExpirations;

    // A helper function we can call from UI to show a spinning cooldown timer later!
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Cooldowns")
    float GetRemainingCooldown(FName CooldownTag) const;
};