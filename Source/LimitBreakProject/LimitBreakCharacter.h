// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LimitBreakCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class LIMITBREAKPROJECT_API ALimitBreakCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALimitBreakCharacter();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Jump(const FInputActionValue& Value);

	void ReleaseJump(const FInputActionValue& Value);

	void Land();

private:
	bool CheckGround() const;

private:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//Exposed parametes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Stats")
	float m_LateralWalkSpeed = 300.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character Stats")
	float m_LateralAirSpeed = 100.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character Stats")
	float m_LateralDistance = 100.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character Stats")
	float m_MaxJumpHeight = 200.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character Stats")
	float m_FallModifier = 3.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character Stats")
	float m_GliderMultiplier = 0.2f;

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

private:
	//ground check related
	bool bIsOnGround = false;

	bool bJumpHeld = false;

	//Derived parameters
	float m_InitialUpwardVelocity;

	float m_Gravity;

	float m_VerticalSpeed = 0.0f;
};
