#ifndef MORPH_HPP
#define MORPH_HPP
#include "raw.hpp"
#include <set>
#include <random>
#include <vector>

using std::set;
using std::random_device;
using std::default_random_engine;
using std::uniform_int_distribution;
using std::vector;

namespace Morph{
	class Pt{
	public:
		int r, c;
		Pt(){ r = c = 0;}
		Pt(const int &a, const int &b){ r = a; c = b;}
		
		bool operator < (const Pt &a) const{ return (r<a.r || (r==a.r && c<a.c));}
		bool out(const int &size) const{ return (r<0 || r>=size || c<0 || c>=size);}
		void print() const{ printf("%d %d\n", r, c);}
	};
	
	template<int SIZE>
	class Set : public set<Pt>{
	public:
		Set<SIZE> (){};
		Set<SIZE> (const Pt *begin, const Pt *end){ for(auto i=begin;i!=end;++i) this->insert(*i);}
		Set<SIZE> (const Image<SIZE> &im){
			for(int i=0;i<SIZE;++i)
				for(int j=0;j<SIZE;++j) if(im.data[i][j]==255) this->insert(Pt(i,j));
		}
		
		Set<SIZE> operator ! () const{
			Set<SIZE> ret;
			for(int i=0;i<SIZE;++i)
				for(int j=0;j<SIZE;++j)
					if(this->find(Pt(i,j)) == this->end()) ret.insert(Pt(i,j));
			return ret;
		}
		
		Set<SIZE> operator ~ () const{
			Set<SIZE> ret;
			for(auto &a : *this) ret.insert(Pt(-a.r,-a.c));
			return ret;
		}
		
		Set<SIZE> operator + (const Set<SIZE> &s) const{
			Set<SIZE> ret;
			for(auto &a : *this){
				for(auto &b : s){
					Pt x(a.r+b.r, a.c+b.c);
					if(!x.out(SIZE)) ret.insert(x);
				}
			}
			return ret;
		}
		
		Set<SIZE> operator - (const Set<SIZE> &s) const{
			Set<SIZE> ret;
			for(int i=0;i<SIZE;++i){
				for(int j=0;j<SIZE;++j){
					bool flag = true;
					for(auto &b : s){
						if(this->find(Pt(b.r+i, b.c+j)) == this->end()){
							flag = false; break;
						}
					}
					if(flag) ret.insert(Pt(i,j));
				}
			}
			return ret;
		}
		
		Set<SIZE> operator & (const Set<SIZE> &s) const{
			Set<SIZE> ret;
			for(auto &x : s)
				if(this->find(x) != this->end()) ret.insert(x);
			return ret;
		}
		
		Set<SIZE> operator | (const Set<SIZE> &s) const{
			Set<SIZE> ret;
			for(auto &x : *this) ret.insert(x);
			for(auto &x : s) ret.insert(x);
			return ret;
		}
		
		void print() const{
			for(auto &a : *this) printf("%d %d   ", a.r, a.c);
			printf("\n");
		}
		
		
	};
	
	Pt all4[5] = {Pt(-1,0),Pt(0,-1),Pt(0,0),Pt(0,1),Pt(1,0)};
	Pt all8[9] = {Pt(-1,-1),Pt(-1,0),Pt(-1,1),Pt(0,-1),Pt(0,0),Pt(0,1),Pt(1,-1),Pt(1,0),Pt(1,1)};
	
	template<int SIZE>
	Image<SIZE> boundary(const Image<SIZE> &im, char mode){
		Set<SIZE> s(im);
		if(mode != '4'){
			if(mode != '8'){ printf("No such boundary mode:  only 4,8\n"); exit(1);}
			s = s & !(s - Set<SIZE>(all4, all4+5));
		}else s = s & !(s - Set<SIZE>(all8,all8+9));
		Image<SIZE> ret;
		for(auto & x : s) ret.data[x.r][x.c]=255;
		return ret;
	}
	
	template<int SIZE>
	int labeling(const Image<SIZE> &im, int (*label)[SIZE], char mode){
		int ret=0; bool flag = true; Set<SIZE> ims(im), t;
		if(mode != '4'){
			if(mode != '8'){ printf("No such boundary mode:  only 4,8\n"); exit(1);}
			t = Set<SIZE>(all4, all4+5);
		}else t = Set<SIZE>(all8,all8+9);
		
		while(ims.size() > 0){
			++ret;
			Set<SIZE> s; s.insert(*(ims.begin()));
			int last_size = 0;
			while(last_size < s.size()){
				last_size = s.size();
				s = (s+t) & ims;
			}
			for(auto &x : s){
				label[x.r][x.c] = ret;
				int tmx = ims.size();
				ims.erase(x);
			}
		}
		return ret;
	}
	
