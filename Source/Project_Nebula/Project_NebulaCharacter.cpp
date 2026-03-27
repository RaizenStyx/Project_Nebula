// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_NebulaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AProject_NebulaCharacter

AProject_NebulaCharacter::AProject_NebulaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm



	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(150.0f); // Adjust this radius for how close they need to be
	InteractionSphere->SetCollisionProfileName(TEXT("Trigger")); // Only overlaps, doesn't block movement



	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// 1. Create the Weapon Mesh Component
	EquippedWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedWeaponMesh"));

	// 2. Attach it to your character skeleton's hand socket 
	// (Make sure "WeaponSocket" exactly matches the socket name on your skeletal mesh!)
	EquippedWeaponMesh->SetupAttachment(GetMesh(), FName("WeaponSocket"));

	// Turn off collision so the weapon doesn't bump into the player's own capsule
	EquippedWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProject_NebulaCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AProject_NebulaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Look);

		// Attacking
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::LightAttack);

		// Dodging & Crouching
		EnhancedInputComponent->BindAction(DodgeCrouchAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::DodgeOrCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AProject_NebulaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AProject_NebulaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AProject_NebulaCharacter::LightAttack(const FInputActionValue& Value)
{
	// Future LitRPG damage scaling and animation triggers will go here.
	// For now, print to screen so we know the button works.
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Light Attack Triggered!"));
}

void AProject_NebulaCharacter::DodgeOrCrouch(const FInputActionValue& Value)
{
	// Check if the character is currently moving faster than a near-stop
	if (GetCharacterMovement()->Velocity.SizeSquared2D() > 100.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Dodge Roll Triggered!"));

		// Note: We will add the physical LaunchCharacter() or Root Motion Dodge Animation here later.
	}
	else
	{
		if (bIsCrouched)
		{
			UnCrouch();
		}
		else
		{
			Crouch();
		}
	}
}

void AProject_NebulaCharacter::EquipWeaponFromRow(FName WeaponRowName)
{
	// Fail-safe: Make sure the Data Table is actually assigned in the editor!
	if (!WeaponDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable is missing on the Character!"));
		return;
	}

	// Search the Data Table for the exact Row Name
	static const FString ContextString(TEXT("Weapon Equip Context"));
	FWeaponInfo* FoundWeaponRow = WeaponDataTable->FindRow<FWeaponInfo>(WeaponRowName, ContextString);

	if (FoundWeaponRow)
	{
		// 1. Overwrite our current stats with the new weapon's stats
		CurrentWeaponInfo = *FoundWeaponRow;

		// 2. Update the 3D model in the player's hand
		if (CurrentWeaponInfo.VisualMesh)
		{
			EquippedWeaponMesh->SetStaticMesh(CurrentWeaponInfo.VisualMesh);
		}
		else
		{
			// If the DT has an empty mesh, clear the hand
			EquippedWeaponMesh->SetStaticMesh(nullptr);
		}

		UE_LOG(LogTemp, Display, TEXT("Successfully equipped: %s"), *CurrentWeaponInfo.ItemName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find weapon row: %s"), *WeaponRowName.ToString());
	}
}