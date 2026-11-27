// unit test

#include <random>
#include <chrono>

#include "unit-test.hpp"

#include "TopoMesh.hpp"

const std::string sBox = std::string(TOPOMESH_TEST_DATA_DIR) + "/Box/Box.obj";
const std::string sBoxHole = std::string(TOPOMESH_TEST_DATA_DIR) + "/BoxHole/BoxHole.obj";
const std::string sTorus = std::string(TOPOMESH_TEST_DATA_DIR) + "/Torus/Torus.obj";
const std::string sShell = std::string(TOPOMESH_TEST_DATA_DIR) + "/Shell/Shell.obj";

static const int nSpeedIters = 2;
static const std::vector<pnt3>& rangePoints(const Mesh::Range& range)
{
	static const std::vector<pnt3> Ps = [&range]
	{
		double w[3]{};
		for (int i = 0; i < 3; ++i) w[i] = range.extent(i);
		pnt3 Pmin = range.min();
		std::vector<pnt3> v{};
		v.reserve(1000);
		for (int i = 0; i < 1000; ++i)
		{
			double x = Pmin.x() + mth::random()*w[0];
			double y = Pmin.y() + mth::random()*w[1];
			double z = Pmin.z() + mth::random()*w[2];
			v.emplace_back(x, y, z);
		}
		return v;
	}();                                                // built once, on first use
	return Ps;
}

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

TEST(TopoMeshTest, is_in_box_outershell_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	mesh.buildZones();

	auto& Ps = rangePoints(mesh._range);

	for (auto& P: Ps)
	{
		vtr3 r = P - pnt3::origin;
		
		bool in = (std::abs(r.x()) < 40.) && (std::abs(r.y()) < 40.) && (std::abs(r.z()) < 40.);
		bool expRes = in;
		
		EXPECT_EQ(mesh.isIn(P, {"outer_box"}), expRes) << P;
	}
}

TEST(TopoMeshTest, is_in_box_shell_no_build)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	
	auto& Ps = rangePoints(mesh._range);
	for (int i = 0; i < nSpeedIters; ++i)
		for (auto& P: Ps)
		{
			vtr3 r = P - pnt3::origin;
			bool inX = ((std::abs(r.x()) > 20.) && (std::abs(r.x()) < 40.)) && (std::abs(r.y()) < 40.) && (std::abs(r.z()) < 40.);
			bool inY = (std::abs(r.x()) < 40.) && ((std::abs(r.y()) > 20.) && (std::abs(r.y()) < 40.)) && (std::abs(r.z()) < 40.);
			bool inZ = (std::abs(r.x()) < 40.) && (std::abs(r.y()) < 40.) && ((std::abs(r.z()) > 20.) && (std::abs(r.z()) < 40.));
			bool expRes = inX || inY || inZ;
			
			EXPECT_EQ(mesh.isIn(P), expRes) << P;
		}

}

TEST(TopoMeshTest, is_in_box_shell_build)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	
	auto& Ps = rangePoints(mesh._range);
	for (int i = 0; i < nSpeedIters; ++i)
		for (auto& P: Ps)
		{
			vtr3 r = P - pnt3::origin;
			bool inX = ((std::abs(r.x()) > 20.) && (std::abs(r.x()) < 40.)) && (std::abs(r.y()) < 40.) && (std::abs(r.z()) < 40.);
			bool inY = (std::abs(r.x()) < 40.) && ((std::abs(r.y()) > 20.) && (std::abs(r.y()) < 40.)) && (std::abs(r.z()) < 40.);
			bool inZ = (std::abs(r.x()) < 40.) && (std::abs(r.y()) < 40.) && ((std::abs(r.z()) > 20.) && (std::abs(r.z()) < 40.));
			bool expRes = inX || inY || inZ;
			
			EXPECT_EQ(mesh.isIn(P), expRes) << P;
		}
	
	mesh.buildZones();
}

TEST(TopoMeshTest, is_in_box_shell_BVH)
{
	Mesh mesh;
	ASSERT_TRUE(mesh.read(sShell));
	mesh.buildZones();

	auto& Ps = rangePoints(mesh._range);

	for (int i = 0; i < 2; ++i)
		for (auto& P: Ps)
		{
			vtr3 r = P - pnt3::origin;
			
			bool inX = ((std::abs(r.x()) > 20.) && (std::abs(r.x()) < 40.)) && (std::abs(r.y()) < 40.) && (std::abs(r.z()) < 40.);
			bool inY = (std::abs(r.x()) < 40.) && ((std::abs(r.y()) > 20.) && (std::abs(r.y()) < 40.)) && (std::abs(r.z()) < 40.);
			bool inZ = (std::abs(r.x()) < 40.) && (std::abs(r.y()) < 40.) && ((std::abs(r.z()) > 20.) && (std::abs(r.z()) < 40.));
			
			bool expRes = inX || inY || inZ;
			
			EXPECT_EQ(mesh.isIn(P), expRes) << P;
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
