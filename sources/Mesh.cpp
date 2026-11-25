// Mesh - P.Ahrenkiel, 2023
#include <cstdlib>
#include <math.h>

#include <queue>
#include <list>
#include <stack>
#include <fstream>
#include <sstream>

#include "Mesh.hpp"

bool Mesh::_showData(false);

//find the WFTIndex in the vector, or push it to the back if not found. Return its vector index.
std::size_t findIndexOrPushVertex(WFTIndexVector &wftIndexVector,const WFTIndex &wftIndex)
{
	bool found=false;
	std::size_t index=0;
	std::size_t nWFT=wftIndexVector.size();
	for(std::size_t i=0;i<nWFT;++i)
	{
		const WFTIndex v=wftIndexVector[i];
		if(wftIndex==v)
		{
			found=true;
			index=i;
			break;
		}
	}
	if(!found)
	{
		index=nWFT;
		wftIndexVector.push_back(wftIndex);
	}
	return index;
}

bool Mesh::Range::isIn(const pnt3 &P)
{
	vtr3 d = _max - _min;
	ags3 box(bas3(d(0)*vtr3::X, d(1)*vtr3::Y, d(2)*vtr3::Z), _min);
	return box.isIn(P);
}

Mesh::Mesh()
{
	_vertex.emplace_back(pnt3(-1.,-1.,0),vtr3::Z,pnt2(0.,1.));
	_vertex.emplace_back(pnt3(1.,-1.,0),vtr3::Z,pnt2(1.,1.));
	_vertex.emplace_back(pnt3(1.,1.,0),vtr3::Z,pnt2(1.,0.));
	_vertex.emplace_back(pnt3(-1.,1.,0),vtr3::Z,pnt2(0.,0.));

	_currentSubmesh=new Submesh(this);
	_submesh["empty"]=_currentSubmesh;
	_currentSubmesh->_tris.emplace_back(0,1,2);
	_currentSubmesh->_tris.emplace_back(0,2,3);
}

Mesh::~Mesh()
{
	clear();
}

//
void Mesh::clear()
{
	for(auto &[key, submesh]:_submesh)
		delete submesh;
	_submesh.clear();
	_vertex.clear();
	_currentSubmesh=nullptr;
}

// Resolve a (possibly negative, i.e. relative) 1-based .obj index to a 0-based index.
static long resolveObjIndex(long i,std::size_t count)
{
	return (i<0)?static_cast<long>(count)+i:i-1;
}

//
bool Mesh::read(const std::string &sPath)
{
	std::ifstream file(sPath);
	if(!file) return false;

	clear();

	std::vector<pnt3> position;
	std::vector<vtr3> normal;
	std::map<std::pair<long,long>,std::size_t> vertexIndex; // (position,normal) -> mesh vertex
	bool ok=true;

	auto vertexFor=[&](const std::string &token)->long
	{
		// token is v, v/vt, v//vn or v/vt/vn; texture index is ignored
		long iv=0,in=0;
		bool hasNormal=false;
		const std::size_t s1=token.find('/');
		iv=std::strtol(token.c_str(),nullptr,10);
		if(s1!=std::string::npos)
		{
			const std::size_t s2=token.find('/',s1+1);
			if(s2!=std::string::npos&&s2+1<token.size())
			{
				in=std::strtol(token.c_str()+s2+1,nullptr,10);
				hasNormal=true;
			}
		}
		const long ip=resolveObjIndex(iv,position.size());
		if(iv==0||ip<0||ip>=static_cast<long>(position.size())) return -1;
		long inorm=-1;
		if(hasNormal)
		{
			inorm=resolveObjIndex(in,normal.size());
			if(in==0||inorm<0||inorm>=static_cast<long>(normal.size())) return -1;
		}
		const auto key=std::make_pair(ip,inorm);
		auto it=vertexIndex.find(key);
		if(it!=vertexIndex.end()) return static_cast<long>(it->second);
		const std::size_t index=_vertex.size();
		_vertex.emplace_back(position[ip],hasNormal?normal[inorm]:vtr3::Z,pnt2(0.,0.));
		vertexIndex[key]=index;
		return static_cast<long>(index);
	};

	auto useSubmesh=[&](const std::string &name)
	{
		auto it=_submesh.find(name);
		if(it==_submesh.end())
		{
			it=_submesh.emplace(name,new Submesh(this)).first;
		}
		_currentSubmesh=it->second;
	};

	std::string line;
	while(ok&&std::getline(file,line))
	{
		std::istringstream ss(line);
		std::string tag;
		ss>>tag;
		if(tag=="v")
		{
			double x,y,z;
			if(ss>>x>>y>>z) position.emplace_back(x,y,z);
			else ok=false;
		}
		else if(tag=="vn")
		{
			double x,y,z;
			if(ss>>x>>y>>z) normal.emplace_back(x,y,z);
			else ok=false;
		}
		else if(tag=="usemtl")
		{
			std::string name;
			std::getline(ss>>std::ws,name);
			while(!name.empty()&&isspace(static_cast<unsigned char>(name.back()))) name.pop_back();
			useSubmesh(name);
		}
		else if(tag=="f")
		{
			std::vector<long> index;
			std::string token;
			while(ss>>token)
			{
				const long i=vertexFor(token);
				if(i<0){ok=false;break;}
				index.push_back(i);
			}
			if(ok&&index.size()<3) ok=false;
			if(!ok) break;
			if(!_currentSubmesh) useSubmesh("default");
			for(std::size_t k=1;k+1<index.size();++k)
			{
				_currentSubmesh->_tris.emplace_back(index[0],index[k],index[k+1]);
			}
		}
	}

	if(!ok)
	{
		clear();
		return false;
	}
	findRange();
	return true;
}
 
