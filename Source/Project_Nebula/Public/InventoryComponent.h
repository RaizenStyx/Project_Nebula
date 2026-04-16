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

protected:
    virtual void BeginPlay() override;

    // Helper to fetch data safely
    FNebulaItemData* GetItemData(FName ItemID) const;
};