#include "OutcastAnimInstance.h"

UOutcastAnimInstance::UOutcastAnimInstance(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  bIsJumping      = false;
  bIsRunning      = false;
  bIsSlashingLeft = false;
  Speed           = 0.0f;
  PlayRate        = 1.0f;
}

void UOutcastAnimInstance::SetIsRunning(const bool bNewIsRunning)
{
  bIsRunning = bNewIsRunning;
}
bool UOutcastAnimInstance::GetIsRunning() const
{
  return bIsRunning;
}

void UOutcastAnimInstance::SetAcceleration(const float NewSpeed)
{
  Speed = NewSpeed;
}

void UOutcastAnimInstance::SetIsJumping(const bool bNewIsJumping)
{
  bIsJumping = bNewIsJumping;
}
bool UOutcastAnimInstance::GetIsJumping() const
{
  return bIsJumping;
}

void UOutcastAnimInstance::SetIsSlashingLeft(const bool bNewIsSlashingLeft)
{
  bIsSlashingLeft = bNewIsSlashingLeft;
}
bool UOutcastAnimInstance::GetIsSlashingLeft() const
{
  return bIsSlashingLeft;
}

void UOutcastAnimInstance::SetWalkPlayrate(const float NewPlayRate)
{
  PlayRate = NewPlayRate;
}