void Mesh::operator*=(const atr3 &T)
{
	for(auto &vertex:_vertex)
	{
		pnt3 &P=std::get<0>(vertex);
		P*=T;
		
		vtr3 &nV=std::get<1>(vertex);
		nV*=T.A();
	}
	_range.min()*=T;
	_range.max()*=T;
}

void Mesh::operator/=(const atr3 &T)
{
	*this*=T.inv();
}

void Mesh::findRange()
{
	_range.init();
	for(auto &vertex:_vertex)
	{
		_range.expand(std::get<0>(vertex));
	}
}


// Determine sign +/-1 for edges, +/-0.5 if crossing, 0 if colinear
double Mesh::crossings(pnt3 P, pnt3 Q, TriIndex t, bool &colinear)
{
		colinear = false;
		vtr3 d = Q - P;
		
		pnt3 pt[3];
		for(int i=0;i<3;++i)
		{
			pt[i] = std::get<0>(_vertex[t._p[i]]);
		}
		vtr3 vt[3] = {P - pt[0], P - pt[1], P - pt[2]};
	
		// get signed volumes
		double sv[3] = {
			d.dot(vt[1].cross(vt[2])),
			d.dot(vt[2].cross(vt[0])),
			d.dot(vt[0].cross(vt[1]))
		};

		// count signs
		int nInter = 0;
		int nPos = 0, nNeg = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (sv[i] == 0.) ++nInter;
			else if (sv[i] > 0.) ++nPos;
			else if (sv[i] < 0.) ++nNeg;
		}

		if (nPos > 0 && nNeg > 0) // mixed
		{
			return 0.; //outside
		}
	
		// check if in forward direction
		vtr3 n = (pt[1] - pt[0]).cross(pt[2] - pt[0]);
		vtr3 rc = pt[2] - pnt3::origin;
		double g = n.dot(rc);
		double f = (g - n.dot(P - pnt3::origin))/n.dot(d);
		if (f <= 0.) return 0.; // opposite ray
				
		if (nInter > 1)
		{
			colinear = true;
			return 0.; // intersects two vertices, change q
		}

		if (nInter == 1) { // intersects a shared edge
			return 0.5*mth::sgn(sv[0]);
		}
		
		return mth::sgn(sv[0]);
}

Mesh::Submesh::~Submesh()
{
	delete(_zone);
}

// Count ray crossings w/o BVH
double Mesh::Submesh::crossings(const pnt3 &P)
{
	double nCrossings = 0; // initialize
	if(numTriang() == 0) return 0;
	
	// find initial ray point Q0
	pnt3 Q0;
	vtr3 n; // normal
	for (auto& t: _tris)
	{
		pnt3 p[3] = {std::get<0>(_mesh->_vertex[t._i]), std::get<0>(_mesh->_vertex[t._j]), std::get<0>(_mesh->_vertex[t._k])};
		
		Q0 = p[0] + ((p[1] - p[0]) + (p[2] - p[0]))/3.; // midpoint
		n = (p[1] - p[0]).cross(p[2] - p[0]); // normal
		if (n.cross(Q0 - P).len() != 0.) // accept if not parallel
		{
			n = n.norm();
			break;
		}
	}
	
	vtr3 dQ; // increment along normal
	pnt3 Q;
	bool needQ = true;
	if (_zone) // use BVH
	{
		if (needQ)
		{
			nCrossings = 0.;
			dQ = 1.e-6*(2.*mth::random() - 1.)*n; // new increment
			Q = Q0 + dQ;
			needQ = false;
		}
		bool colinear = false;
		nCrossings = _zone->crossings(P, Q, colinear);
	} else // linear sum
	{
		int i = 0;
		while (i < numTriang())
		{
			if (needQ)
			{
				nCrossings = 0.;
				dQ = 1.e-6*(2.*mth::random() - 1.)*n; // new increment
				Q = Q0 + dQ;
				needQ = false;
			}
			bool colinear = false;
			auto s = _mesh->crossings(P, Q, _tris[i], colinear);
			if (colinear) // restart
			{
				i = 0;
				needQ = true;
			} else {
				nCrossings += s;
				++i;
			}
		}
	}
	return nCrossings != 0.;
}

