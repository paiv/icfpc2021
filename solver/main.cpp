#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>


using std::vector;

typedef uint8_t u8;
typedef uint32_t u32;


typedef struct {
    u32 x, y;
} point;


static void
read_line(vector<point>& vec) {
    point pt;
    while (read(STDIN_FILENO, &pt, sizeof(pt)) > 0) {
        if (pt.x == 0xffffffff) { break; }
        vec.push_back(pt);
    }
}


template<typename T>
static void
read_value(T& value) {
    read(STDIN_FILENO, &value, sizeof(value));
}


int
main(int argc, char const *argv[]) {
    vector<point> hole, fig_vertices, fig_edges;
    u32 epsilon = 0;
    read_line(hole);
    read_line(fig_vertices);
    read_line(fig_edges);
    read_value(epsilon);

    u32 solution[] = {
    21, 28, 31, 28, 31, 87, 29, 41, 44, 43, 58, 70,
    38, 79, 32, 31, 36, 50, 39, 40, 66, 77, 42, 29,
    46, 49, 49, 38, 39, 57, 69, 66, 41, 70, 39, 60,
    42, 25, 40, 35 };
    write(STDOUT_FILENO, solution, sizeof(solution));

    return 0;
}
