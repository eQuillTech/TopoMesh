// unit test
#include "unit-test.hpp"

#include "TopoMesh.hpp"

const std::string sBox = std::string(TOPOMESH_TEST_DATA_DIR) + "/Box/Box.obj";
const std::string sBoxHole = std::string(TOPOMESH_TEST_DATA_DIR) + "/BoxHole/BoxHole.obj";
const std::string sTorus = std::string(TOPOMESH_TEST_DATA_DIR) + "/Torus/Torus.obj";

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
	
	EXPECT_EQ(mesh.numVertex(), 8192u);
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

TEST(TopoMeshTest, is_in_torus_speed)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sTorus));
	
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
	
	for (int i = 0; i < 1000; ++i)
	{
		mesh.isIn(pnt3(50., 20., 0.));
		mesh.isIn(pnt3(0., 20., 50.));
		mesh.isIn(pnt3(50., 20., 50.));
	}}
