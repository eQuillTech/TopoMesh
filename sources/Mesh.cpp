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

//
Mesh::Mesh()
{
	_vertex.emplace_back(pnt3(-1.,-1.,0),vtr3::Z,pnt2(0.,1.));
	_vertex.emplace_back(pnt3(1.,-1.,0),vtr3::Z,pnt2(1.,1.));
	_vertex.emplace_back(pnt3(1.,1.,0),vtr3::Z,pnt2(1.,0.));
	_vertex.emplace_back(pnt3(-1.,1.,0),vtr3::Z,pnt2(0.,0.));

	_currentSubmesh=new SubMesh();
	_submesh["empty"]=_currentSubmesh;
	_currentSubmesh->_tris.emplace_back(0,1,2);
	_currentSubmesh->_tris.emplace_back(0,2,3);
}

//
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
			it=_submesh.emplace(name,new SubMesh()).first;
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
	FindRange();
	return true;
}

// 
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

void Mesh::FindRange()
{
	_range.init();
	for(auto &vertex:_vertex)
	{
		_range.expand(std::get<0>(vertex));
	}
}

// Determine sign +/-1, if crossing, +/-0.5 for edges, if not colinear
float Mesh::intersectSign(pnt3 p, pnt3 q, TriIndex t, bool &colinear)
{
		colinear = false;
		vtr3 pq = p - q;
		
		pnt3 pt[3];
		for(int i=0;i<3;++i)
		{
			pt[i] = std::get<0>(_vertex[t._p[i]]);
		}
		vtr3 vt[3] = {p - pt[0], p - pt[1], p - pt[2]};
		double sv[3] = {
			pq.dot(vt[1].cross(vt[2])),
			pq.dot(vt[2].cross(vt[0])),
			pq.dot(vt[0].cross(vt[1]))
		};

		// signed volumes
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
	
		vtr3 n = (pt[1] - pt[0]).cross(pt[2] - pt[0]);
		vtr3 rc = pt[2] - pnt3::origin;
		double d = n.dot(rc);
		double f = (d - n.dot(p - pnt3::origin))/n.dot(pq);
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

bool Mesh::isIn(const pnt3 &P)
{
	double nIntersect = 0; // initialize
	for(auto& sp: _submesh)
	{
		auto sm = sp.second;
		if(sm->numTriang() == 0) continue;
		auto t0 = sm->_tris[0];
		pnt3 p[3] = {std::get<0>(_vertex[t0._i]), std::get<0>(_vertex[t0._j]), std::get<0>(_vertex[t0._k])};
		
		const pnt3 q0 = p[0] + ((p[1] - p[0]) + (p[2] - p[0]))/3.; //
		pnt3 q = q0;
		vtr3 n = (p[1] - p[0]).cross(p[2] - p[0]);
		
		int i = 0;
		while (i < sm->numTriang())
		{
			bool colinear = false;
			auto s = intersectSign(P, q, sm->_tris[i], colinear);
			if (colinear)
			{
				nIntersect = 0.;
				i = 0;
				vtr3 dq = 1.e-4*(2.*rand() - 1.)*n; // random increment along normal
				q = q0 + dq;
			} else
			{
				nIntersect += s;
				++i;
			}
		}
	}
	return nIntersect != 0.;
}

