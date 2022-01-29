#include "ProceduralActor.h"

#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

//Press F1 in game for wireframe view
AProceduralActor::AProceduralActor()
{
	tick = true;
	PrimaryActorTick.bCanEverTick = tick;

	CreateEditorPlaceHolder();
	InitializeInGameMesh();
}


void AProceduralActor::CreateEditorPlaceHolder()
{
	editorMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Placeholder"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("Material'/Game/Materials/Material'"));

	editorMash->SetStaticMesh(meshAsset.Object);
	editorMash->GetStaticMesh()->SetMaterial(0, material.Object);
	
	RootComponent = editorMash;
}

void AProceduralActor::InitializeInGameMesh()
{
    mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    mesh->bUseAsyncCooking = true; // New in UE 4.17, multi-threaded PhysX cooking.
}


void AProceduralActor::BeginPlay()
{
    Super::BeginPlay();

    PrimaryActorTick.SetTickFunctionEnable(tick);

    if (LoadHeightMap()) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Loaded");
        CreateMesh();
    }
}

void AProceduralActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    UnloadHeightMap();
}

void AProceduralActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    currentTime += DeltaTime;

    if (currentTime >= updateInterval)
    {
        currentTime = 0.0f;

        int length = vertices.Num();

        if (!feof(filePtr)) 
        {
            count = fread(rawImage, sizeof(UINT16), KINECT_DEPTH_CAPACITY, filePtr);

            if (count != KINECT_DEPTH_CAPACITY)
            {
                GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("tick: count != imageSize"));
            }

            for (int i = 0; i < length; i++)
            {
                vertices[i].Z = Normalize((float)rawImage[i]) * HeightMultiplicator;
            }

            UpdateMesh();
        }
    }
}


bool AProceduralActor::LoadHeightMap()
{
    const char* filePath = TCHAR_TO_UTF8(*(FPaths::ProjectDir() + fileName));

    // Open the 16 bit raw height map file for reading in binary.
    int error = fopen_s(&filePtr, filePath, "rb");
    if (error != 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("ERR in opening file"));
        return false;
    }

    // Allocate memory for the raw image data.
    rawImage = new UINT16[KINECT_DEPTH_CAPACITY];
    if (!rawImage)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("!rawImage"));
        return false;
    }

    // Read in the raw image data.
    count = fread(rawImage, sizeof(UINT16), KINECT_DEPTH_CAPACITY, filePtr);
    if (count != KINECT_DEPTH_CAPACITY)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("count != imageSize"));
        return false;
    }

    return true;
}

bool AProceduralActor::UnloadHeightMap() {
    // Close the file.
    int error = fclose(filePtr);
    if (error != 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("error during closing"));
        return false;
    }

    // Release the bitmap image data.
    delete[] rawImage;

    rawImage = nullptr;
    filePtr = nullptr;

    return true;
}


void AProceduralActor::CreateMesh()
{
	RootComponent = mesh;
	editorMash->DestroyComponent();

	FVector item;
    for (int j = 0; j < KINECT_DEPTH_HEIGHT; j++)
    {
        for (int i = 0; i < KINECT_DEPTH_WIDTH; i++)
        {
            int index = (KINECT_DEPTH_WIDTH * j) + i;

            item.X = i * LengthMultiplicator;
            item.Y = j * LengthMultiplicator;
            item.Z = Normalize((float)rawImage[index]) * HeightMultiplicator; //static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);
            
            vertices.Add(item);
        }
    }

	UpdateMesh();
}

void AProceduralActor::UpdateMesh()
{
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(KINECT_DEPTH_HEIGHT, KINECT_DEPTH_WIDTH, true, triangles);

	mesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, vertexColors, tangents, true);
	mesh->ContainsPhysicsTriMeshData(true); // Enable collision data
}


inline float AProceduralActor::Normalize(const float value) 
{ 
    return (value - MIN) / (MAX - MIN); 
}
