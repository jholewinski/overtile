

namespace overtile {

/**
 * Abstraction of a space in Z^n
 */
class Grid {
public:
  Grid(unsigned Dim);
  ~Grid();

private:

  /// Number of dimensions in the space
  unsigned Dimensions;
};

}
