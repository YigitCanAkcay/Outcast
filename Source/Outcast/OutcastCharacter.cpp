#include "OutcastCharacter.h"
#include "OutcastGameMode.h"

#define COL_Player ECC_GameTraceChannel1
#define COL_Weapon ECC_GameTraceChannel2

AOutcastCharacter::AOutcastCharacter()
  :
  Speed(0.0f),
  Direction(FVector()),
  WalkPlayrate(1.0f),
  LegsRotation(FRotator()),
  TorsoRotation(FRotator()),
  CharacterRotation(FRotator()),
  CharacterLocation(FVector()),
  Jumping(EJump::NONE),
  JumpHeight(0.0f),
  JumpStartLocZ(0.0f),
  BunnyHopSpeedRatio(DefaultJumpSpeedRatio),
  Attacking(EAttack::NONE),
  Health(100),
  MouseInput(FVector2D::ZeroVector)
{
 	PrimaryActorTick.bCanEverTick = true;

  // Initialize KeyMap
  KeyMap.Add(EKey::W, false);
  KeyMap.Add(EKey::A, false);
  KeyMap.Add(EKey::S, false);
  KeyMap.Add(EKey::D, false);
  KeyMap.Add(EKey::Space, false);

  // Initialize Mouse Input
  MouseMap.Add(EMouse::Left, false);
  MouseMap.Add(EMouse::Right, false);

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Warrior.SK_Cat_Warrior'"));
  if (BodyMesh.Succeeded())
  {
    Body = GetMesh();
    Body->SetSkeletalMesh(BodyMesh.Object);

    static ConstructorHelpers::FObjectFinder<UClass> AnimBP(TEXT("Class'/Game/Feline_Warrior/Animations/Character_Animation_BP.Character_Animation_BP_C'"));
    if (AnimBP.Succeeded())
    {
      Body->SetAnimInstanceClass(AnimBP.Object);
    }

    Body->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    Body->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
  }

   ConstructorHelpers::FObjectFinder<UStaticMesh> WeaponMesh(TEXT("StaticMesh'/Game/Meshes/Weapon_StaticMesh.Weapon_StaticMesh'"));
  if (WeaponMesh.Succeeded())
  {
    Weapon = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), FName(TEXT("Sword")));
    Weapon->SetStaticMesh(WeaponMesh.Object);

    Weapon->SetCollisionProfileName(TEXT("Weapon"));
  }

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> EyeballMesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Eyeball.SK_Cat_Eyeball'"));
  if (EyeballMesh.Succeeded())
  {
    Eye_R = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Eye_R")));
    Eye_R->SetSkeletalMesh(EyeballMesh.Object);

    Eye_L = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Eye_L")));
    Eye_L->SetSkeletalMesh(EyeballMesh.Object);
  }

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> ArmorMesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Chest.SK_Cat_Chest'"));
  if (ArmorMesh.Succeeded())
  {
    Armor = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Armor")));
    Armor->SetSkeletalMesh(ArmorMesh.Object);
  }

  Capsule = Cast<UCapsuleComponent>(RootComponent);
  Capsule->SetCapsuleHalfHeight(120.0f);
  Capsule->SetCapsuleRadius(60.0f);
  Capsule->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));
  Capsule->SetCollisionProfileName(TEXT("Player"));
  Capsule->OnComponentHit.__Internal_AddDynamic(this, &AOutcastCharacter::OnHit, FName("OnHit"));
  Capsule->OnComponentBeginOverlap.AddDynamic(this, &AOutcastCharacter::BodyOverlapBegin);
  Capsule->OnComponentEndOverlap.AddDynamic(this, &AOutcastCharacter::BodyOverlapEnd);
  
  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(RootComponent);
  Camera->SetRelativeLocation(FVector(-423.0f, 0.0f, 200.0f));
  Camera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

  Movement               = Cast<UCharacterMovementComponent>(GetMovementComponent());
  Movement->AirControl   = 1.0f;
  Movement->MaxWalkSpeed = 1000.0f;
  Movement->MaxFlySpeed  = 10000.0f;
  Movement->GravityScale = 5.0f;
  Movement->SetIsReplicated(true);

  bUseControllerRotationYaw   = false;
  bUseControllerRotationPitch = false;
  bUseControllerRotationRoll  = false;

  bReplicates        = true;
  bReplicateMovement = true;
}

void AOutcastCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AOutcastCharacter, ReplicatedData);
  DOREPLIFETIME_CONDITION(AOutcastCharacter, ReplicatedAnimData, COND_SimulatedOnly);
}

