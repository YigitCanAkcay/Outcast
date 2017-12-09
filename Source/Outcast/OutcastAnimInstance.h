#pragma once

#include "CoreMinimal.h"
#include "OutcastAnimInstance.generated.h"


UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class OUTCAST_API UOutcastAnimInstance : public UAnimInstance
{
  GENERATED_UCLASS_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsJumping;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsRunning;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    float Speed;  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsSlashingLeft;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    float PlayRate;

public:
  void SetIsRunning(const bool bNewIsRunning);
  bool GetIsRunning() const;

  void SetAcceleration(const float NewSpeed);

  void SetIsJumping(const bool bNewIsJumping);
  bool GetIsJumping() const;

  void SetIsSlashingLeft(const bool bNewIsSlashingLeft);
  bool GetIsSlashingLeft() const;

  void SetWalkPlayrate(const float NewPlayRate);
};
