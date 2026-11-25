// unit test
#include "unit-test.hpp"

#include "TopoMesh.hpp"

const std::string sBox = std::string(TOPOMESH_TEST_DATA_DIR) + "/Box/Box.obj";
const std::string sBoxHole = std::string(TOPOMESH_TEST_DATA_DIR) + "/BoxHole/BoxHole.obj";

/*
 * TopoMesh tests
 */
TEST(TopoMeshTest, is_in_box)
{
	//read file sBox
	Mesh boxMesh;

	ASSERT_TRUE(boxMesh.read(sBox));
	boxMesh.buildBVH();
	EXPECT_EQ(boxMesh.numVertex(), 24u);
	
	EXPECT_EQ(boxMesh.isIn(pnt3(150., 0., 0.)), false);
	EXPECT_EQ(boxMesh.isIn(pnt3(0., 0., 0.)), false);
	
	EXPECT_EQ(boxMesh.isIn(pnt3(0., 50., 0.)), true);
	EXPECT_EQ(boxMesh.isIn(pnt3(20., 80., -30.)), true);
	EXPECT_EQ(boxMesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(boxMesh.isIn(pnt3(0., -10., 0.)), false);

}

TEST(TopoMeshTest, is_in_boxhole)
{
	//read file sBoxHole
	Mesh boxMesh;

	ASSERT_TRUE(boxMesh.read(sBoxHole));
	EXPECT_EQ(boxMesh.numVertex(), 80u);
	
	EXPECT_EQ(boxMesh.isIn(pnt3(150., 0., 0.)), false);

	EXPECT_EQ(boxMesh.isIn(pnt3(50., 50., 50.)), false);
	EXPECT_EQ(boxMesh.isIn(pnt3(20., 80., -30.)), true);
	EXPECT_EQ(boxMesh.isIn(pnt3(0., 150., 0.)), false);
	EXPECT_EQ(boxMesh.isIn(pnt3(0., -10., 0.)), false);

}
