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
	editorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Placeholder"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("Material'/Game/Materials/Material'"));

	editorMesh->SetStaticMesh(meshAsset.Object);
	editorMesh->GetStaticMesh()->SetMaterial(0, material.Object);
	
	RootComponent = editorMesh;
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
        int length = terrainVertices.Num();

        if (!feof(filePtr)) 
        {
            count = fread(rawImage, sizeof(UINT16), KINECT_DEPTH_CAPACITY, filePtr);

            if (count != KINECT_DEPTH_CAPACITY)
            {
                GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("tick: count != imageSize"));
                return;
            }

            UpdateTerrainHeight();
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
	editorMesh->DestroyComponent();

    FVector terrain3DPoint;
    for (int j = 0; j < TerrainHeight; j++)
    {
        for (int i = 0; i < TerrainWidth; i++)
        {
            terrain3DPoint.X = i * LengthMultiplicator;
            terrain3DPoint.Y = j * LengthMultiplicator;
            terrain3DPoint.Z = 0;

            terrainVertices.Add(terrain3DPoint);
        }
    }

	UpdateMesh();
}

void AProceduralActor::UpdateMesh()
{
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(TerrainHeight, TerrainWidth, true, triangles);

	mesh->CreateMeshSection_LinearColor(0, terrainVertices, triangles, normals, uvs, vertexColors, tangents, true);
	mesh->ContainsPhysicsTriMeshData(true); // Enable collision data
}

void AProceduralActor::UpdateTerrainHeight()
{
    int k = 0;
    for (int j = 0; j < TerrainHeight; j++)
    {
        for (int i = 0; i < TerrainWidth; i++)
        {
            float gx = i / float(TerrainWidth) * KINECT_DEPTH_WIDTH;
            float gy = j / float(TerrainHeight) * KINECT_DEPTH_HEIGHT;
            int gxi = int(gx);
            int gyi = int(gy);

            const UINT16& c00 = rawImage[gyi * KINECT_DEPTH_WIDTH + gxi];
            const UINT16& c10 = rawImage[gyi * KINECT_DEPTH_WIDTH + (gxi + 1)];
            const UINT16& c01 = rawImage[(gyi + 1) * KINECT_DEPTH_WIDTH + gxi];
            const UINT16& c11 = rawImage[(gyi + 1) * KINECT_DEPTH_WIDTH + (gxi + 1)];

            terrainVertices[k++].Z = Normalize(Bilinear(gx - gxi, gy - gyi, c00, c10, c01, c11)) * HeightMultiplicator;
        }
    }
}


inline float AProceduralActor::Normalize(const float value) 
{ 
    return (value - MIN) / (MAX - MIN); 
}

inline float AProceduralActor::Bilinear(const float& tx, const float& ty, const UINT16& c00, const UINT16& c10, const UINT16& c01, const UINT16& c11)
{
#if 1
    float a = c00 * (1.f - tx) + c10 * tx;
    float b = c01 * (1.f - tx) + c11 * tx;
    return a * (1.f - ty) + b * ty;
#else 
    return (1 - tx) * (1 - ty) * c00 +
        tx * (1 - ty) * c10 +
        (1 - tx) * ty * c01 +
        tx * ty * c11;
#endif 
}
