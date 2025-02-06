// Copyright Feral Cat Den, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "jc_voronoi.h"

class FJCVoronoiEdge;
class FJCVoronoiSite;
class FJCVoronoiCorner;
class FJCVoronoiDiagram;

class JCVORONOI_API FJCVoronoiEdge
{
public:

	FJCVoronoiEdge(const jcv_edge* InEdge, TSharedRef<const FJCVoronoiDiagram> InDiagram);

	TSharedRef<const FJCVoronoiDiagram> Diagram;
	TSharedPtr<FJCVoronoiCorner> CornerStart;
	TSharedPtr<FJCVoronoiCorner> CornerEnd;

	TArray<TSharedPtr<FJCVoronoiSite>> GetSites() const;
	FVector2D GetStart() const;
	FVector2D GetEnd() const;
	float GetLength() const;

private:

	FVector2D Start;
	FVector2D End;

	const jcv_edge* Edge;

};

class JCVORONOI_API FJCVoronoiCorner
{
public:

	FJCVoronoiCorner(const FVector2D& InLocation)
		: Location(InLocation)
	{

	}

	FVector2D Location;
	TArray<TSharedPtr<FJCVoronoiEdge>> Edges;
	
	TArray<TSharedPtr<FJCVoronoiSite>> GetSites() const;
};

class JCVORONOI_API FJCVoronoiSite
{
public:

	FJCVoronoiSite(const jcv_site* InSite, TSharedRef<const FJCVoronoiDiagram> InDiagram)
		: Diagram(InDiagram)
		, Site(InSite)
	{

	}

	TSharedRef<const FJCVoronoiDiagram> Diagram;
	
	int32 GetIndex() const;
	FVector2D GetCenter() const;
	TArray<TSharedPtr<FJCVoronoiEdge>> GetEdges() const;
	TArray<TSharedPtr<FJCVoronoiSite>> GetNeighbors() const;

private:

	const jcv_site* Site;

};

class JCVORONOI_API FJCVoronoiDiagram : public TSharedFromThis<FJCVoronoiDiagram>
{
public:

	FJCVoronoiDiagram();
	~FJCVoronoiDiagram();
	
	void BuildDiagram(const TArray<FVector2D>& Points, const FBox2D& Bounds);
	void RelaxCorners(float RelaxationAmount = 1.f);
	
	TSharedPtr<FJCVoronoiSite> GetSiteAtLocation(const FVector2D& Location, double Tolerance = 0.1) const;

	TArray<TSharedRef<FJCVoronoiEdge>> Edges;
	TArray<TSharedRef<FJCVoronoiSite>> Sites;
	TArray<TSharedRef<FJCVoronoiCorner>> Corners;

private:
	
	TSharedRef<FJCVoronoiCorner> MakeCorner(const FVector2D& Location);

	TMap<uintptr_t, TSharedRef<FJCVoronoiEdge>> EdgesByAddress;
	TMap<int32, TSharedRef<FJCVoronoiSite>> SitesByIndex;
	TMap<FIntPoint, TSharedRef<FJCVoronoiCorner>> CornersByLocation;

	jcv_diagram Diagram;

	friend class FJCVoronoiEdge;
	friend class FJCVoronoiSite;
	friend class FJCVoronoiCorner;

};
