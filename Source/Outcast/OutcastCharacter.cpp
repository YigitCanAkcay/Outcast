#include "OutcastCharacter.h"

AOutcastCharacter::AOutcastCharacter()
  :
  Speed(0.0f),
  Direction(FVector()),
  WalkPlayrate(1.0f),
  LegsRotation(FRotator()),
  Jumping(EJump::NONE),
  JumpHeight(0.0f),
  JumpStartLocZ(0.0f),
  BunnyHopSpeedRatio(DefaultJumpSpeedRatio)
{
 	PrimaryActorTick.bCanEverTick = true;

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Warrior.SK_Cat_Warrior'"));
  if (Mesh.Succeeded())
  {
    SkeletalMeshComp = GetMesh();
    SkeletalMeshComp->SetSkeletalMesh(Mesh.Object);

    static ConstructorHelpers::FObjectFinder<UClass> AnimBP(TEXT("Class'/Game/Feline_Warrior/Animations/Character_Animation_BP.Character_Animation_BP_C'"));
    if (AnimBP.Succeeded())
    {
      SkeletalMeshComp->SetAnimInstanceClass(AnimBP.Object);
    }

    SkeletalMeshComp->RegisterComponent();
    SkeletalMeshComp->SetupAttachment(Capsule);
    SkeletalMeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    SkeletalMeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
  }

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> Weap(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Sword.SK_Cat_Sword'"));
  if (Weap.Succeeded())
  {
    SkeletalMeshCompWeapon = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Sword")));
    if (SkeletalMeshCompWeapon)
    {
      SkeletalMeshCompWeapon->SetSkeletalMesh(Weap.Object);
    }

    if (SkeletalMeshComp)
    {
      const FName SwordSocket = TEXT("Sword");
      SkeletalMeshCompWeapon->AttachToComponent(
        SkeletalMeshComp,
        FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
        SwordSocket);
    }
  }

  Capsule = Cast<UCapsuleComponent>(RootComponent);
  if (Capsule)
  {
    Capsule->SetCapsuleHalfHeight(120.0f);
    Capsule->SetCapsuleRadius(60.0f);

    Capsule->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));
    Capsule->OnComponentHit.__Internal_AddDynamic(this, &AOutcastCharacter::OnHit, FName("OnHit"));

  }

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(RootComponent);

  Camera->SetRelativeLocation(FVector(-423.0f, 0.0f, 200.0f));
  Camera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

  Movement               = Cast<UCharacterMovementComponent>(GetMovementComponent());
  Movement->AirControl   = 1.0f;
  Movement->MaxWalkSpeed = 1000.0f;

  // Take control of the player
  AutoPossessPlayer = EAutoReceiveInput::Player0;

}

void AOutcastCharacter::BeginPlay()
{
	Super::BeginPlay();

  Anim = Cast<UOutcastAnimInstance>(SkeletalMeshComp->GetAnimInstance());
  if (!Anim)
  {
    Destroy();
  }
}

void AOutcastCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  //******** LOOK AROUND ********
  
  // Rotate whole Character
  FRotator NewRotation = GetActorRotation();
  NewRotation.Yaw      = NewRotation.Yaw + MouseInput.X;
  SetActorRotation(NewRotation);

  // Rotate the torso of the character
  FRotator NewTorsoRotation = Anim->GetTorsoRotation();
  NewTorsoRotation.Roll     = FMath::Clamp(NewTorsoRotation.Roll - MouseInput.Y, -80.0f, 80.0f);
  Anim->SetTorsoRotation(NewTorsoRotation);

  // Rotate the camera
  FRotator NewCameraRot = Camera->GetComponentRotation();
  NewCameraRot.Pitch    = NewCameraRot.Pitch + MouseInput.Y;

  if (NewCameraRot.Pitch >= -80.0f
    && NewCameraRot.Pitch <= 80.0f)
  {
    Camera->SetWorldRotation(NewCameraRot);

    FVector NewCameraLoc = Camera->RelativeLocation;
    const float NewRadius = NewCameraLoc.Size();
    const float Angle = FMath::Atan2(NewCameraLoc.Z, (NewCameraLoc.X == 0 ? 1 : NewCameraLoc.X));

    NewCameraLoc.Z = NewRadius * FMath::Sin(Angle + FMath::DegreesToRadians(MouseInput.Y));
    NewCameraLoc.X = NewRadius * FMath::Cos(Angle + FMath::DegreesToRadians(MouseInput.Y));

    Camera->SetRelativeLocation(NewCameraLoc);
  }

  //******** LOOK AROUND ********


  //******** MOVE AROUND ********
  Direction    = FVector(0.0f, 0.0f, 0.0f);
  WalkPlayrate = 1.0f;

  //**** DIRECTION ****
  if (KeyMap[EKeys::W])
  {
    Direction = Direction + GetActorRotation().Vector();
  }

  if (KeyMap[EKeys::A])
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw      = Rot.Yaw - 90.0f;
    Direction    = Direction + Rot.Vector();
  }

  if (KeyMap[EKeys::S])
  {
    Direction    = Direction + GetActorRotation().Vector() * -1;
    WalkPlayrate = -1.0f;
  }

  if (KeyMap[EKeys::D])
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw      = Rot.Yaw + 90.0f;
    Direction    = Direction + Rot.Vector();
  }
  //**** DIRECTION ****

  //**** SPEED ****
  if ( KeyMap[EKeys::W]
    || KeyMap[EKeys::A]
    || KeyMap[EKeys::S]
    || KeyMap[EKeys::D])
  {
    Speed = FMath::Clamp(Speed + 5.0f, MinSpeed, MaxSpeed);
  }
  else
  {
    Speed = FMath::Clamp(Speed - 5.0f, MinSpeed, MaxSpeed);
  }

  Anim->SetAcceleration(Speed);
  Anim->SetWalkPlayrate(WalkPlayrate);
  if (Speed == 0.0f)
  {
    Direction = GetActorRotation().Vector();
  }
  //**** SPEED ****

  //**** LEGS ROTATION ****
  if (KeyMap[EKeys::W])
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    if (KeyMap[EKeys::A])
    {
      LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
    else if (KeyMap[EKeys::D])
    {
      LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
  }
  else if (KeyMap[EKeys::S])
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    if (KeyMap[EKeys::A])
    {
      LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
    else if (KeyMap[EKeys::D])
    {
      LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
  }
  else if (KeyMap[EKeys::A])
  {
    LegsRotation = FRotator(0.0f, -90.0f, 0.0f);
  }
  else if (KeyMap[EKeys::D])
  {
    LegsRotation = FRotator(0.0f, 90.0f, 0.0f);
  }
  else
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
  }

  Anim->SetLegsRotation(LegsRotation);
  //**** LEGS ROTATION ****

  if (!Direction.IsZero())
  {
    if (Jumping == EJump::NONE)
    {
      AddMovementInput(Direction, Speed / 5);
    }
    else if (Jumping == EJump::BunnyHop)
    {
      AddMovementInput(Direction, Speed / BunnyHopSpeedRatio);
    }
    else
    {
      AddMovementInput(Direction, Speed / DefaultJumpSpeedRatio);
    }
  }

  //******** MOVE AROUND ********


  //******** JUMP ********
  if (KeyMap[EKeys::Space])
  {
    if (Jumping != EJump::BunnyHop)
    {
      if ( JumpHeight < JumpHeightLimit
        && Jumping != EJump::Downwards)
      {
        if (Jumping == EJump::NONE)
        {
          JumpStartLocZ = GetActorLocation().Z;
        }
        Jumping = EJump::Upwards;
      }
      else
      {
        Jumping = EJump::Downwards;
      }
    }
  }
  else if ((Jumping == EJump::Upwards|| Jumping == EJump::BunnyHop) && JumpHeight >= MinJumpHeight)
  {
    Jumping = EJump::Downwards;
  }
 
  if (Jumping == EJump::Upwards)
  {
    JumpHeight = GetActorLocation().Z - JumpStartLocZ;
    Movement->SetMovementMode(MOVE_Flying);

    AddMovementInput(GetActorUpVector(), 5.0f);

    Anim->SetIsJumping(true);
  }
  else if (Jumping == EJump::Downwards)
  {
    Movement->SetMovementMode(MOVE_Falling);
  }
  else if (Jumping == EJump::NONE)
  {
    JumpHeight = 0.0f;
    Anim->SetIsJumping(false);
  }
  else if (Jumping == EJump::BunnyHop)
  {
    Anim->SetIsJumping(true);

    JumpHeight = GetActorLocation().Z - JumpStartLocZ;

    AddMovementInput(GetActorUpVector(), 5.0f);
    if (JumpHeight > BunnyHopMaxHeight)
    {
      Jumping            = EJump::Upwards;
      BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
    }
  }
  //******** JUMP ********

  //******** ATTACK ********
  // Check if a sword attack is being executed or not
  if (Anim->GetIsSlashingLeft())
  {
    Anim->SetIsSwordAttacking(true);
  }
  else
  {
    Anim->SetIsSwordAttacking(false);
  }

  if (MouseMap[EMouse::Left] && KeyMap[EKeys::A])
  {
    Anim->SetIsSlashingLeft(true);
  }
  //******** ATTACK ********
}

void AOutcastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Initialize KeyMap
  KeyMap.Add(EKeys::W, false);
  KeyMap.Add(EKeys::A, false);
  KeyMap.Add(EKeys::S, false);
  KeyMap.Add(EKeys::D, false);
  KeyMap.Add(EKeys::Space, false);

  // Keyboard input
  PlayerInputComponent->BindAction(
    "W",
    IE_Pressed,
    this,
    &AOutcastCharacter::WPressed);
  PlayerInputComponent->BindAction(
    "W",
    IE_Released,
    this,
    &AOutcastCharacter::WReleased);

  PlayerInputComponent->BindAction(
    "A",
    IE_Pressed,
    this,
    &AOutcastCharacter::APressed);
  PlayerInputComponent->BindAction(
    "A",
    IE_Released,
    this,
    &AOutcastCharacter::AReleased);

  PlayerInputComponent->BindAction(
    "S",
    IE_Pressed,
    this,
    &AOutcastCharacter::SPressed);
  PlayerInputComponent->BindAction(
    "S",
    IE_Released,
    this,
    &AOutcastCharacter::SReleased);

  PlayerInputComponent->BindAction(
    "D",
    IE_Pressed,
    this,
    &AOutcastCharacter::DPressed);
  PlayerInputComponent->BindAction(
    "D",
    IE_Released,
    this,
    &AOutcastCharacter::DReleased);

  PlayerInputComponent->BindAction(
    "Space",
    IE_Pressed,
    this,
    &AOutcastCharacter::SpacePressed);
  PlayerInputComponent->BindAction(
    "Space",
    IE_Released,
    this,
    &AOutcastCharacter::SpaceReleased);

  // Initialize Mouse Input
  MouseInput = FVector2D(0.0f, 0.0f);
  MouseMap.Add(EMouse::Left, false);
  MouseMap.Add(EMouse::Right, false);

  // Mouse input
  PlayerInputComponent->BindAction(
    "MouseLeft",
    IE_Pressed,
    this,
    &AOutcastCharacter::MouseLeftPressed);
  PlayerInputComponent->BindAction(
    "MouseRight",
    IE_Pressed,
    this,
    &AOutcastCharacter::MouseRightPressed);

  PlayerInputComponent->BindAction(
    "MouseLeft",
    IE_Released,
    this,
    &AOutcastCharacter::MouseLeftReleased);
  PlayerInputComponent->BindAction(
    "MouseRight",
    IE_Released,
    this,
    &AOutcastCharacter::MouseRightReleased);

  PlayerInputComponent->BindAxis(
    "MouseUpDown",
    this,
    &AOutcastCharacter::MouseUpDown);
  PlayerInputComponent->BindAxis(
    "MouseRightLeft",
    this,
    &AOutcastCharacter::MouseRightLeft);
}

void AOutcastCharacter::WPressed()
{
  KeyMap.Add(EKeys::W, true);
}

void AOutcastCharacter::WReleased()
{
  KeyMap.Add(EKeys::W, false);
}

void AOutcastCharacter::APressed()
{
  KeyMap.Add(EKeys::A, true);
}

void AOutcastCharacter::AReleased()
{
  KeyMap.Add(EKeys::A, false);
}

void AOutcastCharacter::SPressed()
{
  KeyMap.Add(EKeys::S, true);
}

void AOutcastCharacter::SReleased()
{
  KeyMap.Add(EKeys::S, false);
}

void AOutcastCharacter::DPressed()
{
  KeyMap.Add(EKeys::D, true);
}

void AOutcastCharacter::DReleased()
{
  KeyMap.Add(EKeys::D, false);
}

void AOutcastCharacter::SpacePressed()
{
  KeyMap.Add(EKeys::Space, true);
}

void AOutcastCharacter::SpaceReleased()
{
  KeyMap.Add(EKeys::Space, false);
}

void AOutcastCharacter::MouseLeftPressed()
{
  MouseMap.Add(EMouse::Left, true);
}

void AOutcastCharacter::MouseRightPressed()
{
  MouseMap.Add(EMouse::Right, true);
}

void AOutcastCharacter::MouseLeftReleased()
{
  MouseMap.Add(EMouse::Left, false);
}

void AOutcastCharacter::MouseRightReleased()
{
  MouseMap.Add(EMouse::Right, false);
}

void AOutcastCharacter::MouseUpDown(const float AxisValue)
{
  MouseInput.Y = AxisValue;
}

void AOutcastCharacter::MouseRightLeft(const float AxisValue)
{
  MouseInput.X = AxisValue;
}

void AOutcastCharacter::OnHit(
  UPrimitiveComponent* HitComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  FVector NormalImpulse,
  const FHitResult& Hit)
{
  // Character is on top of a walkable plane
  if (OtherActor->ActorHasTag(FName("Walkable")))
  {
    if (KeyMap[EKeys::Space] 
      && (KeyMap[EKeys::A] || KeyMap[EKeys::D]) 
      && (Jumping == EJump::Downwards || Jumping == EJump::BunnyHop))
    {
      Jumping            = EJump::BunnyHop;
      JumpHeight         = 0.0f;
      BunnyHopSpeedRatio = FMath::Clamp(BunnyHopSpeedRatio / 3.0f, BunnyHopFastestSpeedRatio, DefaultJumpSpeedRatio);
      Anim->SetIsJumping(false);
      Movement->SetMovementMode(MOVE_Flying);
    }
    else
    {
      Jumping            = EJump::NONE;
      BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
      Movement->SetMovementMode(MOVE_Walking);
    }
  }
}