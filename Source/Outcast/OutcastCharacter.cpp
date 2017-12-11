#include "OutcastCharacter.h"

AOutcastCharacter::AOutcastCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Warrior.SK_Cat_Warrior'"));
  if (Mesh.Succeeded())
  {
    SkeletalMeshComp = GetMesh();
    SkeletalMeshComp->SetSkeletalMesh(Mesh.Object);
  }

  static ConstructorHelpers::FObjectFinder<UClass> AnimBP(TEXT("Class'/Game/Feline_Warrior/Animations/Character_Animation_BP.Character_Animation_BP_C'"));
  if (AnimBP.Succeeded())
  {
    SkeletalMeshComp->SetAnimInstanceClass(AnimBP.Object);
  }

  Capsule = Cast<UCapsuleComponent>(RootComponent);
  if (Capsule)
  {
    Capsule->SetCapsuleHalfHeight(120.0f);
    Capsule->SetCapsuleRadius(60.0f);

    Capsule->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));
    Capsule->OnComponentHit.__Internal_AddDynamic(this, &AOutcastCharacter::OnHit, FName("OnHit"));

  }

  if (SkeletalMeshComp)
  {
    SkeletalMeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    SkeletalMeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
  }

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(RootComponent);

  Camera->SetRelativeLocation(FVector(-310.0f, 0.0f, 123.0f));
  Camera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

  Movement = Cast<UCharacterMovementComponent>(GetMovementComponent());

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

  Speed              = 0.0f;
  JumpHeight         = 0.0f;
  JumpHeightLimit    = 1000.0f;
  BunnyHopSpeedRatio = 100.0f;
  MinJumpHeight      = 75.0f;
  Direction          = FVector();

  Movement->AirControl = 1.0f;
}

void AOutcastCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  //******** LOOK AROUND ********
  FRotator NewRotation = GetActorRotation();
  NewRotation.Yaw = NewRotation.Yaw + MouseInput.X;
  NewRotation.Pitch = NewRotation.Pitch + MouseInput.Y;
  NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, -80.0f, 80.0f);

  SetActorRotation(NewRotation);
  //******** LOOK AROUND ********


  //******** MOVE AROUND ********
  Direction = FVector(0.0f, 0.0f, 0.0f);
  WalkPlayrate = 1.0f;

  //**** DIRECTION ****
  if (KeyMap[EKeys::W])
  {
    Direction = Direction + GetActorRotation().Vector();
  }

  if (KeyMap[EKeys::A])
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw = Rot.Yaw - 90.0f;
    Direction = Direction + Rot.Vector();
  }

  if (KeyMap[EKeys::S])
  {
    Direction = Direction + GetActorRotation().Vector() * -1;
    WalkPlayrate = -1.0f;
  }

  if (KeyMap[EKeys::D])
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw = Rot.Yaw + 90.0f;
    Direction = Direction + Rot.Vector();
  }
  //**** DIRECTION ****

  //**** SPEED ****
  if ( KeyMap[EKeys::W]
    || KeyMap[EKeys::A]
    || KeyMap[EKeys::S]
    || KeyMap[EKeys::D])
  {
    Speed = FMath::Clamp(Speed + 5.0f, 0.0f, 100.0f);
  }
  else
  {
    Speed = FMath::Clamp(Speed - 5.0f, 0.0f, 100.0f);
  }
  //**** SPEED ****

  if (Speed == 0.0f)
  {
    Direction = FVector(0.0f, 0.0f, 0.0f);
  }

  Anim->SetAcceleration(Speed);
  Anim->SetWalkPlayrate(WalkPlayrate);

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
      AddMovementInput(Direction, Speed / 100);
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
    if (JumpHeight > 100.0f)
    {
      Jumping            = EJump::Upwards;
      BunnyHopSpeedRatio = 100.0f;
    }
  }
  
  FString JumpMode;
  if (Jumping == EJump::NONE)
  {
    JumpMode = "NONE";
  }
  else if (Jumping == EJump::Upwards)
  {
    JumpMode = "Upwards";
  }
  else if (Jumping == EJump::Downwards)
  {
    JumpMode = "Downwards";
  }
  else if(Jumping == EJump::BunnyHop)
  {
    JumpMode = "BUNNYHOP !!!";
  }
  UE_LOG(LogTemp, Warning, TEXT("Mode: %s -- JumpHeight: %f"), *JumpMode, JumpHeight);
  //******** JUMP ********
}

void AOutcastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Initialize Mouse Input
  MouseInput = FVector2D(0.0f, 0.0f);

  // Bind axes
  PlayerInputComponent->BindAxis(
    "MouseUpDown",
    this,
    &AOutcastCharacter::MouseUpDown);
  PlayerInputComponent->BindAxis(
    "MouseRightLeft",
    this,
    &AOutcastCharacter::MouseRightLeft);

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

}

void AOutcastCharacter::WPressed()
{
  if (!KeyMap[EKeys::S])
  {
    KeyMap.Add(EKeys::W, true);
  }
}

void AOutcastCharacter::WReleased()
{
  KeyMap.Add(EKeys::W, false);
}

void AOutcastCharacter::APressed()
{
  if (!KeyMap[EKeys::D])
  {
    KeyMap.Add(EKeys::A, true);
  }
}

void AOutcastCharacter::AReleased()
{
  KeyMap.Add(EKeys::A, false);
}

void AOutcastCharacter::SPressed()
{
  if (!KeyMap[EKeys::W])
  {
    KeyMap.Add(EKeys::S, true);
  }
}

void AOutcastCharacter::SReleased()
{
  KeyMap.Add(EKeys::S, false);
}

void AOutcastCharacter::DPressed()
{
  if (!KeyMap[EKeys::A])
  {
    KeyMap.Add(EKeys::D, true);
  }
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
    if (KeyMap[EKeys::Space] && Jumping == EJump::Downwards || Jumping == EJump::BunnyHop)
    {
      Jumping            = EJump::BunnyHop;
      JumpHeight         = 0.0f;
      BunnyHopSpeedRatio = FMath::Clamp(BunnyHopSpeedRatio / 3.0f, 10.0f, 100.0f);
      Anim->SetIsJumping(false);
      Movement->SetMovementMode(MOVE_Flying);
    }
    else
    {
      Jumping            = EJump::NONE;
      BunnyHopSpeedRatio = 100.0f;
      Movement->SetMovementMode(MOVE_Walking);
    }
  }
}