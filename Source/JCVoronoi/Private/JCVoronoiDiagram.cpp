// Copyright Feral Cat Den, LLC. All Rights Reserved.

#include "JCVoronoiDiagram.h"

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

#define JC_VORONOI_NOTUSED(v) do { (void)(1 ? (void)0 : ( (void)(v) ) ); } while(0)

void* HeapAlloc(void* userData, size_t size)
{
	JC_VORONOI_NOTUSED(userData);
	return FMemory::Malloc(size);
}

void HeapFree(void* userData, void* ptr)
{
	JC_VORONOI_NOTUSED(userData);
	FMemory::Free(ptr);
}

FJCVoronoiEdge::FJCVoronoiEdge(const jcv_edge* InEdge, TSharedRef<const FJCVoronoiDiagram> InDiagram)
	: Diagram(InDiagram)
	, Edge(InEdge)
{
	Start = FVector2D(Edge->pos[0].x, Edge->pos[0].y);
	End = FVector2D(Edge->pos[1].x, Edge->pos[1].y);
}

TArray<TSharedPtr<FJCVoronoiSite>> FJCVoronoiEdge::GetSites() const
{
	TArray<TSharedPtr<FJCVoronoiSite>> Results;
	if (Edge->sites[0])
	{
		Results.Add(Diagram->SitesByIndex[Edge->sites[0]->index]);
	}
	if (Edge->sites[1])
	{
		Results.Add(Diagram->SitesByIndex[Edge->sites[1]->index]);
	}

	return Results;
}

FVector2D FJCVoronoiEdge::GetStart() const
{
	return CornerStart.IsValid() ? CornerStart->Location : Start;
}

FVector2D FJCVoronoiEdge::GetEnd() const
{
	return CornerEnd.IsValid() ? CornerEnd->Location : End;
}

float FJCVoronoiEdge::GetLength() const
{
	return FVector2D::Distance(CornerStart->Location, CornerEnd->Location);
}

int32 FJCVoronoiSite::GetIndex() const
{
	return Site->index;
}

FVector2D FJCVoronoiSite::GetCenter() const
{
	return FVector2D(Site->p.x, Site->p.y);
}

TArray<TSharedPtr<FJCVoronoiEdge>> FJCVoronoiSite::GetEdges() const
{
	TArray<TSharedPtr<FJCVoronoiEdge>> Results;
	const jcv_graphedge* GraphEdge = Site->edges;
	while (GraphEdge)
	{
		Results.Add(Diagram->EdgesByAddress[reinterpret_cast<uintptr_t>(GraphEdge->edge)]);
		GraphEdge = GraphEdge->next;
	}

	return Results;
}

TArray<TSharedPtr<FJCVoronoiSite>> FJCVoronoiSite::GetNeighbors() const
{
	TArray<TSharedPtr<FJCVoronoiSite>> Results;
	const jcv_graphedge* GraphEdge = Site->edges;
	while (GraphEdge)
	{
		if (GraphEdge->neighbor)
		{
			Results.Add(Diagram->SitesByIndex[GraphEdge->neighbor->index]);
		}

		GraphEdge = GraphEdge->next;
	}

	return Results;
}

TArray<TSharedPtr<FJCVoronoiSite>> FJCVoronoiCorner::GetSites() const
{
	TArray<TSharedPtr<FJCVoronoiSite>> Results;
	for (const TSharedPtr<FJCVoronoiEdge>& Edge : Edges)
	{
		for (TSharedPtr<FJCVoronoiSite> Site : Edge->GetSites())
		{
			Results.AddUnique(Site);
		}
	}

	return Results;
}

FJCVoronoiDiagram::FJCVoronoiDiagram()
{
	FMemory::Memset(&Diagram, 0, sizeof(jcv_diagram));
}

FJCVoronoiDiagram::~FJCVoronoiDiagram()
{
	Edges.Empty();
	Sites.Empty();
	Corners.Empty();
	EdgesByAddress.Empty();
	SitesByIndex.Empty();
	CornersByLocation.Empty();

	jcv_diagram_free(&Diagram);
}

