// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetLockComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "../Project_NebulaCharacter.h"

// Sets default values for this component's properties
UTargetLockComponent::UTargetLockComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1. Get the Actor this component is attached to, and cast it to your Character class
	OwnerCharacter = Cast<AProject_NebulaCharacter>(GetOwner());

	// 2. If successful, get the Player Controller
	if (OwnerCharacter)
	{
		PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	}
}

void UTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentTarget)
	{
		// 1. Break lock if they run too far away
		float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Distance > MaxLockDistance)
		{
			BreakLock();
			return;
		}

		// 2. Smoothly rotate camera
		UpdateCameraLook(DeltaTime);

		// --- NEW: Update Reticle Position ---
		if (ReticleWidget && ReticleWidget->IsInViewport())
		{
			FVector2D ScreenPosition;
			// Project the 3D location of the enemy to 2D screen space
			bool bIsOnScreen = PlayerController->ProjectWorldLocationToScreen(CurrentTarget->GetActorLocation(), ScreenPosition);

			if (bIsOnScreen)
			{
				// Center the widget on the target
				ReticleWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
				ReticleWidget->SetPositionInViewport(ScreenPosition);
			}
		}
	}
}

void UTargetLockComponent::UpdateCameraLook(float DeltaTime)
{
	if (!PlayerController) return;

	// Calculate the direction from the camera to the target
	FVector CameraLoc = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
	// Aim slightly lower than the target's origin so the enemy is centered on screen, not at the very bottom
	FVector TargetLoc = CurrentTarget->GetActorLocation() - FVector(0, 0, 100.f);

	FRotator TargetRotation = (TargetLoc - CameraLoc).Rotation();
	FRotator CurrentRotation = PlayerController->GetControlRotation();

	// Smoothly interpolate
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, CameraInterpSpeed);

	PlayerController->SetControlRotation(NewRotation);
}

void UTargetLockComponent::FindBestTarget()
{
	if (!OwnerCharacter) return;

	FVector CameraLoc = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
	FVector CameraForward = OwnerCharacter->GetFollowCamera()->GetForwardVector();

	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(LockSweepRadius);

	// Sweep for pawns (you might want a custom trace channel for "Enemies")
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, CameraLoc, CameraLoc + (CameraForward * MaxLockDistance),
		FQuat::Identity, ECC_Pawn, Sphere
	);

	AActor* BestTarget = nullptr;
	float HighestDotProduct = -1.0f; // Range is -1 (behind) to 1 (perfectly centered)

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != OwnerCharacter /* Add check if IsEnemy() */)
			{
				FVector DirToTarget = (HitActor->GetActorLocation() - CameraLoc).GetSafeNormal();
				float Dot = FVector::DotProduct(CameraForward, DirToTarget);

				// If it's the most centered target so far, save it
				// (Optional: require Dot > 0.5f so you don't lock onto things behind you)
				if (Dot > HighestDotProduct && Dot > 0.5f)
				{
					HighestDotProduct = Dot;
					BestTarget = HitActor;
				}
			}
		}
	}

	if (BestTarget)
	{
		CurrentTarget = BestTarget;
		CurrentTarget->OnDestroyed.AddDynamic(this, &UTargetLockComponent::OnTargetDestroyed);

		// TargetLockComponent.cpp inside FindBestTarget()
		if (TargetReticleClass)
		{
			// Create the widget if it doesn't exist yet
			if (!ReticleWidget)
			{
				ReticleWidget = CreateWidget<UUserWidget>(PlayerController, TargetReticleClass);
			}

			// Add it to the player's screen
			if (ReticleWidget && !ReticleWidget->IsInViewport())
			{
				ReticleWidget->AddToViewport();
			}
		}

		// Adjust Character Movement for Strafe mechanics
		OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		OwnerCharacter->bUseControllerRotationYaw = true;
	}
}

void UTargetLockComponent::BreakLock()
{
	if (CurrentTarget)
	{
		// Unbind the delegate so we don't get crashes
		CurrentTarget->OnDestroyed.RemoveDynamic(this, &UTargetLockComponent::OnTargetDestroyed);
		CurrentTarget = nullptr;

		// TODO: Hide Target Reticle Widget here

		// Revert Character Movement back to free-flow Action RPG style
		OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
		OwnerCharacter->bUseControllerRotationYaw = false;

		// Remove Target Reticle Widget inside BreakLock()
		if (ReticleWidget && ReticleWidget->IsInViewport())
		{
			ReticleWidget->RemoveFromParent();
		}
	}
}

void UTargetLockComponent::OnTargetDestroyed(AActor* DestroyedActor)
{
	BreakLock();
}

void UTargetLockComponent::ToggleTargetLock()
{
	if (CurrentTarget)
	{
		BreakLock();
	}
	else
	{
		FindBestTarget();
	}
}

void UTargetLockComponent::SwitchTarget(float DirectionAxis)
{
	// Don't do anything if we aren't locked on, or the stick isn't pushed hard enough
	//if (!CurrentTarget || FMath::Abs(DirectionAxis) < 0.5f) return;
	if (!CurrentTarget || DirectionAxis == 0.0f) return;

	FVector CameraLoc = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
	FVector CameraRight = OwnerCharacter->GetFollowCamera()->GetRightVector();

	// Vector pointing from camera to our current enemy
	FVector DirToCurrent = (CurrentTarget->GetActorLocation() - CameraLoc).GetSafeNormal();

	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(LockSweepRadius);

	// Sweep again to find everyone nearby
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, CameraLoc, CameraLoc + (DirToCurrent * MaxLockDistance),
		FQuat::Identity, ECC_Pawn, Sphere
	);

	AActor* BestNewTarget = nullptr;
	float ClosestAngle = 9999.0f; // Start impossibly high

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* EvalTarget = Hit.GetActor();

			// Ignore ourselves and our current target
			if (EvalTarget == OwnerCharacter || EvalTarget == CurrentTarget) continue;

			FVector DirToEval = (EvalTarget->GetActorLocation() - CameraLoc).GetSafeNormal();

			// 1. Is this enemy to the Right or Left of our camera?
			float RightDot = FVector::DotProduct(CameraRight, DirToEval);
			bool bIsRight = RightDot > 0.0f;

			// 2. Does their position match the direction the player flicked the stick?
			// (DirectionAxis > 0 means player flicked Right)
			if ((DirectionAxis > 0.0f && bIsRight) || (DirectionAxis < 0.0f && !bIsRight))
			{
				// 3. How large is the angle between the current target and this new one?
				float AngleDifference = FMath::Acos(FVector::DotProduct(DirToCurrent, DirToEval));

				if (AngleDifference < ClosestAngle)
				{
					ClosestAngle = AngleDifference;
					BestNewTarget = EvalTarget;
				}
			}
		}
	}

	// If we found a valid enemy in the direction we pushed, swap to them!
	if (BestNewTarget)
	{
		// Unbind from old target
		CurrentTarget->OnDestroyed.RemoveDynamic(this, &UTargetLockComponent::OnTargetDestroyed);

		// Bind to new target
		CurrentTarget = BestNewTarget;
		CurrentTarget->OnDestroyed.AddDynamic(this, &UTargetLockComponent::OnTargetDestroyed);
	}
}