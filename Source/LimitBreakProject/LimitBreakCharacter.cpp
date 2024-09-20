// Fill out your copyright notice in the Description page of Project Settings.


#include "LimitBreakCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ALimitBreakCharacter::ALimitBreakCharacter()
{
 	// Set this character to call Tick() every frame, tick is performed before physics as it affects physics
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup == TG_PrePhysics;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

// Called when the game starts or when spawned
void ALimitBreakCharacter::BeginPlay()
{
	Super::BeginPlay();

	//properties related to movement
	m_InitialUpwardVelocity = m_MaxJumpHeight * 2.0f * m_LateralAirSpeed / m_LateralDistance;
	m_Gravity = m_MaxJumpHeight * (-2.0f) * m_LateralAirSpeed * m_LateralAirSpeed / (m_LateralDistance * m_LateralDistance);
	GetCharacterMovement()->MaxWalkSpeed = m_LateralWalkSpeed;
}

// Called every frame
void ALimitBreakCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Check if player is on ground
	const bool wasOnGround = bIsOnGround;
	bIsOnGround = CheckGround();

	//Adjust vertical speed if not on ground
	if (!bIsOnGround)
	{
		//Apply gravity
		if (m_VerticalSpeed > 0.0f)
		{
			if (bJumpHeld)
			{
				m_VerticalSpeed += m_Gravity * DeltaTime;
			}
			else
			{
				m_VerticalSpeed += m_Gravity * m_FallModifier * DeltaTime;
			}
		}
		else
		{
			if (bJumpHeld)
			{
				m_VerticalSpeed += m_Gravity * m_GliderMultiplier * DeltaTime;
			}
			else
			{
				m_VerticalSpeed += m_Gravity * m_FallModifier * DeltaTime;
			}
		}

		//Apply speed
		LaunchCharacter(FVector(0.0f, 0.0f, m_VerticalSpeed), false, true);
	}

	//Check if the player is landing at this frame
	if (!wasOnGround && bIsOnGround)
	{
		Land();
	}

	//Rotate the player to match the rotation of controller
	SetActorRotation(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
}

// Called to bind functionality to input
void ALimitBreakCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ALimitBreakCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ALimitBreakCharacter::ReleaseJump);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALimitBreakCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALimitBreakCharacter::Look);
	}
}

void ALimitBreakCharacter::Move(const FInputActionValue& Value)
{
	if (ensure(Controller))
	{
		// input is a Vector2D
		const FVector2D MovementVector = Value.Get<FVector2D>();

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ALimitBreakCharacter::Look(const FInputActionValue& Value)
{
	if (ensure(Controller))
	{
		// input is a Vector2D
		const FVector2D LookAxisVector = Value.Get<FVector2D>();

		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

//Checks if player is on ground
bool ALimitBreakCharacter::CheckGround() const
{
	//if player is moving upwards, don't check it
	if (m_VerticalSpeed > 0.0f)
	{
		return false;
	}

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	FHitResult OutHit;

	const FVector PlayerLocation = GetActorLocation();

	const bool isOnGround = (GetWorld()->LineTraceSingleByChannel(OutHit, PlayerLocation, PlayerLocation - FVector(0.0f, 0.0f, 100.0f), ECC_Visibility, CollisionParams));

	return isOnGround;
}

void ALimitBreakCharacter::Jump(const FInputActionValue& Value)
{
	//Only jump when player is on ground
	if (!bIsOnGround)
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = m_LateralAirSpeed;
	m_VerticalSpeed = m_InitialUpwardVelocity;
	bIsOnGround = false;
	bJumpHeld = true;
}

//Triggered when player releases the jump button
void ALimitBreakCharacter::ReleaseJump(const FInputActionValue& Value)
{
	bJumpHeld = false;
}

//Called when player lands to set the parameters to ground movement parameters
void ALimitBreakCharacter::Land()
{
	GetCharacterMovement()->MaxWalkSpeed = m_LateralWalkSpeed;
	bJumpHeld = false;
	m_VerticalSpeed = 0.0f;
}