void FJCVoronoiDiagram::BuildDiagram(const TArray<FVector2D>& Points, const FBox2D& Bounds)
{
	// Create point structures
	TArray<jcv_point> JCVPoints;
	JCVPoints.AddZeroed(Points.Num());
	for (int32 PointIndex = 0; PointIndex < Points.Num(); ++PointIndex)
	{
		JCVPoints[PointIndex].x = Points[PointIndex].X;
		JCVPoints[PointIndex].y = Points[PointIndex].Y;
	}
	
	// Create the bounding box structure
	jcv_rect JCVBounds;
	JCVBounds.min.x = Bounds.Min.X;
	JCVBounds.min.y = Bounds.Min.Y;
	JCVBounds.max.x = Bounds.Max.X;
	JCVBounds.max.y = Bounds.Max.Y;

	// Generate the diagram
	jcv_diagram_generate_useralloc(Points.Num(), JCVPoints.GetData(), &JCVBounds, nullptr, nullptr, HeapAlloc, HeapFree, &Diagram);
	
	// Extract the sites array
	const jcv_site* JCVSites = jcv_diagram_get_sites(&Diagram);
	for (int32 SiteIndex = 0; SiteIndex < Diagram.numsites; SiteIndex++)
	{
		TSharedRef<FJCVoronoiSite> Site = MakeShared<FJCVoronoiSite>(&JCVSites[SiteIndex], AsShared());
		Sites.Add(Site);
		SitesByIndex.Add(JCVSites[SiteIndex].index, Site);
	}

	// Extract the edge and corner arrays
	const jcv_edge* JCVEdge = jcv_diagram_get_edges(&Diagram);
	while (JCVEdge)
	{
		if (JCVEdge->pos[0].x != JCVEdge->pos[1].x || JCVEdge->pos[0].y != JCVEdge->pos[1].y)
		{
			TSharedRef<FJCVoronoiEdge> Edge = MakeShared<FJCVoronoiEdge>(JCVEdge, AsShared());
			
			Edge->CornerStart = MakeCorner(Edge->GetStart());
			Edge->CornerStart->Edges.AddUnique(Edge);
			
			Edge->CornerEnd = MakeCorner(Edge->GetEnd());
			Edge->CornerEnd->Edges.AddUnique(Edge);

			Edges.Add(Edge);
			EdgesByAddress.Add(reinterpret_cast<uintptr_t>(JCVEdge), Edge);
		}

		JCVEdge = JCVEdge->next;
	}
}

TSharedRef<FJCVoronoiCorner> FJCVoronoiDiagram::MakeCorner(const FVector2D& Location)
{
	// Reuse existing corners if one exists in the desired location already. Compare corners via their integer locations, which avoids floating-point precision issues.
	// Multiplying by a 1eN constant provides an additional N digits of precision.
	FIntPoint IntLocation = (Location * 1e1).IntPoint(); 
	if (CornersByLocation.Contains(IntLocation))
	{
		return CornersByLocation[IntLocation];
	}

	TSharedRef<FJCVoronoiCorner> Corner = MakeShared<FJCVoronoiCorner>(Location);
	CornersByLocation.Add(IntLocation, Corner);
	Corners.Add(Corner);

	return Corner;
}

void FJCVoronoiDiagram::RelaxCorners(float RelaxationAmount)
{
	if (RelaxationAmount > 0.f)
	{
		// Move corners towards the average of the voronoi sites around them. This results in more uniform edge lengths.
		for (int32 CornerIndex = 0; CornerIndex < Corners.Num(); CornerIndex++)
		{
			TSharedRef<FJCVoronoiCorner> Corner = Corners[CornerIndex];
			FVector2D RelaxedLocation = FVector2D::ZeroVector;
			TArray<TSharedPtr<FJCVoronoiSite>> CornerSites = Corner->GetSites();
			for (auto Site : CornerSites)
			{
				RelaxedLocation += Site->GetCenter();
			}

			RelaxedLocation /= CornerSites.Num();
			Corner->Location = FMath::Lerp(Corner->Location, RelaxedLocation, RelaxationAmount);
		}
	}
}

TSharedPtr<FJCVoronoiSite> FJCVoronoiDiagram::GetSiteAtLocation(const FVector2D& Location, double Tolerance) const
{
	for (TSharedPtr<FJCVoronoiSite> Site : Sites)
	{
		if (Site->GetCenter().Equals(Location, Tolerance))
		{
			return Site;
		}
	}

	return nullptr;
}
