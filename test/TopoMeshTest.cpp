// unit test
#include "unit-test.hpp"

#include "TopoMesh.hpp"

const std::string sBox = std::string(TOPOMESH_TEST_DATA_DIR) + "/Box/Box.obj";
const std::string sBoxHole = std::string(TOPOMESH_TEST_DATA_DIR) + "/BoxHole/BoxHole.obj";
const std::string sTorus = std::string(TOPOMESH_TEST_DATA_DIR) + "/Torus/Torus.obj";
const std::string sShell = std::string(TOPOMESH_TEST_DATA_DIR) + "/Shell/Shell.obj";

/*
 * TopoMesh tests
 */
TEST(TopoMeshTest, is_in_box_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sBox));
	
	EXPECT_EQ(mesh.numVertex(), 24u);
	
	EXPECT_EQ(mesh.isIn(pnt3(150., 0., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., 1., 0.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(0., -1., 0.)), false);

	EXPECT_EQ(mesh.isIn(pnt3(0., 50., 0.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(20., 80., -30.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., -10., 0.)), false);

}

TEST(TopoMeshTest, is_in_boxhole)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sBoxHole));

	EXPECT_EQ(mesh.numVertex(), 80u);
	
	EXPECT_EQ(mesh.isIn(pnt3(150., 0., 0.)), false);

	EXPECT_EQ(mesh.isIn(pnt3(50., 49., 50.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(20., 80., -30.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., -10., 0.)), false);

}

TEST(TopoMeshTest, is_in_boxhole_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sBoxHole));
	mesh.buildZones();
	
	EXPECT_EQ(mesh.numVertex(), 80u);
	
	EXPECT_EQ(mesh.isIn(pnt3(150., 0., 0.)), false);

	EXPECT_EQ(mesh.isIn(pnt3(50., 49., 50.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(20., 80., -30.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., -10., 0.)), false);
}

TEST(TopoMeshTest, is_in_torus)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sTorus));
	
	EXPECT_EQ(mesh.numVertex(), 32768u);
	EXPECT_EQ(mesh.isIn(pnt3(150., 0., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(50., 50., 50.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(50., 20., 0.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(0., 20., 50.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(50., 20., 50.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., -10., 0.)), false);

}

TEST(TopoMeshTest, is_in_torus_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sTorus));
	mesh.buildZones();
	
	EXPECT_EQ(mesh.isIn(pnt3(150., 0., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(50., 50., 50.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(50., 20., 0.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(0., 20., 50.)), true);
	EXPECT_EQ(mesh.isIn(pnt3(50., 20., 50.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(mesh.isIn(pnt3(0., -10., 0.)), false);
}

/*
TEST(TopoMeshTest, is_in_shell_random_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	mesh.buildZones();
	
	int i = 0;
	while (i < 10000)
	{

		pnt3 P(200.*(mth::random() - 0.5), 200.*(mth::random() - 0.5), 200.*(mth::random() - 0.5));
		double r = (P - pnt3::origin).len();
		
		if ((std::abs(r - 25.) < 0.1) || (std::abs(r - 50.) < 0.1)) continue;
		
		EXPECT_EQ(mesh.isIn(P), (r > 25.) && (r < 50.)) << P << ", " << r;
		++i;
	}
}
*/

TEST(TopoMeshTest, is_in_box_shell_random_build_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	
	int i = 0;
	while (i < 10000)
	{
		pnt3 P(100.*(mth::random() - 0.5), 100.*(mth::random() - 0.5), 100.*(mth::random() - 0.5));
		vtr3 r = P - pnt3::origin;
		
		bool inX = ((std::abs(r.x()) > 20.) && (std::abs(r.x()) < 40.)) && (std::abs(r.y()) < 40.) && (std::abs(r.z()) < 40.);
		bool inY = (std::abs(r.x()) < 40.) && ((std::abs(r.y()) > 20.) && (std::abs(r.y()) < 40.)) && (std::abs(r.z()) < 40.);
		bool inZ = (std::abs(r.x()) < 40.) && (std::abs(r.y()) < 40.) && ((std::abs(r.z()) > 20.) && (std::abs(r.z()) < 40.));
		
		bool expRes = inX || inY || inZ;
		
		EXPECT_EQ(mesh.isIn(P), expRes) << P;
		++i;
	}
	mesh.buildZones();

}

TEST(TopoMeshTest, is_in_box_shell_random_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	mesh.buildZones();
	
	int i = 0;
	while (i < 10000)
	{

		pnt3 P(100.*(mth::random() - 0.5), 100.*(mth::random() - 0.5), 100.*(mth::random() - 0.5));
		vtr3 r = P - pnt3::origin;
		
		bool inX = ((std::abs(r.x()) > 20.) && (std::abs(r.x()) < 40.)) && (std::abs(r.y()) < 40.) && (std::abs(r.z()) < 40.);
		bool inY = (std::abs(r.x()) < 40.) && ((std::abs(r.y()) > 20.) && (std::abs(r.y()) < 40.)) && (std::abs(r.z()) < 40.);
		bool inZ = (std::abs(r.x()) < 40.) && (std::abs(r.y()) < 40.) && ((std::abs(r.z()) > 20.) && (std::abs(r.z()) < 40.));
		
		bool expRes = inX || inY || inZ;
		
		EXPECT_EQ(mesh.isIn(P), expRes) << P;
		++i;
	}
}


TEST(TopoMeshTest, is_in_torus_speed)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sTorus));
	
	// iterate to compare speed w/o BVH
	for (int i = 0; i < 1000; ++i)
	{
		mesh.isIn(pnt3(50., 20., 0.));
		mesh.isIn(pnt3(0., 20., 50.));
		mesh.isIn(pnt3(50., 20., 50.));
	}
}

TEST(TopoMeshTest, is_in_torus_BVH_speed)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sTorus));
	mesh.buildZones();
	
	// iterate to compare speed w BVH
	for (int i = 0; i < 1000; ++i)
	{
		mesh.isIn(pnt3(50., 20., 0.));
		mesh.isIn(pnt3(0., 20., 50.));
		mesh.isIn(pnt3(50., 20., 50.));
	}
}

TEST(TopoMeshTest, is_in_torus_BVH_build_speed)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sTorus));
	
	// iterate to compare speed w/o BVH
	for (int i = 0; i < 1000; ++i)
	{
		mesh.isIn(pnt3(50., 20., 0.));
		mesh.isIn(pnt3(0., 20., 50.));
		mesh.isIn(pnt3(50., 20., 50.));
	}
	mesh.buildZones();
}
