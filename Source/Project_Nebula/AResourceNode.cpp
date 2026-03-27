// Fill out your copyright notice in the Description page of Project Settings.


#include "AResourceNode.h"

// Sets default values
AAResourceNode::AAResourceNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAResourceNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAResourceNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAResourceNode::Gather_Implementation(AActor* Gatherer, float DamageAmount)
{
    // 1. Subtract the damage your player dealt
    NodeHealth -= DamageAmount;

    // Optional: Play a "wobble" animation or particle effect here!

    // 2. Check if the tree/rock is destroyed
    if (NodeHealth <= 0.0f)
    {
        // 3. Spawn the item the player can walk over to collect
        if (DroppedItemClass)
        {
            FVector SpawnLocation = GetActorLocation();
            FRotator SpawnRotation = GetActorRotation();
            GetWorld()->SpawnActor<AActor>(DroppedItemClass, SpawnLocation, SpawnRotation);
        }

        // 4. Destroy the tree/rock
        Destroy();
    }
}