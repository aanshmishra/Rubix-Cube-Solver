#include "core/cube_state.h"
#include "core/facelet_converter.h"
#include "core/bidirectional_bfs.h"
#include "solver/solve_api.h"
#include <cstdio>
#include <chrono>
using namespace rubiks;
static const std::string T = std::string(9,'W')+std::string(9,'R')+std::string(9,'G')+std::string(9,'Y')+std::string(9,'O')+std::string(9,'B');
int main(){
  std::printf("sizeof(CubeState) = %d bytes\n", (int)sizeof(CubeState));
  std::printf("sizeof(Key64) = %d, sizeof(Key128) = %d\n", (int)sizeof(Key64), (int)sizeof(Key128));
  CubieEncoder enc(T);
  unsigned s=99; auto rnd=[&]{ s=s*1103515245u+12345u; return (s>>16)&0x7fff; };
  const char* f[6]={"U","R","F","D","L","B"}; const char* x[3]={"","'","2"};
  double tot=0; int n=60; long moves=0;
  for(int i=0;i<n;i++){
    std::string scr; int last=-1;
    for(int j=0;j<25;j++){ int a; do{a=rnd()%6;}while(a==last); last=a; if(j)scr+=" "; scr+=f[a]; scr+=x[rnd()%3]; }
    CubeState st=CubeState::solved(); st.applyNotation(scr);
    auto t0=std::chrono::steady_clock::now();
    SolveOutput r=solveFacelets(enc.decode(st),T,true,true,true);
    auto t1=std::chrono::steady_clock::now();
    tot+=std::chrono::duration<double,std::milli>(t1-t0).count();
    moves+=parseNotation(r.crossMoves).size()+parseNotation(r.f2lMoves).size()+parseNotation(r.ollMoves).size()+parseNotation(r.pllMoves).size();
  }
  std::printf("native: %d solves, mean %.1f ms, mean %.1f moves\n", n, tot/n, (double)moves/n);
}
