#pragma once

#include "CoreMinimal.h"

#include "OutcastAnimInstance.generated.h"


UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class OUTCAST_API UOutcastAnimInstance : public UAnimInstance
{
  GENERATED_UCLASS_BODY()

  AActor* MyActor;

  //******** BASIC MOVEMENT ********
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  bool bIsJumping;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  bool bIsRunning;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  float Speed;  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  float PlayRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  FRotator TorsoRotation;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  FRotator LegsRotation;

  TArray<FRotator> LegsRotationBuffer;
  int CurrentLegsRotationBufferIndex;
  //******** BASIC MOVEMENT ********

  //******** ATTACKING ********
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  float AttackMovementBlendWeight;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  bool bIsSlashingLeft;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  bool bIsSlashingRight;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
  bool bIsSlashingForward;
  //******** ATTACKING ********

public:
  void SetMyActor(AActor* const NewActor);
  UFUNCTION(BlueprintCallable)
  AActor* GetMyActor();

  void SetIsRunning(const bool bNewIsRunning);
  bool GetIsRunning() const;

  void SetAccelerationAndLegRotation(const float NewSpeed, const float ForwardDirection, const float SidewardDirection);

  void SetIsJumping(const bool bNewIsJumping);
  bool GetIsJumping() const;

  void AddAttackMovementBlendWeight(const float DeltaAttackMovementBlendWeight);
  void SubtractAttackMovementBlendWeight(const float DeltaAttackMovementBlendWeight);

  void SetAttackMovementBlendWeight(const float NewAttackMovementBlendWeight);
  float GetAttackMovementBlendWeight() const;

  void SetIsSlashingLeft(const bool bNewIsSlashingLeft);
  bool GetIsSlashingLeft() const;

  void SetIsSlashingRight(const bool bNewIsSlashingRight);
  bool GetIsSlashingRight() const;

  void SetIsSlashingForward(const bool bNewIsSlashingForward);
  bool GetIsSlashingForward() const;

  void SetWalkPlayrate(const float NewPlayRate);

  void SetTorsoRotation(const float MouseInputY);
  FRotator GetTorsoRotation() const;

  void SetLegsRotation(const FRotator& NewLegsRotation);
  FRotator GetLegsRotation() const;
};
