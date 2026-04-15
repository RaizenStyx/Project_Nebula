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
#include "PlayerStatsComponent.h"
#include "Public/SkillManagerComponent.h"
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


	// Adding Skill Manager here. 
	// TODO: Look into adding stat component this way too. 
	SkillManager = CreateDefaultSubobject<USkillManagerComponent>(TEXT("SkillManager"));
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


		
		
		// The Left Bumper (Modifier)
		EnhancedInputComponent->BindAction(HotbarModifierAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::HotbarModifierStarted);
		EnhancedInputComponent->BindAction(HotbarModifierAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::HotbarModifierCompleted);

		// The Face Top Button (Y / Triangle) And Future other face buttons
		// ETriggerEvent::Completed acts as a "Tap" (Released before Hold threshold)
		EnhancedInputComponent->BindAction(SlotFaceTopAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_FaceTop_Tap);
		// ETriggerEvent::Triggered acts as a "Hold" (Threshold met)
		EnhancedInputComponent->BindAction(SlotFaceTopAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_FaceTop_Hold);

		// D-Pad Input buttons will go here
		EnhancedInputComponent->BindAction(SlotDPadUpAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_DPadUp_Tap);
		EnhancedInputComponent->BindAction(SlotDPadUpAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_DPadUp_Hold);
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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Light Attack Triggered!"));
}

void AProject_NebulaCharacter::DodgeOrCrouch(const FInputActionValue& Value)
{
	// Check if the character is currently moving faster than a near-stop
	if (GetCharacterMovement()->Velocity.SizeSquared2D() > 100.f)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		// Ensure we aren't already dodging so we can't spam it in the air
		if (DodgeMontage && AnimInstance && !AnimInstance->Montage_IsPlaying(DodgeMontage))
		{
			// 1. Get Stats for Scaling
			UPlayerStatsComponent* StatsComp = FindComponentByClass<UPlayerStatsComponent>();
			float EffectiveAgility = StatsComp ? StatsComp->GetEffectiveStatValue(StatsComp->Agility) : 10.0f;

			// 2. Play the Animation (Scaled by Agility)
			float PlayRate = 1.0f + (EffectiveAgility * 0.002f);
			PlayAnimMontage(DodgeMontage, PlayRate);

			// 3. Calculate Launch Velocity
			// Get the normal direction the player is currently moving
			FVector DodgeDirection = GetCharacterMovement()->Velocity.GetSafeNormal2D();

			// Base force of the dodge + Agility scaling (+10 units of force per effective point)
			float BaseDodgeForce = 1500.0f;
			float TotalDodgeForce = BaseDodgeForce + (EffectiveAgility * 10.0f);

			FVector LaunchVelocity = DodgeDirection * TotalDodgeForce;

			// 4. Launch the Character!
			// The two 'true' booleans override current XY velocity so the dash feels snappy and immediate
			LaunchCharacter(LaunchVelocity, true, false);

			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Dodge Launched! Force: %f"), TotalDodgeForce));
		}
	}
	else
	{
		if (bIsCrouched) UnCrouch();
		else Crouch();
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

float AProject_NebulaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. Check Invincibility (I-frames from dodging)
	if (bIsInvincible)
	{
		return 0.0f;
	}

	// 2. Fetch the Stats Component
	UPlayerStatsComponent* StatsComp = FindComponentByClass<UPlayerStatsComponent>();
	if (StatsComp)
	{
		// 3. Apply Fortitude Reduction
		float ActualDamage = StatsComp->CalculateIncomingPhysicalDamage(DamageAmount);

		// 4. Apply the damage
		StatsComp->ModifyHealth(-ActualDamage);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Took %f damage!"), ActualDamage));

		return ActualDamage;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AProject_NebulaCharacter::HotbarModifierStarted()
{
	// Tell the Blueprint UI to appear!
	OnToggleCrossHotbar(true);
}

void AProject_NebulaCharacter::HotbarModifierCompleted()
{
	// Tell the Blueprint UI to disappear!
	OnToggleCrossHotbar(false);
}

void AProject_NebulaCharacter::Input_FaceTop_Tap()
{
	// Pass execution to the Skill Manager Component
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(ENebulaSkillSlot::Face_Top, false); // false = not a hold
	}
}

void AProject_NebulaCharacter::Input_FaceTop_Hold()
{
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(ENebulaSkillSlot::Face_Top, true); // true = is a hold
	}
}

// At the bottom of the file:
void AProject_NebulaCharacter::Input_DPadUp_Tap()
{
	if (SkillManager) SkillManager->ExecuteSkillInSlot(ENebulaSkillSlot::DPad_Up, false);
}
void AProject_NebulaCharacter::Input_DPadUp_Hold()
{
	if (SkillManager) SkillManager->ExecuteSkillInSlot(ENebulaSkillSlot::DPad_Up, true);
}