void AOutcastCharacter::BeginPlay()
{
  Super::BeginPlay();

  Anim = Cast<UOutcastAnimInstance>(Body->GetAnimInstance());
  if (!Anim)
  {
    Destroy();
  }

  FillReplicatedData();
  ReplicatedData.CharacterLocation = GetActorLocation();
  ReplicatedData.CharacterRotation = ReturnFVector(GetActorRotation());

  if (Body && Eye_L && Eye_R && Weapon && Armor)
  {
    const FName SwordSocket = TEXT("Sword");
    Weapon->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      SwordSocket);

    const FName EyeSocket_R = TEXT("Eye_R");
    const FName EyeSocket_L = TEXT("Eye_L");
    Eye_R->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      EyeSocket_R);
    Eye_L->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      EyeSocket_L);

    const FName ArmorSocket = TEXT("Chest");
    Armor->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      ArmorSocket);
  }
}

void AOutcastCharacter::ExtractReplicatedData()
{
  CharacterLocation = ReplicatedData.CharacterLocation;
  CharacterRotation = ReturnFRotator(ReplicatedData.CharacterRotation);
  Health            = ReplicatedData.Health;
}

void AOutcastCharacter::ApplyReplicatedData()
{
  ExtractReplicatedData();

  SetActorRotation(CharacterRotation);
  SetActorLocation(CharacterLocation);
}

void AOutcastCharacter::FillReplicatedData()
{
  ReplicatedData.CharacterLocation = CharacterLocation;
  ReplicatedData.CharacterRotation = ReturnFVector(CharacterRotation);
  ReplicatedData.Health            = Health;
}

void AOutcastCharacter::ExtractReplicatedAnimData()
{
  Speed         = ReplicatedAnimData.Speed;
  WalkPlayrate  = ReplicatedAnimData.WalkPlayrate;
  LegsRotation  = ReturnFRotator(ReplicatedAnimData.LegsRotation);
  TorsoRotation = ReturnFRotator(ReplicatedAnimData.TorsoRotation);
  Jumping       = ReplicatedAnimData.Jumping;
  Attacking     = ReplicatedAnimData.Attacking;
}

void AOutcastCharacter::ApplyReplicatedAnimData()
{
  ExtractReplicatedAnimData();

  if (Anim)
  {
  Anim->SetAcceleration(Speed);
  Anim->SetWalkPlayrate(WalkPlayrate);
  Anim->SetLegsRotation(LegsRotation);
  Anim->SetTorsoRotation(TorsoRotation);
  Anim->SetIsJumping(Jumping != EJump::NONE);
  Anim->SetIsSlashingLeft(Attacking == EAttack::Left);
  Anim->SetIsSlashingRight(Attacking == EAttack::Right);
  Anim->SetIsSlashingForward(Attacking == EAttack::Forward);
  }
}

void AOutcastCharacter::FillReplicatedAnimData()
{
  ReplicatedAnimData.Speed         = Speed;
  ReplicatedAnimData.WalkPlayrate  = WalkPlayrate;
  ReplicatedAnimData.LegsRotation  = ReturnFVector(LegsRotation);
  ReplicatedAnimData.TorsoRotation = ReturnFVector(TorsoRotation);
  ReplicatedAnimData.Jumping       = Jumping;
  ReplicatedAnimData.Attacking     = Attacking;
}

FVector AOutcastCharacter::ReturnFVector(const FRotator& Rotator)
{
  FVector Vector;
  Vector.X = Rotator.Pitch;
  Vector.Y = Rotator.Roll;
  Vector.Z = Rotator.Yaw;
  return Vector;
}
FRotator AOutcastCharacter::ReturnFRotator(const FVector& Vector)
{
  FRotator Rotator;
  Rotator.Pitch = Vector.X;
  Rotator.Roll = Vector.Y;
  Rotator.Yaw = Vector.Z;
  return Rotator;
}

int AOutcastCharacter::GetHealth()
{
  return Health;
}

void AOutcastCharacter::BodyOverlapBegin(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult& SweepResult)
{
  if (Role == ROLE_AutonomousProxy)
  {
    // Other Actor is the actor that triggered the event. Check that is not ourself.  
    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
    {
      AOutcastCharacter* AttackingCharacter = Cast<AOutcastCharacter>(OtherActor);

      if (!DamageTakenBy.Contains(AttackingCharacter))
      {
        DamageTakenBy.Add(AttackingCharacter, 0.0f);

        if (AttackingCharacter->GetAttack() == EAttack::NONE)
        {
          Health = FMath::Clamp(Health - 1, 0, 100);
        }
        else
        {
          Health = FMath::Clamp(Health - 20, 0, 100);
        }
      }
    }
  }
}

