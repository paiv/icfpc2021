#include <stdint.h>
#include <unistd.h>
#include <vector>


using std::vector;

typedef uint8_t u8;
typedef uint32_t u32;


typedef struct {
    u32 x, y;
} point;


typedef struct {
    vector<point> hole;
    vector<point> fig_vertices;
    vector<point> fig_edges;
    u32 epsilon;
    vector<point> soa;
} problem_t;


static void
read_line(vector<point>& vec) {
    point pt;
    while (read(STDIN_FILENO, &pt, sizeof(pt)) > 0) {
        if (pt.y == 0xffffffff) {
            if (pt.x != 0xffffffff) {
                vec.push_back(pt);
            }
            break;
        }
        vec.push_back(pt);
    }
}


template<typename T>
static void
read_value(T& value) {
    read(STDIN_FILENO, &value, sizeof(value));
}


static problem_t
read_problem() {
    problem_t problem = {};
    read_line(problem.hole);
    read_line(problem.fig_vertices);
    read_line(problem.fig_edges);
    read_line(problem.soa);
    if (problem.soa[0].y == 0xffffffff) {
        problem.epsilon = problem.soa[0].x;
        problem.soa.clear();
    }
    else {
        read_value(problem.epsilon);
    }
    return problem;
}

static void
write_solution(vector<point>& solution) {
    point* soldata = solution.data();
    u32 solsize = solution.size() * sizeof(point);
    write(STDOUT_FILENO, soldata, solsize);
}


static vector<point>
stub_solver(problem_t& problem) {
    return problem.fig_vertices;
}


int
main(int argc, char const *argv[]) {
    problem_t problem = read_problem();
    auto solution = stub_solver(problem);
    write_solution(solution);

    return 0;
}
