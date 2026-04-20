// Fill out your copyright notice in the Description page of Project Settings.


#include "NebulaItemDrop.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Project_NebulaCharacter.h"
#include "InventoryComponent.h"

// Sets default values
ANebulaItemDrop::ANebulaItemDrop()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // 1. Setup the Mesh and enable Physics so it falls to the ground!
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;
    ItemMesh->SetSimulatePhysics(true);
    ItemMesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // Bounces off the ground

    // 2. Setup the Pickup Sphere
    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    PickupSphere->SetupAttachment(RootComponent);
    // How close the player needs to get
    PickupSphere->SetSphereRadius(100.f); 
    PickupSphere->SetCollisionProfileName(TEXT("Trigger")); 

}

// Called when the game starts or when spawned
void ANebulaItemDrop::BeginPlay()
{
	Super::BeginPlay();

    // Bind the overlap event to our function
    if (PickupSphere)
    {
        PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ANebulaItemDrop::OnPickupOverlap);
    }
	
}

void ANebulaItemDrop::OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. Is the overlapping actor our player?
    AProject_NebulaCharacter* PlayerChar = Cast<AProject_NebulaCharacter>(OtherActor);
    if (!PlayerChar) return;

    // 2. Does the player have an inventory?
    UInventoryComponent* Inventory = PlayerChar->FindComponentByClass<UInventoryComponent>();
    if (!Inventory || ItemID.IsNone() || Quantity <= 0) return;

    // 3. Try to add the item!
    int32 Remaining;
    bool bSuccess = Inventory->AddItem(ItemID, Quantity, Remaining);

    if (bSuccess)
    {
        // We picked up everything. Destroy the drop!
        // TODO: Play a pickup sound/particle effect here
        Destroy();
    }
    else if (Remaining < Quantity)
    {
        // We picked up SOME of the items, but the inventory got full!
        // Update this drop's quantity so it stays on the ground with the leftover amount.
        Quantity = Remaining;
        // TODO: Play an "Inventory Full" error sound here
    }
}

