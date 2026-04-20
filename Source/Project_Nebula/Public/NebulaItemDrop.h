#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NebulaItemDrop.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class PROJECT_NEBULA_API ANebulaItemDrop : public AActor
{
    GENERATED_BODY()

public:
    ANebulaItemDrop();

    // The Item ID from your DataTable (e.g., "BasicWood")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Data")
    FName ItemID;

    // How many of this item are in this drop?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Data")
    int32 Quantity = 1;

protected:
    virtual void BeginPlay() override;

    // The visual mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ItemMesh;

    // The pickup radius
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> PickupSphere;

    // The function that fires when the player walks into the sphere
    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};