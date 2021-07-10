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

    vector<u32> solution;
    for (point& pt : fig_vertices) {
        solution.push_back(pt.x);
        solution.push_back(pt.y);
    }
    u32* soldata = solution.data();
    u32 solsize = solution.size() * sizeof(u32);
    write(STDOUT_FILENO, soldata, solsize);

    return 0;
}
