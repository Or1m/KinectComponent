// Copyright 2021, Bc. Miroslav Kaèeriak. All Rights Reserved.

#include "ProceduralActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>
#include <filesystem>
#include <fstream>
#include <string>

// Sets default values
AProceduralActor::AProceduralActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CreateEditorPlaceHolder();
	InitializeInGameMesh();
}


void AProceduralActor::CreateEditorPlaceHolder()
{
	editorMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Placeholder"));

	ConstructorHelpers::FObjectFinder<UStaticMesh>	meshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	ConstructorHelpers::FObjectFinder<UMaterial>	material(TEXT("Material'/Game/Materials/Material'"));

	editorMash->SetStaticMesh(meshAsset.Object);
	editorMash->GetStaticMesh()->SetMaterial(0, material.Object);
	
	RootComponent = editorMash;
}

void AProceduralActor::InitializeInGameMesh()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	mesh->bUseAsyncCooking = true; // New in UE 4.17, multi-threaded PhysX cooking.
}

void AProceduralActor::CreateMesh()
{
	RootComponent = mesh;
	editorMash->DestroyComponent();

	FString absoluteFilePath = FPaths::ProjectDir() + TEXT("output.txt");
	std::string stringPath = std::string(TCHAR_TO_UTF8(*absoluteFilePath));

	TArray<float> arr;

	std::ifstream file(stringPath);

	if (file.is_open()) 
	{
		std::string line;

		while (std::getline(file, line)) {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, line.c_str());
			arr.Add(std::stof(line));
		}

		file.close();
	}

	FVector item;
	for (int i = 0; i < Width; i++)
	{
		for (int j = 0; j < Length; j++)
		{
			item.X = i * LengthMultiplicator;
			item.Y = j * LengthMultiplicator;
			item.Z = arr[(Width * i ) + j] * HeightMultiplicator; //static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);

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
	mesh->ContainsPhysicsTriMeshData(true); // Enable collision data
}


// Called when the game starts or when spawned
void AProceduralActor::BeginPlay()
{
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "BeginPlay");

	CreateMesh();
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
