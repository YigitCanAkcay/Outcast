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

  static ConstructorHelpers::FObjectFinder<UAnimBlueprint> Anim(TEXT("AnimBlueprint'/Game/Feline_Warrior/Animations/Character_Animation_BP.Character_Animation_BP'"));
  if (Anim.Succeeded())
  {
    SkeletalMeshComp->SetAnimInstanceClass(Anim.Object->GetAnimBlueprintGeneratedClass());
  }

  Capsule = Cast<UCapsuleComponent>(RootComponent);
  if (Capsule)
  {
    Capsule->SetCapsuleHalfHeight(120.0f);
    Capsule->SetCapsuleRadius(60.0f);

    Capsule->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));

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

  Speed           = 0.0f;
  JumpHeight      = 0.0f;
  JumpHeightLimit = 500.0f;
  Direction       = FVector();
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
    FVector Location = GetActorLocation();
    SetActorLocation(Location + Direction * Speed / 5);
  }

  //******** MOVE AROUND ********


  //******** JUMP ********
  if (KeyMap[EKeys::Space])
  {
    if (JumpHeight < JumpHeightLimit && Jumping != EJump::Downwards)
    {
      Jumping = EJump::Upwards;
    }
    else
    {
      Jumping = EJump::Downwards;
    }
  }
  else
  {
    if (JumpHeight > 0.0f)
    {
      Jumping = EJump::Downwards;
    }
    else
    {
      Jumping = EJump::NONE;
    }
  }

  if (Jumping == EJump::Upwards)
  {
    JumpHeight = JumpHeight + 5;

    FVector Loc = GetActorLocation();
    Loc.Z       = Loc.Z + 5;
    SetActorLocation(Loc);

    //Jump();
    Anim->SetIsJumping(true);
  }
  else if (Jumping == EJump::Downwards && JumpHeight > 0.0f)
  {
    JumpHeight = JumpHeight - 5;

    FVector Loc = GetActorLocation();
    Loc.Z       = Loc.Z - 5;
    SetActorLocation(Loc);
  }
  else
  {
    Anim->SetIsJumping(false);
  }

  FString TheJump;
  if (Jumping == EJump::Upwards)
  {
    TheJump = "Jumping Upwards";
  }
  else if (Jumping == EJump::Downwards)
  {
    TheJump = "Jumping Downwards";
  }
  else
  {
    TheJump = "Jumping NONE";
  }
  UE_LOG(LogTemp, Warning, TEXT("%s -- %f"), *TheJump, JumpHeight);
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