
#include <list>
#include "ShapeToTriangulation.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

// 将TopoDS_Shape转换为Poly_Triangulation
Handle(Poly_Triangulation) ShapeToTriangulation(TopoDS_Shape aShape)
{
	clock_t time = clock();
	Standard_Integer aNbNodes = 0;
	Standard_Integer aNbTriangles = 0;

	// calculate total number of the nodes and triangles
	for (TopExp_Explorer anExpSF(aShape, TopAbs_FACE); anExpSF.More(); anExpSF.Next())
	{
		TopLoc_Location aLoc;
		const TopoDS_Face& face = TopoDS::Face(anExpSF.Current());
		Handle(Poly_Triangulation) aTriangulation = BRep_Tool::Triangulation(face, aLoc);
		if (aTriangulation.IsNull())
		{
			TopoDS_Shape shape = *(TopoDS_Shape*)&face;
			BRepMesh_IncrementalMesh mesh(shape, 1e-1, true, 0.5, true);
			mesh.Perform();

			aTriangulation = BRep_Tool::Triangulation(face, aLoc);
		}

		if (!aTriangulation.IsNull())
		{
			aNbNodes += aTriangulation->NbNodes();
			aNbTriangles += aTriangulation->NbTriangles();
		}
		else
		{
			std::cout << "empty face" << std::endl;
		}
	}

	// create temporary triangulation
	// Handle包裹的对象是自动进行引用计数以及自动释放的
	Handle(Poly_Triangulation) aMesh = new Poly_Triangulation(aNbNodes, aNbTriangles, Standard_False);

	// fill temporary triangulation
	Standard_Integer aNodeOffset = 0;
	Standard_Integer aTriangleOffet = 0;
	for (TopExp_Explorer anExpSF(aShape, TopAbs_FACE); anExpSF.More(); anExpSF.Next())
	{
		TopLoc_Location aLoc;
		Handle(Poly_Triangulation) aTriangulation = BRep_Tool::Triangulation(TopoDS::Face(anExpSF.Current()), aLoc);

		if (aTriangulation.IsNull()) continue;

		const TColgp_Array1OfPnt& aNodes = aTriangulation->Nodes();
		const Poly_Array1OfTriangle& aTriangles = aTriangulation->Triangles();

		// copy nodes
		gp_Trsf aTrsf = aLoc.Transformation();
		for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
		{
			gp_Pnt aPnt = aNodes(aNodeIter);
			aPnt.Transform(aTrsf);
			aMesh->ChangeNode(aNodeIter + aNodeOffset) = aPnt;
		}

		// copy triangles
		const TopAbs_Orientation anOrientation = anExpSF.Current().Orientation();
		for (Standard_Integer aTriIter = aTriangles.Lower(); aTriIter <= aTriangles.Upper(); ++aTriIter)
		{
			Poly_Triangle aTri = aTriangles(aTriIter);

			Standard_Integer anId[3];
			aTri.Get(anId[0], anId[1], anId[2]);
			if (anOrientation == TopAbs_REVERSED)
			{
				// Swap 1, 2.
				Standard_Integer aTmpIdx = anId[1];
				anId[1] = anId[2];
				anId[2] = aTmpIdx;
			}

			// Update nodes according to the offset.
			anId[0] += aNodeOffset;
			anId[1] += aNodeOffset;
			anId[2] += aNodeOffset;

			aTri.Set(anId[0], anId[1], anId[2]);
			aMesh->ChangeTriangle(aTriIter + aTriangleOffet) = aTri;
		}

		aNodeOffset += aNodes.Size();
		aTriangleOffet += aTriangles.Size();
	}

	std::cout << "ShapeToTriangulation: " << (clock() - time) / 1000.0 << "s" << std::endl;
	return aMesh;
}
