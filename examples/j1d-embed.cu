

int main() {

  const int Dim_0 = 10000;
  float     A[Dim_0];
  int       TS    = 100;
  
  #pragma overtile begin

  program j1d is
  grid 1
  field A float inout
    
  A[1:1] = 0.333 * (A[-1] + A[0] + A[1])

  #pragma overtile end

  return 0;
}