void AOutcastCharacter::BodyOverlapEnd(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex)
{
  if (Role == ROLE_AutonomousProxy)
  {
    // Other Actor is the actor that triggered the event. Check that is not ourself.  
    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
    {
      AOutcastCharacter* AttackingCharacter = Cast<AOutcastCharacter>(OtherActor);

      if (DamageTakenBy.Contains(AttackingCharacter))
      {
        DamageTakenBy.Remove(AttackingCharacter);
      }
    }
  }
}

void AOutcastCharacter::Local_LookAround()
{
  // Rotate the camera
  FRotator NewCameraRot = Camera->GetComponentRotation();
  NewCameraRot.Pitch = NewCameraRot.Pitch + MouseInput.Y;

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
}

void AOutcastCharacter::LookAround()
{
  // Rotate whole Character
  CharacterRotation = GetActorRotation();
  CharacterRotation.Yaw = CharacterRotation.Yaw + MouseInput.X;
  SetActorRotation(CharacterRotation);

  // Rotate the torso of the character
  TorsoRotation = Anim->GetTorsoRotation();
  TorsoRotation.Roll = FMath::Clamp(TorsoRotation.Roll - MouseInput.Y, -80.0f, 80.0f);
  Anim->SetTorsoRotation(TorsoRotation);

  // Rotate the camera
  FRotator NewCameraRot = Camera->GetComponentRotation();
  NewCameraRot.Pitch = NewCameraRot.Pitch + MouseInput.Y;

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
}

void AOutcastCharacter::MoveAround()
{
  Direction = FVector(0.0f, 0.0f, 0.0f);
  WalkPlayrate = 1.0f;

  //**** DIRECTION ****
  if (KeyMap[EKey::W])
  {
    Direction = Direction + GetActorRotation().Vector();
  }

  if (KeyMap[EKey::A])
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw = Rot.Yaw - 90.0f;
    Direction = Direction + Rot.Vector();
  }

  if (KeyMap[EKey::S])
  {
    Direction = Direction + GetActorRotation().Vector() * -1;
    WalkPlayrate = -1.0f;
  }

  if (KeyMap[EKey::D])
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw = Rot.Yaw + 90.0f;
    Direction = Direction + Rot.Vector();
  }
  //**** DIRECTION ****

  //**** SPEED ****
  if (KeyMap[EKey::W] && !KeyMap[EKey::S]
    || KeyMap[EKey::A] && !KeyMap[EKey::D]
    || KeyMap[EKey::S] && !KeyMap[EKey::W]
    || KeyMap[EKey::D] && !KeyMap[EKey::A])
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
  if (KeyMap[EKey::W])
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    if (KeyMap[EKey::A])
    {
      LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
    else if (KeyMap[EKey::D])
    {
      LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
  }
  else if (KeyMap[EKey::S])
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    if (KeyMap[EKey::A])
    {
      LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
    else if (KeyMap[EKey::D])
    {
      LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
  }
  else if (KeyMap[EKey::A] && !KeyMap[EKey::D])
  {
    LegsRotation = FRotator(0.0f, -90.0f, 0.0f);
  }
  else if (KeyMap[EKey::D] && !KeyMap[EKey::A])
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
      //AddMovementInput(Direction, Speed / 5);
      SetActorLocation(GetActorLocation() + Direction * Speed / 6);
    }
    else if (Jumping == EJump::BunnyHop)
    {
      //AddMovementInput(Direction, Speed / BunnyHopSpeedRatio);
      SetActorLocation(GetActorLocation() + Direction * Speed / BunnyHopSpeedRatio);
    }
    else
    {
      //AddMovementInput(Direction, Speed / DefaultJumpSpeedRatio);
      SetActorLocation(GetActorLocation() + Direction * Speed / DefaultJumpSpeedRatio);
    }
  }

  if (HasAuthority())
  {
    CharacterLocation = GetActorLocation();
    //UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *GetName(), *ReplicatedData.CharacterLocation.ToString());
  }
}

