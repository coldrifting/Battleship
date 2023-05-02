//// Custom class definitions
#define UP    Point( 0, -1)
#define DOWN  Point( 0,  1)
#define LEFT  Point(-1,  0)
#define RIGHT Point( 1,  0)

// Useful class for storing points and related operations
class Point : public Printable {
    public:
    int8_t x;
    int8_t y;

    // Default constuctor with no arguments
    Point() {
        this -> x = 0;
        this -> y = 0;
    }

    // Constructor
    Point(int8_t x, int8_t y) {
        this -> x = x;
        this -> y = y;
    }

    Point operator + (Point other) {
        return Point(this -> x + other.x, this -> y + other.y);
    }

    Point operator + (int scalar) {
        return Point(this -> x + scalar, this -> y + scalar);
    }

    Point operator * (Point other) {
        return Point(this -> x * other.x, this -> y * other.y);
    }

    Point operator * (int scalar) {
        return Point(this -> x * scalar, this -> y * scalar);
    }

    void operator += (Point other) {
        this -> x = this -> x + other.x;
        this -> y = this -> y + other.y;
    }

    void operator += (int scalar) {
        this -> x = this -> x + scalar;
        this -> y = this -> y + scalar;
    }

    void operator *= (Point other) {
        this -> x = this -> x * other.x;
        this -> y = this -> y * other.y;
    }

    void operator *= (int scalar) {
        this -> x = this -> x * scalar;
        this -> y = this -> y * scalar;
    }

    bool operator == (Point other) {
        return (this -> x == other.x && this -> y == other.y);
    }

    bool operator != (Point other) {
        return !(this == &other);
    }

    // get the distance between two points. Assumes they share an axis
    int8_t getDistance(const Point other) {
        int8_t distX = abs(this -> x - other.x);
        int8_t distY = abs(this -> y - other.y);
        if (distX >= distY)
            return distX;
        else
            return distY;
    }

    // Get a point representing the direction from this point to the other point
    Point getVector(const Point other) {
        if (this -> x == other.x) {
            if (this -> y > other.y)
                return UP;
            else
                return DOWN;
        } else {
            if (this -> x > other.x)
                return LEFT;
            else
                return RIGHT;
        }
    }

    // Convert a point to a one-dimensional index
    int8_t convertTo1D(int8_t width) {
      return (this -> x * width) + this -> y;
    }

    // Get the row value from a one-dimensional index with width wide columns
    static int8_t getRow(int8_t index1D, int8_t width) {
      return index1D / width;
    }

    // Get the column value from a one-dimensional index with width wide columns
    static int8_t getCol(int8_t index1D, int8_t width) {
      return index1D % width;
    }

    // Check if a point is inside the bounds of (0,0) to (size,size)
    bool isInBounds(int8_t size) {
        if (this -> x < 0 || this -> x >= size)
            return false;
        if (this -> y < 0 || this -> y >= size)
            return false;
        return true;
    }

    // Allows for printing points to the serial console
    size_t printTo(Print& p) const {
        size_t r = 0;

        r += p.print("Point: (");
        r += p.print(x);
        r += p.print(", ");
        r += p.print(y);
        r += p.print(")");
        return r;
    }
};

// Store basic information about ships
class Ship {
    public:
        int8_t hits;
        int8_t size;
        bool isPlaced;
        Point points[5];
        Ship(int8_t size) {
            this -> size = size;
            for (int i = 0; i < 5; i++) {
                points[i] = Point(-1,-1);
            }
        }
        
        bool isSunk() {
            return hits == size;
        }
};
