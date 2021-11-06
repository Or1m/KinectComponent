// Copyright 2021, Bc. Miroslav Kaèeriak. All Rights Reserved.

#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>

#include "ProceduralActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AProceduralActor::AProceduralActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	UStaticMeshComponent* SphereComponent = CreateEditorOnlyDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	//SphereComponent->InitSphereRadius(30);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	SphereComponent->SetStaticMesh(SphereMeshAsset.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> plane_material(TEXT("Material'/Game/Material'"));
	SphereComponent->GetStaticMesh()->SetMaterial(0, plane_material.Object);

	RootComponent = SphereComponent;

	//RootComponent->Rotat(FRotator(90.0f));

	//RootComponent->SetWorldLocationAndRotation(RootComponent->GetComponentLocation(), FRotator(-90.0f));


	//UStaticMeshComponent* SphereComponent = CreateEditorOnlyDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	////SphereComponent->InitSphereRadius(30);
	//static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	//SphereComponent->SetStaticMesh(SphereMeshAsset.Object);

	//static ConstructorHelpers::FObjectFinder<UMaterial> plane_material(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial'"));
	//SphereComponent->GetStaticMesh()->SetMaterial(0, plane_material.Object);


	//RootComponent = SphereComponent;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));

	// New in UE 4.17, multi-threaded PhysX cooking.
	mesh->bUseAsyncCooking = true;
}

void AProceduralActor::InitializeMesh()
{
	RootComponent = mesh;

	FVector item;
	for (int i = -Width; i <= Width; i++)
	{
		for (int j = -Length; j <= Length; j++)
		{
			item.X = i * LengthMultiplicator;
			item.Y = j * LengthMultiplicator;
			item.Z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);

			vertices.Add(item);
		}
	}

	UpdateMesh();
}

void AProceduralActor::UpdateMesh()
{
	int numXY = (Width * 2) + 1;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(numXY, numXY, false, triangles);

	mesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}


// Called when the game starts or when spawned
void AProceduralActor::BeginPlay()
{
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "BeginPlay");

	InitializeMesh();
}

// Called every frame
void AProceduralActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	currentTime += DeltaTime;

	if (currentTime >= interval)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Tick"));
		currentTime = 0.0f;

		int length = vertices.Num();

		for (int i = 0; i < length; i++)
		{
			vertices[i].Z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);
		}

		UpdateMesh();
	}
}