void AOutcastCharacter::Jump()
{
  if (Role == ROLE_AutonomousProxy)
  {
    if (KeyMap[EKey::Space])
    {
      if (Jumping != EJump::BunnyHop)
      {
        if (JumpHeight < JumpHeightLimit
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
    else if ((Jumping == EJump::Upwards || Jumping == EJump::BunnyHop) && JumpHeight >= MinJumpHeight)
    {
      Jumping = EJump::Downwards;
    }
  }

  Anim->SetIsJumping(Jumping != EJump::NONE);

  if (Jumping == EJump::Upwards)
  {
    JumpHeight = GetActorLocation().Z - JumpStartLocZ;
    Movement->SetMovementMode(MOVE_Flying);

    AddMovementInput(GetActorUpVector(), 5.0f);
  }
  else if (Jumping == EJump::Downwards)
  {
    Movement->SetMovementMode(MOVE_Falling);
  }
  else if (Jumping == EJump::NONE)
  {
    JumpHeight = 0.0f;
    //Movement->SetMovementMode(MOVE_Walking);
  }
  else if (Jumping == EJump::BunnyHop)
  {
    JumpHeight = GetActorLocation().Z - JumpStartLocZ;

    AddMovementInput(GetActorUpVector(), 5.0f);
    Movement->SetMovementMode(MOVE_Flying);
    if (Role == ROLE_AutonomousProxy)
    {
      if (JumpHeight > BunnyHopMaxHeight)
      {
        Jumping = EJump::Upwards;
        BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
      }
    }
  }
}

void AOutcastCharacter::Attack(const float DeltaTime)
{
  if (Role == ROLE_AutonomousProxy)
  {
    // Otherwise SimulatedProxies won't know when the attack animation has finished
    if (Attacking == EAttack::Left)
    {
      Attacking = Anim->GetIsSlashingLeft() ? EAttack::Left : EAttack::NONE;
    }
    else if (Attacking == EAttack::Right)
    {
      Attacking = Anim->GetIsSlashingRight() ? EAttack::Right : EAttack::NONE;
    }
    else if (Attacking == EAttack::Forward)
    {
      Attacking = Anim->GetIsSlashingForward() ? EAttack::Forward : EAttack::NONE;
    }

    if (MouseMap[EMouse::Left])
    {
      if (Attacking == EAttack::NONE)
      {
        // Slashing left/right has priority over forward
        if (KeyMap[EKey::A]
          && !KeyMap[EKey::D])
        {
          Attacking = EAttack::Left;
        }
        else if (!KeyMap[EKey::A]
          && KeyMap[EKey::D])
        {
          Attacking = EAttack::Right;
        }
        else if (!KeyMap[EKey::A]
          && !KeyMap[EKey::D]
          && KeyMap[EKey::W])
        {
          Attacking = EAttack::Forward;
        }
        else
        {
          int Random = FMath::RandRange(1, 3);
          Attacking = static_cast<EAttack>(Random);
        }
      }
    }
  }

  Anim->SetIsSlashingLeft(Attacking == EAttack::Left);
  Anim->SetIsSlashingRight(Attacking == EAttack::Right);
  Anim->SetIsSlashingForward(Attacking == EAttack::Forward);

  // Specify the blend weight for sword attacks/basic movement
  if (Attacking != EAttack::NONE)
  {
    Anim->AddAttackMovementBlendWeight(0.1f);
  }
  else
  {
    Anim->SubtractAttackMovementBlendWeight(0.05f);
  }
}

void AOutcastCharacter::Alive(const float DeltaTime)
{
  if (HasAuthority())
  {
    if (Health == 0)
    {
      Cast<AOutcastGameMode>(GetWorld()->GetAuthGameMode())->Respawn(MyPlayerController);
    }
  }

  if (Role == ROLE_AutonomousProxy)
  {
    for (auto AttackerIt = DamageTakenBy.CreateIterator(); AttackerIt; ++AttackerIt)
    {
      AttackerIt->Value = AttackerIt->Value + DeltaTime;

      if (AttackerIt->Value >= 1.0f)
      {
        AttackerIt->Value = 0.0f;

        if (AttackerIt->Key->GetAttack() == EAttack::NONE)
        {
          Health = FMath::Clamp(Health - 1, 0, 100);
        }
        else
        {
          Health = FMath::Clamp(Health - 20, 0, 100);
        }
      }
    }
  }
}

void AOutcastCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  LookAround();

  MoveAround();

  if (HasAuthority())
  {

    //Jump();

    //Attack(DeltaTime);

    //Alive(DeltaTime);

    FillReplicatedData();
    FillReplicatedAnimData();
  }
  else
  {
    ApplyReplicatedData();
  }

  if (Role == ROLE_SimulatedProxy)
  {
    ApplyReplicatedAnimData();
  }
}

void AOutcastCharacter::SetMyPlayerController(APlayerController* const NewPlayerController)
{
  MyPlayerController = NewPlayerController;
}

EAttack AOutcastCharacter::GetAttack()
{
  return Attacking;
}

void AOutcastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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
  KeyMap.Add(EKey::W, true);
  Server_SetKey(EKey::W, true);
}

void AOutcastCharacter::WReleased()
{
  KeyMap.Add(EKey::W, false);
  Server_SetKey(EKey::W, false);
}

void AOutcastCharacter::APressed()
{
  KeyMap.Add(EKey::A, true);
  Server_SetKey(EKey::A, true);
}

void AOutcastCharacter::AReleased()
{
  KeyMap.Add(EKey::A, false);
  Server_SetKey(EKey::A, false);
}

void AOutcastCharacter::SPressed()
{
  KeyMap.Add(EKey::S, true);
  Server_SetKey(EKey::S, true);
}

void AOutcastCharacter::SReleased()
{
  KeyMap.Add(EKey::S, false);
  Server_SetKey(EKey::S, false);
}

void AOutcastCharacter::DPressed()
{
  KeyMap.Add(EKey::D, true);
  Server_SetKey(EKey::D, true);
}

void AOutcastCharacter::DReleased()
{
  KeyMap.Add(EKey::D, false);
  Server_SetKey(EKey::D, false);
}

void AOutcastCharacter::SpacePressed()
{
  KeyMap.Add(EKey::Space, true);
  Server_SetKey(EKey::Space, true);
}

void AOutcastCharacter::SpaceReleased()
{
  KeyMap.Add(EKey::Space, false);
  Server_SetKey(EKey::Space, false);
}

void AOutcastCharacter::Server_SetKey_Implementation(const EKey Key, const bool bIsPressed)
{
  KeyMap.Add(Key, bIsPressed);

  /*FString TheRole;
  switch (Role)
  {
  case ROLE_Authority:
    TheRole = FString("Authority");
    break;
  case ROLE_AutonomousProxy:
    TheRole = FString("AutonomousProxy");
    break;
  case ROLE_SimulatedProxy:
    TheRole = FString("SimulatedProxy");
    break;
  }
  UE_LOG(LogTemp, Warning, TEXT("Key Hit: %s"), *TheRole)*/
}

bool AOutcastCharacter::Server_SetKey_Validate(const EKey Key, const bool bIsPressed)
{
  return true;
}

void AOutcastCharacter::MouseLeftPressed()
{
  Server_SetMouse(EMouse::Left, true);
}

void AOutcastCharacter::MouseRightPressed()
{
  Server_SetMouse(EMouse::Right, true);
}

void AOutcastCharacter::MouseLeftReleased()
{
  Server_SetMouse(EMouse::Left, false);
}

void AOutcastCharacter::MouseRightReleased()
{
  Server_SetMouse(EMouse::Right, false);
}

void AOutcastCharacter::MouseUpDown(const float AxisValue)
{
  MouseInput.Y = AxisValue;
  Server_SetMouseInput(MouseInput);
}

void AOutcastCharacter::MouseRightLeft(const float AxisValue)
{
  MouseInput.X = AxisValue;
  Server_SetMouseInput(MouseInput);
}

void AOutcastCharacter::Server_SetMouse_Implementation(const EMouse Key, const bool bIsPressed)
{
  MouseMap.Add(Key, bIsPressed);
}

bool AOutcastCharacter::Server_SetMouse_Validate(const EMouse Key, const bool bIsPressed)
{
  return true;
}

void AOutcastCharacter::Server_SetMouseInput_Implementation(const FVector2D NewMouseInput)
{
  MouseInput = NewMouseInput;
}

bool AOutcastCharacter::Server_SetMouseInput_Validate(const FVector2D NewMouseInput)
{
  return true;
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
    if (Role == ROLE_AutonomousProxy)
    {
      if (KeyMap[EKey::Space]
        && (KeyMap[EKey::A] || KeyMap[EKey::D])
        && (Jumping == EJump::Downwards || Jumping == EJump::BunnyHop))
      {
        Jumping = EJump::BunnyHop;
        JumpHeight = 0.0f;
        BunnyHopSpeedRatio = FMath::Clamp(BunnyHopSpeedRatio / 3.0f, BunnyHopFastestSpeedRatio, DefaultJumpSpeedRatio);
      }
      else
      {
        Jumping = EJump::NONE;
        BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
      }
    }
  }
}