// Count ray crossings
double Mesh::Submesh::crossings(const pnt3 &P, pnt3 Q, bool &colinear)
{
	double nCrossings = 0; // initialize
	
	int i = 0;
	for (auto& tri: _tris )
	{
		nCrossings += _mesh->crossings(P, Q, tri, colinear);
		if (colinear) return 0;
	}
	return nCrossings;
}

Mesh::Submesh::Zone::~Zone()
{
	for(auto sz: _subzones)
	{
		delete(sz);
	}
}

// Count ray crossings
double Mesh::Submesh::Zone::crossings(pnt3 P, pnt3 Q, bool &colinear)
{
	double s = 0;
	vtr3 d = Q - P;
	auto& C = _range.min();
	auto diag = _range.diag();
	double tLim[2] = {0, std::numeric_limits<double>::infinity()};
	for (int i = 0; i < 3; ++i)
	{
		double x[2] = {C(i), C(i) + diag(i)};
		if (abs(d(i)) < 1.e-12) // parallel
		{
			if (P(i) < x[0] || P(i) > x[1]) { return 0; } // outside
			continue;
		}
		double t[2] = {(x[0] - P(i))/d(i), (x[1] - P(i))/d(i)};
		if (t[0] > t[1]) std::swap(t[0], t[1]);
		tLim[0] = fmax(tLim[0], t[0]);
		tLim[1] = fmin(tLim[1], t[1]);
		if (tLim[0] > tLim[1]) { return 0; } // misses box
	}

	// Ray penetrates box; check triangles
	for (int iTri: _iTris)
	{
		s += _submesh->_mesh->crossings(P, Q, _submesh->_tris[iTri], colinear);
		if (colinear) return 0;
	}

	for (auto& sz: _subzones)
	{
		if (sz) s += sz->crossings(P, Q, colinear);
		if (colinear) return 0;
	}
	return s;
}

bool Mesh::isIn(const pnt3 &P)
{
	double nCrossings = 0; // initialize
	for(auto& sm: _submesh)
	{
		nCrossings += sm.second->crossings(P);
	}
	return nCrossings != 0.;
}


// bounding-volume hierarchy (in progress)
void Mesh::Submesh::buildZones()
{
	_zone = new Submesh::Zone(_mesh->_range, this);
	for (int i = 0; i < _tris.size(); ++i)
	{
		_zone->insert(i);
	}
}

// bounding-volume hierarchy
void Mesh::buildZones()
{
	for (auto sm: _submesh)
	{
		sm.second->buildZones();
	}
}


void Mesh::Submesh::Zone::insert(int iTri)
{
	bool allIn = true;
	auto& tri = _submesh->_tris[iTri];
	const auto& verts = _submesh->_mesh->_vertex;
	pnt3 V[3]{};
	for(int i = 0; i < 3; ++i)
	{
		auto& iVertex = tri._p[i]; // vertex indices
		V[i] = std::get<0>(verts[iVertex]);
		allIn &= _range.isIn(V[i]);// in zone volume
	}

	if (allIn) {
		vtr3 subDiag = _range.diag()/2;
		
		// reject if triangle is somewhat
		if (subDiag.len() > Zone::minDiag) // preset resolution
		{
			auto& C = _range._min;
			vtr3 r[3] = {V[0] - C, V[1] - C, V[2] - C};

			bool first = true;
			int iZone = 0; // invalid
			for (int m = 0; m < 3; ++m)
			{
				int i = floor(2.0*r[m](0)/_range.diag()(0));
				int j = floor(2.0*r[m](1)/_range.diag()(1));
				int k = floor(2.0*r[m](2)/_range.diag()(2));
				if (first)
				{
					iZone = 4*i + 2*j + k;
					first = false;
				}
				else if (iZone != 4*i + 2*j + k) // split across subzones
				{
					_iTris.push_back(iTri); // insert in this zone

					return;
				}
			}
			if (!_subzones[iZone])
			{
				int i = iZone/4;
				int j = (iZone - 4*i)/2;
				int k = iZone - 4*i - 2*j;
				auto C = _range.min() + vtr3(i*subDiag(0), j*subDiag(1), k*subDiag(2));
				_subzones[iZone] = new Zone(Range(C, subDiag), _submesh);
			}
			_subzones[iZone]->insert(iTri);
		}
		else
		{
			_iTris.push_back(iTri);
		}
	}
}