	template<int SIZE>
	void generateColor(const int (*label)[SIZE], const int &colorNum, const char *name){
		random_device rd;
		default_random_engine gen(rd());
		uniform_int_distribution<int> dis(64,255);
		vector<vector<Data>> colors;
		vector<Data> tmx(3, 0); colors.push_back(tmx);
		for(int i=0;i<colorNum;++i){
			tmx.clear();
			for(int j=0;j<3;++j) tmx.push_back(dis(gen));
			colors.push_back(tmx);
		}
		FILE *fx = myOpen(name, "wb");
		for(int k=0;k<3;++k)
			for(int i=0;i<SIZE;++i)
				for(int j=0;j<SIZE;++j) fwrite(&colors[label[i][j]][k], sizeof(Data), 1, fx);
		fclose(fx);
	}
	
	
	//---------------------------thinning-----------------------
	bool isMThin(const bool b0, const bool b1, const bool b2, const bool b3, const bool b4, const bool b5, const bool b6, const bool b7, const bool b8){
		if(!b0 && b1 &&!b2 &&!b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 && b1 &&!b2 && b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 &&!b5 &&!b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 && b5 &&!b6 && b7 &&!b8) return true;
		
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 &&!b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 &&!b2 &&!b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 && b1 &&!b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if(!b0 && b1 && b2 && b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 &&!b8) return true;
		
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		
		if( b0 && b1 &&!b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if(!b0 && b1 && b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 && b6 &&!b7 &&!b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 && b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if( b0 &&!b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		return false;
	}
	
	bool canDelThin(const bool b0, const bool b1, const bool b2, const bool b3, const bool b5, const bool b6, const bool b7, const bool b8){
		if(!b0 &&!b1 && b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if( b0 &&!b1 &&!b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 &&!b5 &&!b6 && b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 && b5 &&!b6 &&!b7 &&!b8) return false;
		
		if(!b0 &&!b1 && b2 &&!b3 && b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 && b1 && b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if( b0 && b1 &&!b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if( b0 &&!b1 &&!b2 && b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 && b3 &&!b5 && b6 &&!b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 &&!b5 && b6 && b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 &&!b5 &&!b6 && b7 && b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 && b5 &&!b6 &&!b7 && b8) return false;
		
		if(!b0 && b1 && b2 && b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if( b0 && b1 &&!b2 &&!b3 && b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 && b1 &&!b2 &&!b3 && b5 &&!b6 &&!b7 && b8) return false;
		if(!b0 &&!b1 && b2 &&!b3 && b5 &&!b6 && b7 &&!b8) return false;
		
		if(!b0 && b2 &&!b3 && b6 &&!b7 &&!b8 && (b1 || b5)) return false;
		if( b0 &&!b2 &&!b5 &&!b6 &&!b7 && b8 && (b3 || b1)) return false;
		if(!b0 &&!b1 && b2 &&!b5 && b6 &&!b8 && (b3 || b7)) return false;
		if( b0 &&!b1 &&!b2 &&!b3 &&!b6 && b8 && (b7 || b5)) return false;
		
		if( b0 && b1 && b3) return false;
		
		if( b1 &&!b2 && b3 && b5 &&!b7 &&!b8) return false;
		if(!b0 && b1 && b3 && b5 &&!b6 &&!b7) return false;
		if(!b0 &&!b1 && b3 && b5 &&!b6 && b7) return false;
		if(!b1 &&!b2 && b3 && b5 && b7 &&!b8) return false;
		if( b1 && b3 &&!b5 &&!b6 && b7 &&!b8) return false;
		if(!b0 && b1 &&!b2 && b3 &&!b5 && b7) return false;
		if(!b0 && b1 &&!b2 &&!b3 && b5 && b7) return false;
		if( b1 &&!b3 && b5 &&!b6 && b7 &&!b8) return false;
		
		if( b0 && b2 && (b6 || b7 || b8)) return false;
		if( b0 && b6 && (b8 || b5 || b2)) return false;
		if( b6 && b8 && (b2 || b1 || b0)) return false;
		if( b2 && b8 && (b0 || b3 || b6)) return false;
		
		if( b1 &&!b2 &&!b3 && b5 && b6 &&!b7) return false;
		if(!b0 && b1 && b3 &&!b5 &&!b7 && b8) return false;
		if(!b1 && b2 && b3 &&!b5 &&!b6 && b7) return false;
		if( b0 &&!b1 &&!b3 && b5 && b7 &&!b8) return false;
		return true;
	}
	
	template<int SIZE>
	Image<SIZE> thinning(const Image<SIZE> &im){
		Image<SIZE> ret(im); bool flag = true;
		while(flag){
			flag = false;
			bool x[SIZE+4][SIZE+4] = {{false}};
			for(int i=0;i<SIZE;++i) for(int j=0;j<SIZE;++j)
				if(ret.data[i][j] == 255) x[i+2][j+2] = true;
			
			bool m[SIZE+2][SIZE+2] = {{false}};
			for(int i=0;i<SIZE+2;++i) for(int j=0;j<SIZE+2;++j)
				m[i][j] = isMThin(x[i][j], x[i][j+1], x[i][j+2], x[i+1][j], x[i+1][j+1], x[i+1][j+2], x[i+2][j], x[i+2][j+1], x[i+2][j+2]);
			
			for(int i=1;i<=SIZE;++i){
				for(int j=1;j<=SIZE;++j){
					if(ret.data[i-1][j-1] == 255 && m[i][j]){
						if(canDelThin(m[i-1][j-1], m[i-1][j], m[i-1][j+1], m[i][j-1], m[i][j+1], m[i+1][j-1], m[i+1][j], m[i+1][j+1])){
							ret.data[i-1][j-1] = 0; flag = true;
						}
					}
				}
			}
		}
		return ret;
	}
	
	//----------------------skeletonizing-----------------------
	bool isMSkel(const bool b0, const bool b1, const bool b2, const bool b3, const bool b4, const bool b5, const bool b6, const bool b7, const bool b8){
		if(!b0 && b1 &&!b2 &&!b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 && b1 &&!b2 && b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 &&!b5 &&!b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 && b5 &&!b6 && b7 &&!b8) return true;
		
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 &&!b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 &&!b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 && b6 &&!b7 &&!b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 && b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 &&!b6 &&!b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if(!b0 &&!b1 &&!b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		if(!b0 && b1 && b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 && b6 &&!b7 &&!b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 &&!b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 && b6 && b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if( b0 &&!b1 &&!b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		if(!b0 &&!b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 &&!b3 && b4 && b5 && b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 && b6 &&!b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 &&!b5 && b6 && b7 && b8) return true;
		if( b0 &&!b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		
		if( b0 && b1 && b2 && b3 && b4 && b5 &&!b6 && b7 && b8) return true;
		if( b0 && b1 && b2 && b3 && b4 && b5 && b6 && b7 &&!b8) return true;
		if( b0 && b1 &&!b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		if(!b0 && b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8) return true;
		return false;
	}
	
	bool canDelSkel(const bool b0, const bool b1, const bool b2, const bool b3, const bool b5, const bool b6, const bool b7, const bool b8){
		if(!b0 &&!b1 &&!b2 &&!b3 &&!b5 &&!b6 &&!b7 && b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 &&!b5 && b6 &&!b7 &&!b8) return false;
		if(!b0 &&!b1 && b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if( b0 &&!b1 &&!b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		
		if(!b0 &&!b1 &&!b2 &&!b3 &&!b5 &&!b6 && b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 && b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 && b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 && b1 &&!b2 &&!b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		
		if(!b0 && b1 &&!b2 &&!b3 && b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 && b1 &&!b2 && b3 &&!b5 &&!b6 &&!b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 &&!b3 && b5 &&!b6 && b7 &&!b8) return false;
		if(!b0 &&!b1 &&!b2 && b3 &&!b5 &&!b6 && b7 &&!b8) return false;
		
		if( b1 && b2 && b5) return false;
		if( b3 && b6 && b7) return false;
		if( b0 && b1 && b3) return false;
		if( b5 && b7 && b8) return false;
		
		if( b1 && b3 && b5 &&!b7 &&!b8) return false;
		if( b1 && b3 && b7) return false;
		if( b3 && b5 && b7) return false;
		if( b1 && b5 && b7) return false;
		
		if( b0 && b2 && (b6 || b7 || b8)) return false;
		if( b0 && b6 && (b8 || b5 || b2)) return false;
		if( b6 && b8 && (b2 || b1 || b0)) return false;
		if( b2 && b8 && (b0 || b3 || b6)) return false;
		
		if( b1 &&!b2 &&!b3 && b5 && b6 &&!b7) return false;
		if(!b0 && b1 && b3 &&!b5 &&!b7 && b8) return false;
		if(!b1 && b2 && b3 &&!b5 &&!b6 && b7) return false;
		if( b0 &&!b1 &&!b3 && b5 && b7 &&!b8) return false;
		return true;
	}
	
	template<int SIZE>
	Image<SIZE> skeletonizing(const Image<SIZE> &im){
		Image<SIZE> ret(im); bool flag = true;
		while(flag){
			flag = false;
			bool x[SIZE+4][SIZE+4] = {{false}};
			for(int i=0;i<SIZE;++i) for(int j=0;j<SIZE;++j)
				if(ret.data[i][j] == 255) x[i+2][j+2] = true;
			
			bool m[SIZE+2][SIZE+2] = {{false}};
			for(int i=0;i<SIZE+2;++i) for(int j=0;j<SIZE+2;++j)
				m[i][j] = isMSkel(x[i][j], x[i][j+1], x[i][j+2], x[i+1][j], x[i+1][j+1], x[i+1][j+2], x[i+2][j], x[i+2][j+1], x[i+2][j+2]);
			
			for(int i=1;i<=SIZE;++i){
				for(int j=1;j<=SIZE;++j){
					if(ret.data[i-1][j-1] == 255 && m[i][j]){
						if(canDelSkel(m[i-1][j-1], m[i-1][j], m[i-1][j+1], m[i][j-1], m[i][j+1], m[i+1][j-1], m[i+1][j], m[i+1][j+1])){
							ret.data[i-1][j-1] = 0; flag = true;
						}
					}
				}
			}
		}
		return ret;
	}
	
	template<int SIZE>
	Image<SIZE> skeletonizing2(const Image<SIZE> &im){
		Image<SIZE> ret(im); int mode = 0, cnt = 0;
		while(cnt < 8){
			bool flag = false;
			bool b[SIZE+2][SIZE+2] = {{false}};
			for(int i=0;i<SIZE;++i) for(int j=0;j<SIZE;++j)
				if(ret.data[i][j]==255) b[i+1][j+1] = true;
			for(int i=1;i<=SIZE;++i){ for(int j=1;j<=SIZE;++j){
				if(!b[i][j]) continue;
				switch(mode){
				case 0:
					if(!b[i-1][j-1]&&!b[i-1][j]&&!b[i-1][j+1]&&b[i+1][j-1]&&b[i+1][j]&&b[i+1][j+1]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 1:
					if(!b[i-1][j]&&!b[i-1][j+1]&&b[i][j-1]&&!b[i][j+1]&&b[i+1][j-1]&&b[i+1][j]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 2:
					if(b[i-1][j-1]&&!b[i-1][j+1]&&b[i][j-1]&&!b[i][j+1]&&b[i+1][j-1]&&!b[i+1][j+1]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 3:
					if(b[i-1][j-1]&&b[i-1][j]&&b[i][j-1]&&!b[i][j+1]&&!b[i+1][j]&&!b[i+1][j+1]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 4:
					if(b[i-1][j-1]&&b[i-1][j]&&b[i-1][j+1]&&!b[i+1][j-1]&&!b[i+1][j]&&!b[i+1][j+1]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 5:
					if(b[i-1][j]&&b[i-1][j+1]&&!b[i][j-1]&&b[i][j+1]&&!b[i+1][j-1]&&!b[i+1][j]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 6:
					if(!b[i-1][j-1]&&b[i-1][j+1]&&!b[i][j-1]&&b[i][j+1]&&!b[i+1][j-1]&&b[i+1][j+1]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
					break;
				case 7:
					if(!b[i-1][j-1]&&!b[i-1][j]&&!b[i][j-1]&&b[i][j+1]&&b[i+1][j]&&b[i+1][j+1]){
						ret.data[i-1][j-1] = 0; flag = true;
					}
				}
			}}
			mode = (mode+1)%8;
			cnt = (flag)? 0:(cnt+1);
		}
		return ret;
	}
}
#endif